/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 */

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdexcept>

#include "MarketExchange2.h"
#include "DBT5Util.h"

// A Brokerage House connection that sends trade requests
struct CMarketExchange2::TConnection
{
	ev_io io; // becomes ready when a trade request arrives
	CSocket socket;
	CMarketExchange2 *pMarketExchange;
};

// Constructor
CMarketExchange2::CMarketExchange2(const DataFileManager &inputFiles,
		char *szFileLoc, UINT32 UniqueId, TIdent iConfiguredCustomerCount,
		TIdent iActiveCustomerCount, int iListenPort, char *szBHaddr,
		int iBHlistenPort, char *outputDirectory, bool verbose,
		int iConnections)
: m_UniqueId(UniqueId), m_iListenPort(iListenPort), m_pLog(NULL),
  m_Verbose(verbose), m_pLoop(EV_DEFAULT), m_pCMEESUT(NULL), m_pCMEE(NULL)
{
	char filename[iMaxPath + 1];
	snprintf(filename, sizeof(filename), "%s/MarketExchange.log",
			outputDirectory);
	m_pLog = new CEGenLogger(eDriverEGenLoader, 0, filename, &m_fmt);

	// Initialize MEESUT
	m_pCMEESUT = new CMEESUT2(
			m_pLoop, outputDirectory, szBHaddr, iBHlistenPort, iConnections);

	// Initialize MEE
	m_pCMEE = new CMEE(0, m_pCMEESUT, m_pLog, inputFiles, UniqueId);
	m_pCMEE->SetBaseTime();

	ev_init(&m_Timer, timerCallback);
	m_Timer.data = this;
}

// Destructor
CMarketExchange2::~CMarketExchange2()
{
	ev_timer_stop(m_pLoop, &m_Timer);
	ev_io_stop(m_pLoop, &m_Accept);

	delete m_pCMEE;
	delete m_pCMEESUT;
	delete m_pLog;
}

void
CMarketExchange2::startListener(void)
{
	m_Socket.dbt5Listen(m_iListenPort);

	// A connection reset before it is accepted leaves nothing to accept,
	// which must not block the event loop.
	if (!setNonBlocking(m_Socket.getListenFd())) {
		throw std::runtime_error(string("cannot make listener non-blocking: ")
								 + strerror(errno));
	}

	ev_io_init(&m_Accept, acceptCallback, m_Socket.getListenFd(), EV_READ);
	m_Accept.data = this;
	ev_io_start(m_pLoop, &m_Accept);

	ev_run(m_pLoop, 0);
}

void
CMarketExchange2::acceptCallback(struct ev_loop *loop, ev_io *w, int revents)
{
	CMarketExchange2 *pMarketExchange
			= reinterpret_cast<CMarketExchange2 *>(w->data);
	int fd;

	do {
		fd = accept(w->fd, NULL, NULL);
	} while (fd == -1 && errno == EINTR);
	if (fd == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK && errno != ECONNABORTED) {
			cerr << "Problem to accept socket connection" << endl
				 << "Error: " << strerror(errno) << " at "
				 << "MarketExchange2::acceptCallback" << endl;
		}
		return;
	}

	TConnection *pConnection = new TConnection;
	pConnection->socket.setSocketFd(fd);
	pConnection->pMarketExchange = pMarketExchange;
	ev_io_init(&pConnection->io, requestCallback, fd, EV_READ);
	pConnection->io.data = pConnection;
	ev_io_start(loop, &pConnection->io);
}

void
CMarketExchange2::requestCallback(struct ev_loop *loop, ev_io *w, int revents)
{
	TConnection *pConnection = reinterpret_cast<TConnection *>(w->data);
	CMarketExchange2 *pMarketExchange = pConnection->pMarketExchange;

	// No exception may propagate into the event loop, which is C.
	try {
		pMarketExchange->requestReady(pConnection);
	} catch (...) {
		cerr << time(NULL) << " Trade Request not submitted to Market Exchange"
			 << endl
			 << "Error: " << caughtExceptionText() << endl;
		pMarketExchange->closeConnection(pConnection);
	}
}

// Submit a trade request that a Brokerage House sent
void
CMarketExchange2::requestReady(TConnection *pConnection)
{
	TTradeRequest request;

	try {
		pConnection->socket.dbt5Receive(
				reinterpret_cast<void *>(&request), sizeof(TTradeRequest));
	} catch (CSocketErr *pErr) {
		if (pErr->getAction() != CSocketErr::ERR_SOCKET_CLOSED) {
			cerr << time(NULL)
				 << " Trade Request not submitted to Market Exchange" << endl
				 << "Error: " << pErr->ErrorText() << endl;
		}
		delete pErr;

		// The socket is closed, stop serving it.
		closeConnection(pConnection);
		return;
	}

	if (m_Verbose) {
		cout << "TTradeRequest" << endl
			 << "  price_quote: " << request.price_quote << endl
			 << "  trade_id: " << request.trade_id << endl
			 << "  trade_qty: " << request.trade_qty << endl
			 << "  eAction: " << request.eAction << endl
			 << "  symbol: " << request.symbol << endl
			 << "  trade_type_id: " << request.trade_type_id << endl;
	}

	// submit trade request
	runMEE(&request);
}

// Stop serving a connection and close it
void
CMarketExchange2::closeConnection(TConnection *pConnection)
{
	ev_io_stop(m_pLoop, &pConnection->io);
	delete pConnection;
}

// Fire expired MEE timers.  SubmitTradeRequest and GenerateTradeResult
// return the number of milliseconds until the next pending timer, and
// GenerateTradeResult must be called when that time elapses or pending
// Trade-Result and triggered Market-Feed transactions are never sent.
void
CMarketExchange2::timerCallback(struct ev_loop *loop, ev_timer *w, int revents)
{
	CMarketExchange2 *pMarketExchange
			= reinterpret_cast<CMarketExchange2 *>(w->data);

	// No exception may propagate into the event loop, which is C.
	try {
		pMarketExchange->runMEE(NULL);
	} catch (...) {
		cerr << "Error: " << caughtExceptionText()
			 << " at MarketExchange2::timerCallback" << endl;
	}
}

// Submit a trade request to the MEE, or fire its expired timers when
// pRequest is NULL.  Then rearm the timer, and send the Trade-Result and
// Market-Feed transactions that the MEE generated.
void
CMarketExchange2::runMEE(PTradeRequest pRequest)
{
	INT32 delay = 0;

	// The MEE does not release its lock when an exception propagates out of
	// it, so it cannot be called again.
	try {
		if (pRequest != NULL) {
			delay = m_pCMEE->SubmitTradeRequest(pRequest);
		} else {
			delay = m_pCMEE->GenerateTradeResult();
		}
	} catch (...) {
		cerr << "Error: " << caughtExceptionText()
			 << " in Market Exchange Emulator, exiting" << endl;
		exit(1);
	}

	setTimer(delay);
	m_pCMEESUT->sendRequests();
}

// Arm the timer to expire in delay milliseconds, or leave it stopped when
// delay is negative because no MEE timer is outstanding.
void
CMarketExchange2::setTimer(INT32 delay)
{
	ev_timer_stop(m_pLoop, &m_Timer);
	if (delay < 0) {
		return;
	}

	// The loop read the time before the callbacks that ran since then.
	ev_now_update(m_pLoop);
	ev_timer_set(&m_Timer, delay / 1000.0, 0.);
	ev_timer_start(m_pLoop, &m_Timer);
}

bool
CMarketExchange2::verbose()
{
	return m_Verbose;
}

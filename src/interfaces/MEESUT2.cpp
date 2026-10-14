/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 */

#include <unistd.h>

#include "MEESUT2.h"
#include "DBT5Util.h"

// Constructor
CMEESUT2::CMEESUT2(struct ev_loop *pLoop, char *outputDirectory, char *addr,
		const int iListenPort, int iConnections)
: m_pLoop(pLoop)
{
	pid_t pid = getpid();
	char filename[iMaxPath + 1];

	snprintf(filename, sizeof(filename), "%s/mix-me-%d.log", outputDirectory,
			pid);
	m_fMix.open(filename, ios::out);
	snprintf(filename, sizeof(filename), "%s/error-me-%d.log", outputDirectory,
			pid);
	m_fLog.open(filename, ios::out);

	for (int i = 0; i < iConnections; i++) {
		TConnection *pConnection = new TConnection;
		pConnection->pInterface
				= new CBaseInterface(&m_fMix, &m_fLog, addr, iListenPort);
		pConnection->pMEESUT = this;
		pConnection->bBusy = false;
		ev_init(&pConnection->io, replyCallback);
		pConnection->io.data = pConnection;
		m_Connections.push_back(pConnection);
	}
}

// Destructor
CMEESUT2::~CMEESUT2()
{
	for (size_t i = 0; i < m_Connections.size(); i++) {
		ev_io_stop(m_pLoop, &m_Connections[i]->io);
		delete m_Connections[i]->pInterface;
		delete m_Connections[i];
	}
}

// Copy the transaction and return immediately, as MEESUTInterface.h
// recommends.  The caller sends it with sendRequests() once the MEE returns.
bool
CMEESUT2::TradeResult(PTradeResultTxnInput pTxnInput)
{
	TMsgDriverBrokerage request;

	memset(&request, 0, sizeof(TMsgDriverBrokerage));
	request.TxnType = TRADE_RESULT;
	memcpy(&(request.TxnInput.TradeResultTxnInput), pTxnInput,
			sizeof(request.TxnInput.TradeResultTxnInput));
	m_Requests.push_back(request);

	return true;
}

// Market Feed, queued the same way as Trade Result
bool
CMEESUT2::MarketFeed(PMarketFeedTxnInput pTxnInput)
{
	TMsgDriverBrokerage request;

	memset(&request, 0, sizeof(TMsgDriverBrokerage));
	request.TxnType = MARKET_FEED;
	memcpy(&(request.TxnInput.MarketFeedTxnInput), pTxnInput,
			sizeof(request.TxnInput.MarketFeedTxnInput));
	m_Requests.push_back(request);

	return true;
}

// Send queued transactions on the connections that are not waiting for a
// reply.  The response time of each is measured from when it is sent, as in
// the thread based MEESUT, where it is measured once the thread holds the
// socket.
void
CMEESUT2::sendRequests()
{
	for (size_t i = 0; i < m_Connections.size(); i++) {
		TConnection *pConnection = m_Connections[i];

		while (!pConnection->bBusy && !m_Requests.empty()) {
			TMsgDriverBrokerage request = m_Requests.front();
			m_Requests.pop_front();

			// A transaction that cannot be sent is logged and dropped, as
			// in the thread based MEESUT.  The failed send reconnects for
			// the next one, and throws if connecting again fails.
			try {
				if (pConnection->pInterface->sendRequest(&request)) {
					pConnection->bBusy = true;
					ev_io_set(&pConnection->io,
							pConnection->pInterface->getSocketFd(), EV_READ);
					ev_io_start(m_pLoop, &pConnection->io);
				}
			} catch (...) {
				ostringstream osErr;
				osErr << "Error: " << caughtExceptionText()
					  << " at MEESUT2::sendRequests" << endl;
				logErrorMessage(osErr.str());
				break;
			}
		}
	}
}

void
CMEESUT2::replyCallback(struct ev_loop *loop, ev_io *w, int revents)
{
	TConnection *pConnection = reinterpret_cast<TConnection *>(w->data);
	CMEESUT2 *pMEESUT = pConnection->pMEESUT;

	ev_io_stop(loop, w);
	pConnection->bBusy = false;

	// No exception may propagate into the event loop, which is C.
	// Receiving logs the response time, and reconnects after the Brokerage
	// House closed the connection, which throws if it fails.
	try {
		pConnection->pInterface->receiveReply();
	} catch (...) {
		ostringstream osErr;
		osErr << "Error: " << caughtExceptionText()
			  << " at MEESUT2::replyCallback" << endl;
		pMEESUT->logErrorMessage(osErr.str());
	}

	pMEESUT->sendRequests();
}

// logErrorMessage
void
CMEESUT2::logErrorMessage(const string sErr)
{
	cerr << sErr;
	m_fLog << sErr;
	m_fLog.flush();
}

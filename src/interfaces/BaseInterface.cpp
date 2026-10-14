/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 *
 * 13 August 2006
 */

#include <unistd.h>
#include <sys/syscall.h>

#include "BaseInterface.h"
#include "DBT5Consts.h"

// The interface opens its own mix and error logs, and talkToSUT() waits for
// each reply, blocking the thread that calls it.
CBaseInterface::CBaseInterface(const char type[3], char *outputDirectory,
		char *addr, const int iListenPort)
: m_szBHAddress(addr), m_iBHlistenPort(iListenPort), m_pfLog(&m_fLog),
  m_pfMix(&m_fMix), m_bEventLoop(false), m_bReplyPending(false),
  m_eTxnType(NULL_TXN)
{
	m_pid = syscall(SYS_gettid);

	sock = new CSocket(m_szBHAddress, m_iBHlistenPort);
	biConnect();

	char filename[iMaxPath + 1];

	memset(filename, 0, sizeof(filename));
	snprintf(filename, sizeof(filename), "%s/mix-%s-%d.log", outputDirectory,
			type, m_pid);
	m_fMix.open(filename, ios::out);

	memset(filename, 0, sizeof(filename));
	snprintf(filename, sizeof(filename), "%s/error-%s-%d.log", outputDirectory,
			type, m_pid);
	m_fLog.open(filename, ios::out);
}

// The interface writes to the mix and error logs that every interface in the
// process shares, and talkToSUT() only sends the request: the event loop
// calls receiveReply() once getSocketFd() is readable.  For programs that run
// many interfaces from one event loop.
CBaseInterface::CBaseInterface(
		ofstream *pfMix, ofstream *pfLog, char *addr, const int iListenPort)
: m_szBHAddress(addr), m_iBHlistenPort(iListenPort), m_pfLog(pfLog),
  m_pfMix(pfMix), m_bEventLoop(true), m_bReplyPending(false),
  m_eTxnType(NULL_TXN)
{
	m_pid = syscall(SYS_gettid);

	sock = new CSocket(m_szBHAddress, m_iBHlistenPort);
	biConnect();
}

// destructor
CBaseInterface::~CBaseInterface()
{
	biDisconnect();
	delete sock;

	m_fMix.close();
}

// connect to BrokerageHouse
bool
CBaseInterface::biConnect()
{
	try {
		sock->dbt5Connect();
		return true;
	} catch (std::runtime_error &err) {
		logErrorMessage(err.what());
		return false;
	} catch (CSocketErr *pErr) {
		ostringstream osErr;
		osErr << "Error: " << pErr->ErrorText()
			  << " at CBaseInterface::talkToSUT " << endl;
		logErrorMessage(osErr.str());
		return false;
	}
}

// close connection to BrokerageHouse
bool
CBaseInterface::biDisconnect()
{
	try {
		sock->dbt5Disconnect();
		return true;
	} catch (CSocketErr *pErr) {
		ostringstream osErr;
		osErr << "Error: " << pErr->ErrorText()
			  << " at CBaseInterface::talkToSUT " << endl;
		logErrorMessage(osErr.str());
		return false;
	}
}

// Send request to BrokerageHouse, then receive reply and calculate response
// time, unless the event loop receives the reply.  When it does, the return
// value only reports whether the request was sent.
bool
CBaseInterface::talkToSUT(PMsgDriverBrokerage pRequest)
{
	if (!sendRequest(pRequest))
		return false;
	if (m_bEventLoop)
		return true;
	return receiveReply();
}

// Send request to BrokerageHouse and start timing it
bool
CBaseInterface::sendRequest(PMsgDriverBrokerage pRequest)
{
	int length = 0;

	// record txn start time -- please, see TPC-E specification clause
	// 6.2.1.3
	m_StartTime.Set();
	m_eTxnType = pRequest->TxnType;

	try {
		length = sock->dbt5Send(
				reinterpret_cast<void *>(pRequest), sizeof(*pRequest));
	} catch (CSocketErr *pErr) {
		sock->dbt5Reconnect();
		logResponseTime(-1, 0, -1);

		ostringstream msg;
		msg << time(NULL) << " " << m_pid << " "
			<< szTransactionName[pRequest->TxnType] << ": " << endl
			<< "Error sending " << length << " bytes of data" << endl
			<< pErr->ErrorText() << endl;
		logErrorMessage(msg.str());
		delete pErr;
		// The request never reached the BrokerageHouse; do not wait
		// for a reply that cannot arrive.
		return false;
	}
	m_bReplyPending = true;
	return true;
}

// Receive reply to the request sendRequest() sent, and log its RT
bool
CBaseInterface::receiveReply()
{
	TMsgBrokerageDriver Reply; // reply message from BrokerageHouse
	memset(&Reply, 0, sizeof(Reply));

	m_bReplyPending = false;
	try {
		sock->dbt5Receive(reinterpret_cast<void *>(&Reply), sizeof(Reply));
	} catch (CSocketErr *pErr) {
		logResponseTime(-1, 0, -2);

		ostringstream msg;
		msg << time(NULL) << " " << m_pid << " "
			<< szTransactionName[m_eTxnType] << ": " << endl
			<< "Error receiving " << sizeof(Reply) << " bytes of data" << endl
			<< pErr->ErrorText() << endl;
		logErrorMessage(msg.str());
		if (pErr->getAction() == CSocketErr::ERR_SOCKET_CLOSED)
			sock->dbt5Reconnect();
		delete pErr;
		// No reply was received; the zeroed Reply would otherwise be
		// logged below as a successful transaction.
		return false;
	}

	// record txn end time
	CDateTime EndTime;

	// calculate txn response time
	CDateTime TxnTime;
	TxnTime.Set(0); // clear time
	TxnTime.Add(0, (int) ((EndTime - m_StartTime) * MsPerSecond)); // add ms

	// log response time
	logResponseTime(Reply.iStatus, m_eTxnType, TxnTime.MSec() / 1000.0);

	if (Reply.iStatus == CBaseTxnErr::SUCCESS)
		return true;
	return false;
}

// Whether a request was sent and its reply not received yet
bool
CBaseInterface::replyPending()
{
	return m_bReplyPending;
}

int
CBaseInterface::getSocketFd()
{
	return sock->getSocketFd();
}

// Log Transaction Response Times
void
CBaseInterface::logResponseTime(int iStatus, int iTxnType, double dRT)
{
	*m_pfMix << (long long) time(NULL) << "," << iTxnType << "," << iStatus
			 << "," << dRT << "," << m_pid << endl;
	m_pfMix->flush();
}

// logErrorMessage
void
CBaseInterface::logErrorMessage(const string sErr)
{
	cerr << sErr;
	*m_pfLog << sErr;
	m_pfLog->flush();
}

void
CBaseInterface::logStopTime()
{
	*m_pfMix << time(NULL) << ",STOP,,," << m_pid << endl;
	m_pfMix->flush();
}

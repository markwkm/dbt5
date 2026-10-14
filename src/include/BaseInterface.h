/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 *
 * Base class for emulator-SUT interface
 * 13 August 2006
 */

#ifndef BASE_INTERFACE_H
#define BASE_INTERFACE_H

#include "locking.h"

#include "CommonStructs.h"
#include "CSocket.h"
using namespace TPCE;

class CBaseInterface
{
protected:
	bool talkToSUT(PMsgDriverBrokerage);
	void logErrorMessage(const string);

	char *m_szBHAddress;
	int m_iBHlistenPort;

private:
	CSocket *sock;
	pid_t m_pid;
	ofstream m_fLog; // error log file
	ofstream m_fMix; // mix log file
	// The logs written to: m_fLog and m_fMix, or the logs that every
	// interface in an event driven process shares.
	ofstream *m_pfLog;
	ofstream *m_pfMix;

	// Whether the event loop, instead of talkToSUT(), reads each reply.
	bool m_bEventLoop;

	// The request sent and not yet answered.
	bool m_bReplyPending;
	eTxnType m_eTxnType;
	CDateTime m_StartTime;

	void logResponseTime(int, int, double);

public:
	CBaseInterface(const char *, char *, char *, const int);
	CBaseInterface(ofstream *, ofstream *, char *, const int);
	~CBaseInterface(void);
	bool biConnect();
	bool biDisconnect();

	bool sendRequest(PMsgDriverBrokerage);
	bool receiveReply();
	bool replyPending();
	int getSocketFd();

	void logStopTime();
};

#endif // BASE_INTERFACE_H

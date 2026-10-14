/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 *
 * The workload driver, emulating the users with an event loop in each of
 * several processes instead of a thread per user.
 */

#ifndef DRIVER2_H
#define DRIVER2_H

#include <ev.h>

#include <vector>
using namespace std;

#include "EGenLogFormatterTab.h"
#include "EGenLogger.h"
#include "CESUT.h"
#include "DMSUT.h"

using namespace TPCE;

class CDriver2;

// An emulated user: its Customer Emulator, and its connection to the
// Brokerage House.
class CUser
{
private:
	CDriver2 *m_pDriver;
	UINT32 m_UniqueId;
	CCESUT *m_pCCESUT;
	CCE *m_pCCE;

	// Expires to start the user, then at the end of each pacing delay.
	ev_timer m_Timer;
	// Becomes ready when the Brokerage House replies to a transaction.
	ev_io m_Reply;

	static void timerCallback(struct ev_loop *, ev_timer *, int);
	static void replyCallback(struct ev_loop *, ev_io *, int);

	void start();
	void doTxn();
	void pace();
	void stop(bool);
	void fail();

	// non-copyable
	CUser(const CUser &);
	CUser &operator=(const CUser &);

public:
	CUser(CDriver2 *, UINT32, ev_tstamp);
	~CUser();
};

class CDriver2
{
private:
	CLogFormatTab m_fmt;
	CEGenLogger *m_pLog;
	ofstream m_fLog; // error log file
	int m_iProcessesPerProcessor;
	time_t m_StopTime;

	// Data-Maintenance, which the parent process runs.
	CDMSUT *m_pCDMSUT;
	CDM *m_pCDM;

	// The users that a forked process emulates, and the event loop and logs
	// that they share.
	struct ev_loop *m_pLoop;
	CEGenLogger *m_pUserLog;
	ofstream m_fUserMix; // mix log file
	ofstream m_fUserLog; // error log file
	vector<CUser *> m_Users;
	size_t m_iUsersStarted;

	void logErrorMessage(const string);
	void runDataMaintenance();
	int runUsers(int, int, ev_tstamp, int);
	void userStarted();

	friend class CUser;

	// non-copyable
	CDriver2(const CDriver2 &);
	CDriver2 &operator=(const CDriver2 &);

public:
	const DataFileManager &m_InputFiles;
	char szInDir[iMaxPath + 1];
	TIdent iConfiguredCustomerCount;
	TIdent iActiveCustomerCount;
	INT32 iScaleFactor;
	INT32 iDaysOfInitialTrades;
	UINT32 iSeed;
	char szBHaddr[iMaxHostname + 1];
	int iBHlistenPort;
	int iUsers;
	int iPacingDelay;
	char outputDirectory[iMaxPath + 1];

	CDriver2(const DataFileManager &, char *, TIdent, TIdent, INT32, INT32,
			UINT32, char *, int, int, int, char *, int);
	~CDriver2();

	void runTest(int, int);
};

#endif // DRIVER2_H

/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Driver2.h"
#include "DBT5Util.h"

// Constructor
CUser::CUser(CDriver2 *pDriver, UINT32 UniqueId, ev_tstamp delay)
: m_pDriver(pDriver), m_UniqueId(UniqueId), m_pCCESUT(NULL), m_pCCE(NULL)
{
	ev_timer_init(&m_Timer, timerCallback, delay, 0.);
	m_Timer.data = this;
	ev_init(&m_Reply, replyCallback);
	m_Reply.data = this;

	ev_timer_start(m_pDriver->m_pLoop, &m_Timer);
}

// Destructor
CUser::~CUser()
{
	ev_timer_stop(m_pDriver->m_pLoop, &m_Timer);
	ev_io_stop(m_pDriver->m_pLoop, &m_Reply);

	delete m_pCCE;
	delete m_pCCESUT;
}

void
CUser::timerCallback(struct ev_loop *loop, ev_timer *w, int revents)
{
	CUser *pUser = reinterpret_cast<CUser *>(w->data);

	// No exception may propagate into the event loop, which is C.
	try {
		// The first expiry starts the user, as creating its thread does
		// in the thread based driver.  Each later one ends a pacing delay.
		if (pUser->m_pCCE == NULL) {
			pUser->start();
		} else if (time(NULL) >= pUser->m_pDriver->m_StopTime) {
			pUser->stop(true);
			return;
		}
		pUser->doTxn();
	} catch (...) {
		pUser->fail();
	}
}

void
CUser::replyCallback(struct ev_loop *loop, ev_io *w, int revents)
{
	CUser *pUser = reinterpret_cast<CUser *>(w->data);

	ev_io_stop(loop, w);

	// No exception may propagate into the event loop, which is C.
	try {
		pUser->m_pCCESUT->receiveReply();
		pUser->pace();
	} catch (...) {
		pUser->fail();
	}
}

void
CUser::start()
{
	// Count the user as started even if starting it fails, as the thread
	// based driver does, so that the end of ramp-up is still logged.
	m_pDriver->userStarted();

	m_pCCESUT = new CCESUT(&m_pDriver->m_fUserMix, &m_pDriver->m_fUserLog,
			m_pDriver->szBHaddr, m_pDriver->iBHlistenPort);
	if (m_pDriver->iSeed == 0) {
		m_pCCE = new CCE(m_pCCESUT, m_pDriver->m_pUserLog,
				m_pDriver->m_InputFiles, m_pDriver->iConfiguredCustomerCount,
				m_pDriver->iActiveCustomerCount, m_pDriver->iScaleFactor,
				m_pDriver->iDaysOfInitialTrades, m_UniqueId);
	} else {
		// Specifying the random number generator seed is considered an
		// invalid run.
		// FIXME: Allow the TxnMixRNGSeed and TxnInputRGNSeed to be set.
		m_pCCE = new CCE(m_pCCESUT, m_pDriver->m_pUserLog,
				m_pDriver->m_InputFiles, m_pDriver->iConfiguredCustomerCount,
				m_pDriver->iActiveCustomerCount, m_pDriver->iScaleFactor,
				m_pDriver->iDaysOfInitialTrades, m_UniqueId, m_pDriver->iSeed,
				m_pDriver->iSeed);
	}
}

// Send the next transaction and wait for its reply
void
CUser::doTxn()
{
	m_pCCE->DoTxn();

	if (m_pCCESUT->replyPending()) {
		// Connecting again after an error can change the socket.
		ev_io_set(&m_Reply, m_pCCESUT->getSocketFd(), EV_READ);
		ev_io_start(m_pDriver->m_pLoop, &m_Reply);
	} else {
		// The request could not be sent, and was logged as an error.  Pace
		// the next one as if it had been sent.
		pace();
	}
}

// wait for pacing delay -- this delay happens after the mix logging
void
CUser::pace()
{
	ev_timer_set(&m_Timer, m_pDriver->iPacingDelay / 1000.0, 0.);
	ev_timer_start(m_pDriver->m_pLoop, &m_Timer);
}

// Stop emulating the user and close its connection.  Log its stop time only
// if it ran to the end of the test, because the first stop time logged ends
// the measurement interval.
void
CUser::stop(bool bLogStopTime)
{
	ev_timer_stop(m_pDriver->m_pLoop, &m_Timer);
	ev_io_stop(m_pDriver->m_pLoop, &m_Reply);

	if (m_pCCESUT == NULL) {
		return;
	}
	if (bLogStopTime) {
		m_pCCESUT->logStopTime();
	}
	m_pCCESUT->biDisconnect();

	cout << "[" << getpid() << "] user " << m_UniqueId << " stopped." << endl;
}

// Log the exception being handled and stop the user, as an error ends the
// thread of a user in the thread based driver.  Call only from a catch
// handler.
void
CUser::fail()
{
	ostringstream osErr;

	osErr << "user " << m_UniqueId << " error: " << caughtExceptionText()
		  << endl;
	m_pDriver->logErrorMessage(osErr.str());

	stop(false);
}

// Constructor
CDriver2::CDriver2(const DataFileManager &inputFiles, char *szInDir,
		TIdent iConfiguredCustomerCount, TIdent iActiveCustomerCount,
		INT32 iScaleFactor, INT32 iDaysOfInitialTrades, UINT32 iSeed,
		char *szBHaddr, int iBHlistenPort, int iUsers, int iPacingDelay,
		char *outputDirectory, int iProcessesPerProcessor)
: m_pLog(NULL), m_iProcessesPerProcessor(iProcessesPerProcessor),
  m_StopTime(0), m_pCDMSUT(NULL), m_pCDM(NULL), m_pLoop(NULL),
  m_pUserLog(NULL), m_iUsersStarted(0), m_InputFiles(inputFiles)
{
	strncpy(this->szInDir, szInDir, iMaxPath);
	this->szInDir[iMaxPath] = '\0';
	this->iConfiguredCustomerCount = iConfiguredCustomerCount;
	this->iActiveCustomerCount = iActiveCustomerCount;
	this->iScaleFactor = iScaleFactor;
	this->iDaysOfInitialTrades = iDaysOfInitialTrades;
	this->iSeed = iSeed;
	strncpy(this->szBHaddr, szBHaddr, iMaxHostname);
	this->szBHaddr[iMaxHostname] = '\0';
	this->iBHlistenPort = iBHlistenPort;
	this->iUsers = iUsers;
	this->iPacingDelay = iPacingDelay;
	strncpy(this->outputDirectory, outputDirectory, iMaxPath);
	this->outputDirectory[iMaxPath] = '\0';

	char filename[iMaxPath + 1];
	snprintf(filename, sizeof(filename), "%s/Driver.log", outputDirectory);
	m_pLog = new CEGenLogger(eDriverEGenLoader, 0, filename, &m_fmt);

	snprintf(filename, sizeof(filename), "%s/Driver_Error.log",
			outputDirectory);
	m_fLog.open(filename, ios::out);

	cout << "initializing data maintenance..." << endl;

	// initialize DMSUT interface
	m_pCDMSUT = new CDMSUT(outputDirectory, szBHaddr, iBHlistenPort);

	// initialize DM - Data Maintenance
	pid_t pid = getpid();
	if (iSeed == 0) {
		m_pCDM = new CDM(m_pCDMSUT, m_pLog, inputFiles,
				iConfiguredCustomerCount, iActiveCustomerCount, iScaleFactor,
				iDaysOfInitialTrades, pid);
	} else {
		// Specifying the random number generator seed is considered an
		// invalid run.
		m_pCDM = new CDM(m_pCDMSUT, m_pLog, inputFiles,
				iConfiguredCustomerCount, iActiveCustomerCount, iScaleFactor,
				iDaysOfInitialTrades, pid, iSeed);
	}
}

// Destructor
CDriver2::~CDriver2()
{
	for (size_t i = 0; i < m_Users.size(); i++) {
		delete m_Users[i];
	}
	delete m_pUserLog;

	delete m_pCDM;
	delete m_pCDMSUT;

	m_fLog.close();

	delete m_pLog;
}

// RunTest
void
CDriver2::runTest(int iSleep, int iTestDuration)
{
	// before starting the test run Trade-Cleanup transaction
	cout << endl
		 << "Running Trade-Cleanup transaction before starting the test..."
		 << endl;
	m_pCDM->DoCleanupTxn();
	cout << "Trade-Cleanup transaction completed." << endl << endl;

	// Calculate when the test should stop, the same way as the thread based
	// driver, which starts one user every iSleep milliseconds.
	int threads_start_time
			= (int) ((double) iSleep / 1000.0 * (double) iUsers);
	m_StopTime = time(NULL) + iTestDuration + threads_start_time;

	vector<int> processors = availableProcessors();
	int iProcesses = (int) processors.size() * m_iProcessesPerProcessor;
	if (iProcesses > iUsers) {
		iProcesses = iUsers;
	}

	CDateTime dtAux;

	cout << "Test is starting at " << dtAux.ToStr(02) << endl
		 << "Estimated duration of ramp-up: " << threads_start_time
		 << " seconds" << endl;

	dtAux.AddMinutes((iTestDuration + threads_start_time) / 60);
	cout << "Estimated end time " << dtAux.ToStr(02) << endl;

	cout << processors.size() << " processor(s) available" << endl
		 << "starting " << iProcesses << " user process(es) for " << iUsers
		 << " user(s)" << endl;

	cout << ">> Start of ramp-up." << endl;

	// The processes start their users on a shared schedule that begins
	// now.
	ev_tstamp start = ev_time();
	vector<pid_t> children;
	for (int i = 0; i < iProcesses; i++) {
		pid_t pid = forkOnProcessor(processors[i % processors.size()]);
		if (pid == 0) {
			// Exit instead of returning into main(), so the child does not
			// destroy its copies of the parent's objects.
			int rc = 1;
			try {
				rc = runUsers(i, iProcesses, start, iSleep);
			} catch (...) {
				cerr << "[" << getpid() << "] error: " << caughtExceptionText()
					 << endl;
			}
			exit(rc);
		} else if (pid == -1) {
			ostringstream osErr;
			osErr << "cannot fork user process: " << strerror(errno) << endl;
			logErrorMessage(osErr.str());

			// Do not run a test with only some of the users.
			for (size_t j = 0; j < children.size(); j++) {
				kill(children[j], SIGKILL);
				waitpid(children[j], NULL, 0);
			}
			return;
		}
		children.push_back(pid);
	}

	runDataMaintenance();

	// wait until all user processes quit
	for (size_t i = 0; i < children.size(); i++) {
		int status = 0;
		pid_t rc;
		do {
			rc = waitpid(children[i], &status, 0);
		} while (rc == -1 && errno == EINTR);
		if (rc == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			ostringstream osErr;
			osErr << "user process " << children[i] << " did not exit normally"
				  << endl;
			logErrorMessage(osErr.str());
		}
	}
}

// The parent process runs the Data-Maintenance transaction as the thread
// based driver's Data-Maintenance thread does.
void
CDriver2::runDataMaintenance()
{
	// The Data-Maintenance transaction must run once per minute.  If a
	// transaction takes more than a minute, remaining goes negative and
	// the next transaction starts immediately.
	time_t start_time;
	time_t end_time;
	time_t remaining;
	try {
		do {
			start_time = time(NULL);
			m_pCDM->DoTxn();
			end_time = time(NULL);
			remaining = 60 - (end_time - start_time);
			if (end_time < m_StopTime && remaining > 0)
				sleep(remaining);
		} while (end_time < m_StopTime);

		m_pCDMSUT->logStopTime();
	} catch (CBaseErr *pErr) {
		// Log and let the users finish the test.
		ostringstream osErr;
		osErr << "Data-Maintenance error: " << pErr->ErrorText() << endl;
		logErrorMessage(osErr.str());
		delete pErr;
	} catch (std::exception &e) {
		ostringstream osErr;
		osErr << "Data-Maintenance error: " << e.what() << endl;
		logErrorMessage(osErr.str());
	}

	cout << "Data-Maintenance stopped." << endl;
}

// Emulate the users of one forked process.  User i runs in process
// (i - 1) % iProcesses and starts (i - 1) * iSleep milliseconds after start,
// so all processes ramp up at the same time, and the users start at the same
// rate as with the thread based driver.
int
CDriver2::runUsers(int iProcess, int iProcesses, ev_tstamp start, int iSleep)
{
	pid_t pid = getpid();
	char filename[iMaxPath + 1];

	snprintf(filename, sizeof(filename), "%s/Customer_%d.log", outputDirectory,
			pid);
	m_pUserLog = new CEGenLogger(eDriverEGenLoader, 0, filename, &m_fmt);
	snprintf(filename, sizeof(filename), "%s/mix-ce-%d.log", outputDirectory,
			pid);
	m_fUserMix.open(filename, ios::out);
	snprintf(filename, sizeof(filename), "%s/error-ce-%d.log", outputDirectory,
			pid);
	m_fUserLog.open(filename, ios::out);

	m_pLoop = EV_DEFAULT;
	ev_now_update(m_pLoop);

	for (int i = iProcess + 1; i <= iUsers; i += iProcesses) {
		ev_tstamp delay
				= start + (double) (i - 1) * iSleep / 1000.0 - ev_now(m_pLoop);
		m_Users.push_back(new CUser(this, i, delay > 0. ? delay : 0.));
	}
	cout << "[" << pid << "] starting " << m_Users.size() << " user(s)"
		 << endl;

	// Returns once every user has stopped.
	ev_run(m_pLoop, 0);

	for (size_t i = 0; i < m_Users.size(); i++) {
		delete m_Users[i];
	}
	m_Users.clear();
	delete m_pUserLog;
	m_pUserLog = NULL;

	m_fUserMix.close();
	m_fUserLog.close();
	return 0;
}

// Mark the end of ramp-up of this process once its last user started.
void
CDriver2::userStarted()
{
	if (++m_iUsersStarted < m_Users.size()) {
		return;
	}

	m_fUserMix << (int) time(NULL) << ",START,,," << getpid() << endl;
	cout << "[" << getpid() << "] >> End of ramp-up." << endl;
}

// logErrorMessage
void
CDriver2::logErrorMessage(const string sErr)
{
	cout << sErr;
	m_fLog << sErr;
	m_fLog.flush();
}

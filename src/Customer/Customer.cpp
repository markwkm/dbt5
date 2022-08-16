/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong
 *
 * 03 August 2006
 */

#include "Customer.h"

#include "CE.h"

// Constructor
CCustomer::CCustomer(char *szInDir,
		TIdent iConfiguredCustomerCount, TIdent iActiveCustomerCount,
		INT32 iScaleFactor, INT32 iDaysOfInitialTrades, UINT32 iSeed,
		char *szBHaddr, int iBHlistenPort, int iUsers, int iPacingDelay,
		char *outputDirectory, ofstream *m_fMix, CMutex *m_MixLock)
: m_iUsers(iUsers), m_iPacingDelay(iPacingDelay)
{
	char filename[iMaxPath + 1];
	snprintf(filename, iMaxPath, "%s/Customer_%lld.log", outputDirectory,
			(long long) pthread_self());
	m_pLog = new CEGenLogger(eDriverEGenLoader, 0, filename, &m_fmt);
	m_pDriverCETxnSettings = new TDriverCETxnSettings;
	dfm = new DataFileManager(szInDir, iConfiguredCustomerCount,
			iActiveCustomerCount);

	snprintf(filename, iMaxPath, "%s/Customer_Error_%lld.log", outputDirectory,
			(long long) pthread_self());
	m_fLog.open(filename, ios::out);

	// initialize CESUT interface
	m_pCCESUT = new CCESUT(szBHaddr, iBHlistenPort, &m_fLog, m_fMix,
			&m_LogLock, m_MixLock);

	// initialize CE - Customer Emulator
	if (iSeed == 0) {
		m_pCCE = new CCE(m_pCCESUT, m_pLog, *dfm, iConfiguredCustomerCount,
				iActiveCustomerCount, iScaleFactor, iDaysOfInitialTrades,
				pthread_self(), m_pDriverCETxnSettings);
	} else {
		// Specifying the random number generator seed is considered an
		// invalid run.
		// FIXME: Allow the TxnMixRNGSeed and TxnInputRGNSeed to be set.
		m_pCCE = new CCE(m_pCCESUT, m_pLog, *dfm, iConfiguredCustomerCount,
				iActiveCustomerCount, iScaleFactor, iDaysOfInitialTrades,
				pthread_self(), iSeed, iSeed, m_pDriverCETxnSettings);
	}
}

// Destructor
CCustomer::~CCustomer()
{
	delete m_pCCE;
	delete m_pCCESUT;

	m_fLog.close();

	delete m_pDriverCETxnSettings;
	delete m_pLog;
}

void CCustomer::DoTxn()
{
	m_pCCE->DoTxn();
}

// LogErrorMessage
void CCustomer::LogErrorMessage(const string sErr)
{
	m_LogLock.lock();
	cout<<sErr;
	m_fLog<<sErr;
	m_fLog.flush();
	m_LogLock.unlock();
}

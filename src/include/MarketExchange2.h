/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 *
 * The Market Exchange driver, with an event loop instead of a thread per
 * Brokerage House connection and a timer thread.
 */

#ifndef MARKET_EXCHANGE2_H
#define MARKET_EXCHANGE2_H

#include <ev.h>

#include "EGenLogFormatterTab.h"
#include "EGenLogger.h"

#include "CSocket.h"
#include "MEESUT2.h"
using namespace TPCE;

class CMarketExchange2
{
private:
	struct TConnection;

	UINT32 m_UniqueId;
	int m_iListenPort;
	CSocket m_Socket;
	CLogFormatTab m_fmt;
	CEGenLogger *m_pLog;
	bool m_Verbose;
	struct ev_loop *m_pLoop;
	CMEESUT2 *m_pCMEESUT;

	// Becomes ready when a Brokerage House connects.
	ev_io m_Accept;
	// Expires with the next pending MEE timer, which fires deferred
	// Trade-Result and triggered Market-Feed processing.
	ev_timer m_Timer;

	static void acceptCallback(struct ev_loop *, ev_io *, int);
	static void requestCallback(struct ev_loop *, ev_io *, int);
	static void timerCallback(struct ev_loop *, ev_timer *, int);

	void requestReady(TConnection *);
	void closeConnection(TConnection *);
	void runMEE(PTradeRequest);
	void setTimer(INT32);

	// non-copyable
	CMarketExchange2(const CMarketExchange2 &);
	CMarketExchange2 &operator=(const CMarketExchange2 &);

public:
	CMEE *m_pCMEE;

	CMarketExchange2(const DataFileManager &, char *, UINT32, TIdent, TIdent,
			int, char *, int, char *, bool, int);
	~CMarketExchange2();

	void startListener(void);
	bool verbose();
};

#endif // MARKET_EXCHANGE2_H

/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 *
 * MEE (Market Exchange Emulator) - SUT Interface class for an event loop
 */

#ifndef MEE_SUT2_H
#define MEE_SUT2_H

#include <ev.h>

#include <deque>
#include <vector>
using namespace std;

#include "MEESUTInterface.h"
#include "MEE.h"

#include "BaseInterface.h"
using namespace TPCE;

// Queues the Trade-Result and Market-Feed transactions that the MEE
// generates, and sends each to the Brokerage House on a connection that is
// not waiting for a reply.  The thread based MEESUT sends each from a thread
// of its own instead.
class CMEESUT2: public CMEESUTInterface
{
private:
	struct TConnection
	{
		ev_io io; // becomes ready when the Brokerage House replies
		CBaseInterface *pInterface;
		CMEESUT2 *pMEESUT;
		bool bBusy; // waiting for a reply
	};

	struct ev_loop *m_pLoop;
	ofstream m_fMix; // mix log file
	ofstream m_fLog; // error log file
	vector<TConnection *> m_Connections;
	deque<TMsgDriverBrokerage> m_Requests; // not sent yet

	static void replyCallback(struct ev_loop *, ev_io *, int);

	void logErrorMessage(const string);

	// non-copyable
	CMEESUT2(const CMEESUT2 &);
	CMEESUT2 &operator=(const CMEESUT2 &);

public:
	CMEESUT2(struct ev_loop *, char *, char *, const int, int);
	~CMEESUT2();

	bool TradeResult(PTradeResultTxnInput);
	bool MarketFeed(PMarketFeedTxnInput);

	void sendRequests();
};

#endif // MEE_SUT2_H

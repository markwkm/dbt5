/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 *
 * The Brokerage House, executing transactions with an event loop in each of
 * several worker processes instead of a thread per driver connection.
 */

#ifndef BROKERAGE_HOUSE2_H
#define BROKERAGE_HOUSE2_H

#include <ev.h>

#include "BrokerageHouse.h"

class CBrokerageHouseWorker;

class CBrokerageHouse2: public CBrokerageHouse
{
private:
	struct TConnection;

	int m_iProcessesPerProcessor;
	struct ev_loop *m_pLoop;

	// The listener passes each connection it accepts to the worker processes
	// through this socket pair.  It sends on the first socket, and every
	// worker process receives on the second.
	int m_Pass[2];

	// Listener: becomes ready when a driver connects.
	ev_io m_Accept;

	// Worker process: becomes ready when the listener passes a connection.
	ev_io m_Receive;
	// Worker process: its database connection and transactions.
	CBrokerageHouseWorker *m_pWorker;

	static void acceptCallback(struct ev_loop *, ev_io *, int);
	static void receiveCallback(struct ev_loop *, ev_io *, int);
	static void requestCallback(struct ev_loop *, ev_io *, int);

	void runWorker();
	void addConnection(int);
	void closeConnection(TConnection *);
	void requestReady(TConnection *);

	// non-copyable
	CBrokerageHouse2(const CBrokerageHouse2 &);
	CBrokerageHouse2 &operator=(const CBrokerageHouse2 &);

public:
	CBrokerageHouse2(const char *, const char *, const char *, const char *,
			const char *, const int, char *, int, bool, int);
	~CBrokerageHouse2();

	void startListener(void);
};

#endif // BROKERAGE_HOUSE2_H

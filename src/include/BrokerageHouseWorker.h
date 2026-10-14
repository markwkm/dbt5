/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 *
 * The database connection, Market Exchange Emulator connection, and
 * transaction harness objects that execute the transaction requests the
 * Brokerage House receives.
 */

#ifndef BROKERAGE_HOUSE_WORKER_H
#define BROKERAGE_HOUSE_WORKER_H

#include "BrokerageHouse.h"
#include "CommonStructs.h"
#include "DBConnection.h"
#include "TxnHarnessSendToMarket.h"

#include "BrokerVolumeDB.h"
#include "CustomerPositionDB.h"
#include "DataMaintenanceDB.h"
#include "MarketFeedDB.h"
#include "MarketWatchDB.h"
#include "SecurityDetailDB.h"
#include "TradeCleanupDB.h"
#include "TradeLookupDB.h"
#include "TradeOrderDB.h"
#include "TradeResultDB.h"
#include "TradeStatusDB.h"
#include "TradeUpdateDB.h"

class CBrokerageHouseWorker
{
private:
	CBrokerageHouse *m_pBrokerageHouse;

	// Connected before the database, because connecting to the market
	// exchange throws on failure and would leak a database connection
	// opened first.
	CSendToMarket m_SendToMarket;
	CDBConnection *m_pDBConnection;

	CMarketFeedDB m_MarketFeedDB;
	CMarketFeed m_MarketFeed;
	CTradeOrderDB m_TradeOrderDB;
	CTradeOrder m_TradeOrder;
	CBrokerVolumeDB m_BrokerVolumeDB;
	CBrokerVolume m_BrokerVolume;
	CCustomerPositionDB m_CustomerPositionDB;
	CCustomerPosition m_CustomerPosition;
	CMarketWatchDB m_MarketWatchDB;
	CMarketWatch m_MarketWatch;
	CSecurityDetailDB m_SecurityDetailDB;
	CSecurityDetail m_SecurityDetail;
	CTradeLookupDB m_TradeLookupDB;
	CTradeLookup m_TradeLookup;
	CTradeStatusDB m_TradeStatusDB;
	CTradeStatus m_TradeStatus;
	CTradeUpdateDB m_TradeUpdateDB;
	CTradeUpdate m_TradeUpdate;
	CDataMaintenanceDB m_DataMaintenanceDB;
	CDataMaintenance m_DataMaintenance;
	CTradeCleanupDB m_TradeCleanupDB;
	CTradeCleanup m_TradeCleanup;
	CTradeResultDB m_TradeResultDB;
	CTradeResult m_TradeResult;

	static CDBConnection *newDBConnection(CBrokerageHouse *);

	// non-copyable
	CBrokerageHouseWorker(const CBrokerageHouseWorker &);
	CBrokerageHouseWorker &operator=(const CBrokerageHouseWorker &);

public:
	CBrokerageHouseWorker(CBrokerageHouse *, char *, char *);
	~CBrokerageHouseWorker();

	INT32 runTransaction(PMsgDriverBrokerage);
};

#endif // BROKERAGE_HOUSE_WORKER_H

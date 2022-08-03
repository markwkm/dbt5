/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// loader class factory for PostgreSQL.
// This class instantiates particular table loader classes.
//

#ifndef CUSTOM_LOAD_STDAFX_H
#define CUSTOM_LOAD_STDAFX_H

#ifdef PGSQL
#include "pgsql/PGSQLLoad.h"
#endif // PGSQL

namespace TPCE
{

class CCustomLoaderFactory : public CBaseLoaderFactory
{
protected:
	char szConnectStr[iConnectStrLen + 1];

public:
	// Constructor
	CCustomLoaderFactory(char *szLoaderParms) {
		assert(szLoaderParms);

		strncpy(szConnectStr, szLoaderParms, iConnectStrLen);
	}

	// Functions to create loader classes for individual tables.

	virtual CBaseLoader<ACCOUNT_PERMISSION_ROW>
			*CreateAccountPermissionLoader() {
		return new CPGSQLAccountPermissionLoad(szConnectStr);
	};

	virtual CBaseLoader<ADDRESS_ROW> *CreateAddressLoader() {
		return new CPGSQLAddressLoad(szConnectStr);
	};

	virtual CBaseLoader<BROKER_ROW> *CreateBrokerLoader() {
		return new CPGSQLBrokerLoad(szConnectStr);
	};

	virtual CBaseLoader<CASH_TRANSACTION_ROW> *CreateCashTransactionLoader() {
		return new CPGSQLCashTransactionLoad(szConnectStr);
	};

	virtual CBaseLoader<CHARGE_ROW> *CreateChargeLoader() {
		return new CPGSQLChargeLoad(szConnectStr);
	};

	virtual CBaseLoader<COMMISSION_RATE_ROW> *CreateCommissionRateLoader() {
		return new CPGSQLCommissionRateLoad(szConnectStr);
	};

	virtual CBaseLoader<COMPANY_COMPETITOR_ROW>
			*CreateCompanyCompetitorLoader() {
		return new CPGSQLCompanyCompetitorLoad(szConnectStr);
	};

	virtual CBaseLoader<COMPANY_ROW> *CreateCompanyLoader() {
		return new CPGSQLCompanyLoad(szConnectStr);
	};

	virtual CBaseLoader<CUSTOMER_ACCOUNT_ROW> *CreateCustomerAccountLoader() {
		return new CPGSQLCustomerAccountLoad(szConnectStr);
	};

	virtual CBaseLoader<CUSTOMER_ROW> *CreateCustomerLoader() {
		return new CPGSQLCustomerLoad(szConnectStr);
	};

	virtual CBaseLoader<CUSTOMER_TAXRATE_ROW> *CreateCustomerTaxrateLoader() {
		return new CPGSQLCustomerTaxRateLoad(szConnectStr);
	};

	virtual CBaseLoader<DAILY_MARKET_ROW> *CreateDailyMarketLoader() {
		return new CPGSQLDailyMarketLoad(szConnectStr);
	};

	virtual CBaseLoader<EXCHANGE_ROW> *CreateExchangeLoader() {
		return new CPGSQLExchangeLoad(szConnectStr);
	};

	virtual CBaseLoader<FINANCIAL_ROW> *CreateFinancialLoader() {
		return new CPGSQLFinancialLoad(szConnectStr);
	};

	virtual CBaseLoader<HOLDING_ROW> *CreateHoldingLoader() {
		return new CPGSQLHoldingLoad(szConnectStr);
	};

	virtual CBaseLoader<HOLDING_HISTORY_ROW> *CreateHoldingHistoryLoader() {
		return new CPGSQLHoldingHistoryLoad(szConnectStr);
	};

	virtual CBaseLoader<HOLDING_SUMMARY_ROW> *CreateHoldingSummaryLoader() {
		return new CPGSQLHoldingSummaryLoad(szConnectStr);
	};

	virtual CBaseLoader<INDUSTRY_ROW> *CreateIndustryLoader() {
		return new CPGSQLIndustryLoad(szConnectStr);
	};

	virtual CBaseLoader<LAST_TRADE_ROW> *CreateLastTradeLoader() {
		return new CPGSQLLastTradeLoad(szConnectStr);
	};

	virtual CBaseLoader<NEWS_ITEM_ROW> *CreateNewsItemLoader() {
		return new CPGSQLNewsItemLoad(szConnectStr);
	};

	virtual CBaseLoader<NEWS_XREF_ROW> *CreateNewsXRefLoader() {
		return new CPGSQLNewsXRefLoad(szConnectStr);
	};

	virtual CBaseLoader<SECTOR_ROW> *CreateSectorLoader() {
		return new CPGSQLSectorLoad(szConnectStr);
	};

	virtual CBaseLoader<SECURITY_ROW> *CreateSecurityLoader() {
		return new CPGSQLSecurityLoad(szConnectStr);
	};

	virtual CBaseLoader<SETTLEMENT_ROW> *CreateSettlementLoader() {
		return new CPGSQLSettlementLoad(szConnectStr);
	};

	virtual CBaseLoader<STATUS_TYPE_ROW> *CreateStatusTypeLoader() {
		return new CPGSQLStatusTypeLoad(szConnectStr);
	};

	virtual CBaseLoader<TAX_RATE_ROW> *CreateTaxRateLoader() {
		return new CPGSQLTaxRateLoad(szConnectStr);
	};

	virtual CBaseLoader<TRADE_HISTORY_ROW> *CreateTradeHistoryLoader() {
		return new CPGSQLTradeHistoryLoad(szConnectStr);
	};

	virtual CBaseLoader<TRADE_ROW> *CreateTradeLoader() {
		return new CPGSQLTradeLoad(szConnectStr);
	};

	virtual CBaseLoader<TRADE_REQUEST_ROW> *CreateTradeRequestLoader() {
		return new CPGSQLTradeRequestLoad(szConnectStr);
	};

	virtual CBaseLoader<TRADE_TYPE_ROW> *CreateTradeTypeLoader() {
		return new CPGSQLTradeTypeLoad(szConnectStr);
	};

	virtual CBaseLoader<WATCH_ITEM_ROW> *CreateWatchItemLoader() {
		return new CPGSQLWatchItemLoad(szConnectStr);
	};

	virtual CBaseLoader<WATCH_LIST_ROW> *CreateWatchListLoader() {
		return new CPGSQLWatchListLoad(szConnectStr);
	};

	virtual CBaseLoader<ZIP_CODE_ROW> *CreateZipCodeLoader() {
		return new CPGSQLZipCodeLoad(szConnectStr);
	};
};

} // namespace TPCE

#endif // CUSTOM_LOAD_STDAFX_H

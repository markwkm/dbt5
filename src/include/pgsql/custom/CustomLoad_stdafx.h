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

#include "pgsql/PGSQLLoad.h"

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

	CBaseLoader<ACCOUNT_PERMISSION_ROW> *CreateAccountPermissionLoader() {
		return new CPGSQLAccountPermissionLoad(szConnectStr);
	};

	CBaseLoader<ADDRESS_ROW> *CreateAddressLoader() {
		return new CPGSQLAddressLoad(szConnectStr);
	};

	CBaseLoader<BROKER_ROW> *CreateBrokerLoader() {
		return new CPGSQLBrokerLoad(szConnectStr);
	};

	CBaseLoader<CASH_TRANSACTION_ROW> *CreateCashTransactionLoader() {
		return new CPGSQLCashTransactionLoad(szConnectStr);
	};

	CBaseLoader<CHARGE_ROW> *CreateChargeLoader() {
		return new CPGSQLChargeLoad(szConnectStr);
	};

	CBaseLoader<COMMISSION_RATE_ROW> *CreateCommissionRateLoader() {
		return new CPGSQLCommissionRateLoad(szConnectStr);
	};

	CBaseLoader<COMPANY_COMPETITOR_ROW>
			*CreateCompanyCompetitorLoader() {
		return new CPGSQLCompanyCompetitorLoad(szConnectStr);
	};

	CBaseLoader<COMPANY_ROW> *CreateCompanyLoader() {
		return new CPGSQLCompanyLoad(szConnectStr);
	};

	CBaseLoader<CUSTOMER_ACCOUNT_ROW> *CreateCustomerAccountLoader() {
		return new CPGSQLCustomerAccountLoad(szConnectStr);
	};

	CBaseLoader<CUSTOMER_ROW> *CreateCustomerLoader() {
		return new CPGSQLCustomerLoad(szConnectStr);
	};

	CBaseLoader<CUSTOMER_TAXRATE_ROW> *CreateCustomerTaxrateLoader() {
		return new CPGSQLCustomerTaxRateLoad(szConnectStr);
	};

	CBaseLoader<DAILY_MARKET_ROW> *CreateDailyMarketLoader() {
		return new CPGSQLDailyMarketLoad(szConnectStr);
	};

	CBaseLoader<EXCHANGE_ROW> *CreateExchangeLoader() {
		return new CPGSQLExchangeLoad(szConnectStr);
	};

	CBaseLoader<FINANCIAL_ROW> *CreateFinancialLoader() {
		return new CPGSQLFinancialLoad(szConnectStr);
	};

	CBaseLoader<HOLDING_ROW> *CreateHoldingLoader() {
		return new CPGSQLHoldingLoad(szConnectStr);
	};

	CBaseLoader<HOLDING_HISTORY_ROW> *CreateHoldingHistoryLoader() {
		return new CPGSQLHoldingHistoryLoad(szConnectStr);
	};

	CBaseLoader<HOLDING_SUMMARY_ROW> *CreateHoldingSummaryLoader() {
		return new CPGSQLHoldingSummaryLoad(szConnectStr);
	};

	CBaseLoader<INDUSTRY_ROW> *CreateIndustryLoader() {
		return new CPGSQLIndustryLoad(szConnectStr);
	};

	CBaseLoader<LAST_TRADE_ROW> *CreateLastTradeLoader() {
		return new CPGSQLLastTradeLoad(szConnectStr);
	};

	CBaseLoader<NEWS_ITEM_ROW> *CreateNewsItemLoader() {
		return new CPGSQLNewsItemLoad(szConnectStr);
	};

	CBaseLoader<NEWS_XREF_ROW> *CreateNewsXRefLoader() {
		return new CPGSQLNewsXRefLoad(szConnectStr);
	};

	CBaseLoader<SECTOR_ROW> *CreateSectorLoader() {
		return new CPGSQLSectorLoad(szConnectStr);
	};

	CBaseLoader<SECURITY_ROW> *CreateSecurityLoader() {
		return new CPGSQLSecurityLoad(szConnectStr);
	};

	CBaseLoader<SETTLEMENT_ROW> *CreateSettlementLoader() {
		return new CPGSQLSettlementLoad(szConnectStr);
	};

	CBaseLoader<STATUS_TYPE_ROW> *CreateStatusTypeLoader() {
		return new CPGSQLStatusTypeLoad(szConnectStr);
	};

	CBaseLoader<TAX_RATE_ROW> *CreateTaxRateLoader() {
		return new CPGSQLTaxRateLoad(szConnectStr);
	};

	CBaseLoader<TRADE_HISTORY_ROW> *CreateTradeHistoryLoader() {
		return new CPGSQLTradeHistoryLoad(szConnectStr);
	};

	CBaseLoader<TRADE_ROW> *CreateTradeLoader() {
		return new CPGSQLTradeLoad(szConnectStr);
	};

	CBaseLoader<TRADE_REQUEST_ROW> *CreateTradeRequestLoader() {
		return new CPGSQLTradeRequestLoad(szConnectStr);
	};

	CBaseLoader<TRADE_TYPE_ROW> *CreateTradeTypeLoader() {
		return new CPGSQLTradeTypeLoad(szConnectStr);
	};

	CBaseLoader<WATCH_ITEM_ROW> *CreateWatchItemLoader() {
		return new CPGSQLWatchItemLoad(szConnectStr);
	};

	CBaseLoader<WATCH_LIST_ROW> *CreateWatchListLoader() {
		return new CPGSQLWatchListLoad(szConnectStr);
	};

	CBaseLoader<ZIP_CODE_ROW> *CreateZipCodeLoader() {
		return new CPGSQLZipCodeLoad(szConnectStr);
	};
};

} // namespace TPCE

#endif // CUSTOM_LOAD_STDAFX_H

/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for TRADE_REQUEST table.
//

#ifndef PGSQL_TRADE_REQUEST_LOAD_H
#define PGSQL_TRADE_REQUEST_LOAD_H

namespace TPCE
{

class CPGSQLTradeRequestLoad : public CPGSQLLoader<TRADE_REQUEST_ROW>
{
public:
	CPGSQLTradeRequestLoad(const char *szConnectStr,
			const char *szTable = "trade_request")
			: CPGSQLLoader<TRADE_REQUEST_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%" PRId64 "%c%s%c%s%c%d%c%.2f%c%" PRId64 "\n",
				next_record->TR_T_ID, delimiter,
				next_record->TR_TT_ID, delimiter,
				next_record->TR_S_SYMB, delimiter,
				next_record->TR_QTY, delimiter,
				next_record->TR_BID_PRICE, delimiter,
				next_record->TR_B_ID);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_TRADE_REQUEST_LOAD_H

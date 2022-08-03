/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for TRADE_HISTORY table.
//

#ifndef PGSQL_TRADE_HISTORY_LOAD_H
#define PGSQL_TRADE_HISTORY_LOAD_H

namespace TPCE
{

class CPGSQLTradeHistoryLoad : public CPGSQLLoader<TRADE_HISTORY_ROW>
{
private:
	CDateTime th_dts;

public:
	CPGSQLTradeHistoryLoad(const char *szConnectStr,
			const char *szTable = "trade_history")
			: CPGSQLLoader<TRADE_HISTORY_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		th_dts = next_record->TH_DTS;

		fprintf(p, "%" PRId64 "%c%s%c%s\n",
				next_record->TH_T_ID, delimiter,
				th_dts.ToStr(iDateTimeFmt), delimiter,
				next_record->TH_ST_ID);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_TRADE_HISTORY_LOAD_H

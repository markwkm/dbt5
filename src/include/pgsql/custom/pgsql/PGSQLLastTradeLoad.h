/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for LAST TRADE table.
//

#ifndef PGSQL_LAST_TRADE_LOAD_H
#define PGSQL_LAST_TRADE_LOAD_H

namespace TPCE
{

class CPGSQLLastTradeLoad : public CPGSQLLoader<LAST_TRADE_ROW>
{
private:
	CDateTime lt_dts;

public:
	CPGSQLLastTradeLoad(const char *szConnectStr,
			const char *szTable = "last_trade")
			: CPGSQLLoader<LAST_TRADE_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const LAST_TRADE_ROW& next_record) {
		lt_dts = next_record.LT_DTS;

		fprintf(p, "%s%c%s%c%.2f%c%.2f%c%" PRId64 "\n",
				next_record.LT_S_SYMB, delimiter,
				lt_dts.ToStr(iDateTimeFmt), delimiter,
				next_record.LT_PRICE, delimiter,
				next_record.LT_OPEN_PRICE, delimiter,
				next_record.LT_VOL);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_LAST_TRADE_LOAD_H

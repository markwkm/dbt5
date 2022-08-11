/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// PGSQL Database loader class for DAILY_MARKET table.
//

#ifndef PGSQL_DAILY_MARKET_LOAD_H
#define PGSQL_DAILY_MARKET_LOAD_H

namespace TPCE
{

class CPGSQLDailyMarketLoad : public CPGSQLLoader<DAILY_MARKET_ROW>
{
private:
	CDateTime dm_date;

public:
	CPGSQLDailyMarketLoad(const char *szConnectStr,
			const char *szTable = "daily_market")
			: CPGSQLLoader<DAILY_MARKET_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const DAILY_MARKET_ROW& next_record) {
		dm_date = next_record.DM_DATE;

		fprintf(p, "%s%c%s%c%.2f%c%.2f%c%.2f%c%" PRId64 "\n",
				dm_date.ToStr(iDateTimeFmt), delimiter,
				next_record.DM_S_SYMB, delimiter,
				next_record.DM_CLOSE, delimiter,
				next_record.DM_HIGH, delimiter,
				next_record.DM_LOW, delimiter,
				next_record.DM_VOL);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_DAILY_MARKET_LOAD_H

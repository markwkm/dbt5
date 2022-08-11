/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for SECURITY table.
//

#ifndef PGSQL_SECURITY_LOAD_H
#define PGSQL_SECURITY_LOAD_H

namespace TPCE
{

class CPGSQLSecurityLoad : public CPGSQLLoader<SECURITY_ROW>
{
private:
	CDateTime s_start_date;
	CDateTime s_exch_date;
	CDateTime s_52wk_high_date;
	CDateTime s_52wk_low_date;

public:
	CPGSQLSecurityLoad(const char *szConnectStr,
			const char *szTable = "security")
			: CPGSQLLoader<SECURITY_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const SECURITY_ROW& next_record) {
		s_start_date = next_record.S_START_DATE;
		s_exch_date = next_record.S_EXCH_DATE;
		s_52wk_high_date = next_record.S_52WK_HIGH_DATE;
		s_52wk_low_date = next_record.S_52WK_LOW_DATE;

		fprintf(p,
				"%s%c%s%c%s%c%s%c%s%c%" PRId64 "%c%" PRId64 "%c%s%c%s%c%.2f%c%.2f%c%s%c%.2f%c%s%c%.2f%c%.2f\n",
				next_record.S_SYMB, delimiter,
				next_record.S_ISSUE, delimiter,
				next_record.S_ST_ID, delimiter,
				next_record.S_NAME, delimiter,
				next_record.S_EX_ID, delimiter,
				next_record.S_CO_ID, delimiter,
				next_record.S_NUM_OUT, delimiter,
				s_start_date.ToStr(iDateTimeFmt), delimiter,
				s_exch_date.ToStr(iDateTimeFmt), delimiter,
				next_record.S_PE, delimiter,
				next_record.S_52WK_HIGH, delimiter,
				s_52wk_high_date.ToStr(iDateTimeFmt), delimiter,
				next_record.S_52WK_LOW, delimiter,
				s_52wk_low_date.ToStr(iDateTimeFmt), delimiter,
				next_record.S_DIVIDEND, delimiter,
				next_record.S_YIELD);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_SECURITY_LOAD_H

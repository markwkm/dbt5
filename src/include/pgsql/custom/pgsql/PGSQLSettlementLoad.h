/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for SETTLEMENT table.
//

#ifndef PGSQL_SETTLEMENT_LOAD_H
#define PGSQL_SETTLEMENT_LOAD_H

namespace TPCE
{

class CPGSQLSettlementLoad : public CPGSQLLoader<SETTLEMENT_ROW>
{
private:
	CDateTime se_cash_due_date;

public:
	CPGSQLSettlementLoad(const char *szConnectStr,
			const char *szTable = "settlement")
			: CPGSQLLoader<SETTLEMENT_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const SETTLEMENT_ROW& next_record) {
		se_cash_due_date = next_record.SE_CASH_DUE_DATE;
		fprintf(p, "%" PRId64 "%c%s%c%s%c%.2f\n",
				next_record.SE_T_ID, delimiter,
				next_record.SE_CASH_TYPE, delimiter,
				se_cash_due_date.ToStr(iDateTimeFmt), delimiter,
				next_record.SE_AMT);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_SETTLEMENT_LOAD_H

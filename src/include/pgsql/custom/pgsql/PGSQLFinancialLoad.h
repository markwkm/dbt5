/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

/*
 * Database loader class for FINANCIAL table.
 */

#ifndef PGSQL_FINANCIAL_LOAD_H
#define PGSQL_FINANCIAL_LOAD_H

namespace TPCE
{

class CPGSQLFinancialLoad : public CPGSQLLoader<FINANCIAL_ROW>
{
private:
	CDateTime fi_qtr_start_date;

public:
	CPGSQLFinancialLoad(const char *szConnectStr,
			const char *szTable = "financial")
			: CPGSQLLoader<FINANCIAL_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fi_qtr_start_date = next_record->FI_QTR_START_DATE;

		fprintf(p,
				"%" PRId64 "%c%d%c%d%c%s%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%ld%c%ld\n",
				next_record->FI_CO_ID, delimiter,
				next_record->FI_YEAR, delimiter,
				next_record->FI_QTR, delimiter,
				fi_qtr_start_date.ToStr(iDateTimeFmt), delimiter,
				next_record->FI_REVENUE, delimiter,
				next_record->FI_NET_EARN, delimiter,
				next_record->FI_BASIC_EPS, delimiter,
				next_record->FI_DILUT_EPS, delimiter,
				next_record->FI_MARGIN, delimiter,
				next_record->FI_INVENTORY, delimiter,
				next_record->FI_ASSETS, delimiter,
				next_record->FI_LIABILITY, delimiter,
				next_record->FI_OUT_BASIC, delimiter,
				next_record->FI_OUT_DILUT);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_FINANCIAL_LOAD_H

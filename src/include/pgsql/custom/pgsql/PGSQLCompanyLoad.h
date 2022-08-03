/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

/*
 * Database loader class for COMPANY table.
 */

#ifndef PGSQL_COMPANY_LOAD_H
#define PGSQL_COMPANY_LOAD_H

namespace TPCE
{

class CPGSQLCompanyLoad : public CPGSQLLoader<COMPANY_ROW>
{
private:
	CDateTime co_open_date;

public:
	CPGSQLCompanyLoad(const char *szConnectStr, const char *szTable = "company")
			: CPGSQLLoader<COMPANY_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		co_open_date = next_record->CO_OPEN_DATE;

		fprintf(p, "%" PRId64 "%c%s%c%s%c%s%c%s%c%s%c%" PRId64 "%c%s%c%s\n",
				next_record->CO_ID, delimiter,
				next_record->CO_ST_ID, delimiter,
				next_record->CO_NAME, delimiter,
				next_record->CO_IN_ID, delimiter,
				next_record->CO_SP_RATE, delimiter,
				next_record->CO_CEO, delimiter,
				next_record->CO_AD_ID, delimiter,
				next_record->CO_DESC, delimiter,
				co_open_date.ToStr(iDateTimeFmt));
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_COMPANY_LOAD_H

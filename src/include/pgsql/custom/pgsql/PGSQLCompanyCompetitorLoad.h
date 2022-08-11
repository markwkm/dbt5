/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for COMPANY_COMPETITOR table.
//

#ifndef PGSQL_COMPANY_COMPETITOR_LOAD_H
#define PGSQL_COMPANY_COMPETITOR_LOAD_H

namespace TPCE
{

class CPGSQLCompanyCompetitorLoad : public CPGSQLLoader<COMPANY_COMPETITOR_ROW>
{
public:
	CPGSQLCompanyCompetitorLoad(const char *szConnectStr,
			const char *szTable = "company_competitor")
			: CPGSQLLoader<COMPANY_COMPETITOR_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const COMPANY_COMPETITOR_ROW& next_record) {
		fprintf(p, "%" PRId64 "%c%" PRId64 "%c%s\n",
				next_record.CP_CO_ID, delimiter,
				next_record.CP_COMP_CO_ID, delimiter,
				next_record.CP_IN_ID);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_COMPANY_COMPETITOR_LOAD_H

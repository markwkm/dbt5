/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for INDUSTRY table.
//

#ifndef PGSQL_INDUSTRY_LOAD_H
#define PGSQL_INDUSTRY_LOAD_H

namespace TPCE
{

class CPGSQLIndustryLoad : public CPGSQLLoader<INDUSTRY_ROW>
{
public:
	CPGSQLIndustryLoad(const char *szConnectStr,
			const char *szTable = "industry")
			: CPGSQLLoader<INDUSTRY_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%s%c%s%c%s\n",
				next_record->IN_ID, delimiter,
				next_record->IN_NAME, delimiter,
				next_record->IN_SC_ID);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_INDUSTRY_LOAD_H

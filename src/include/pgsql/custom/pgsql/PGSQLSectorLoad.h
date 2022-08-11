/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for SECTOR table.
//

#ifndef PGSQL_SECTOR_LOAD_H
#define PGSQL_SECTOR_LOAD_H

namespace TPCE
{

class CPGSQLSectorLoad : public CPGSQLLoader<SECTOR_ROW>
{
public:
	CPGSQLSectorLoad(const char *szConnectStr, const char *szTable = "sector")
			: CPGSQLLoader<SECTOR_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const SECTOR_ROW& next_record) {
		fprintf(p, "%s%c%s\n",
				next_record.SC_ID, delimiter,
				next_record.SC_NAME);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_SECTOR_LOAD_H

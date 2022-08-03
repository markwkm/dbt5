/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for ZIP_CODE table.
//

#ifndef PGSQL_ZIP_CODE_LOAD_H
#define PGSQL_ZIP_CODE_LOAD_H

namespace TPCE
{

class CPGSQLZipCodeLoad : public CPGSQLLoader<ZIP_CODE_ROW>
{
public:
	CPGSQLZipCodeLoad(const char *szConnectStr,
			const char *szTable = "zip_code")
			: CPGSQLLoader<ZIP_CODE_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%s%c%s%c%s\n",
				next_record->ZC_CODE, delimiter,
				next_record->ZC_TOWN, delimiter,
				next_record->ZC_DIV);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_ZIP_CODE_LOAD_H

/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for STATUS_TYPE table.
//

#ifndef PGSQL_STATUS_TYPE_LOAD_H
#define PGSQL_STATUS_TYPE_LOAD_H

namespace TPCE
{

class CPGSQLStatusTypeLoad : public CPGSQLLoader<STATUS_TYPE_ROW>
{
public:
	CPGSQLStatusTypeLoad(const char *szConnectStr,
			const char *szTable = "status_type")
			: CPGSQLLoader<STATUS_TYPE_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%s%c%s\n",
				next_record->ST_ID, delimiter,
				next_record->ST_NAME);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_STATUS_TYPE_LOAD_H

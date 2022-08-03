/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for WATCH_LIST table.
//

#ifndef PGSQL_WATCH_LIST_LOAD_H
#define PGSQL_WATCH_LIST_LOAD_H

namespace TPCE
{

class CPGSQLWatchListLoad : public CPGSQLLoader<WATCH_LIST_ROW>
{
public:
	CPGSQLWatchListLoad(const char *szConnectStr,
			const char *szTable = "watch_list")
			: CPGSQLLoader<WATCH_LIST_ROW>(szConnectStr, szTable) { };

	//copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%" PRId64 "%c%" PRId64 "\n",
				next_record->WL_ID, delimiter,
				next_record->WL_C_ID);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_WATCH_LIST_LOAD_H

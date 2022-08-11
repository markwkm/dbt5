/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for WATCH_ITEM table.
//

#ifndef PGSQL_WATCH_ITEM_LOAD_H
#define PGSQL_WATCH_ITEM_LOAD_H

namespace TPCE
{

class CPGSQLWatchItemLoad : public CPGSQLLoader<WATCH_ITEM_ROW>
{
public:
	CPGSQLWatchItemLoad(const char *szConnectStr,
			const char *szTable = "watch_item")
			: CPGSQLLoader<WATCH_ITEM_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const WATCH_ITEM_ROW& next_record) {
		fprintf(p, "%" PRId64 "%c%s\n",
				next_record.WI_WL_ID, delimiter,
				next_record.WI_S_SYMB);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_WATCH_ITEM_LOAD_H

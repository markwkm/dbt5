/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for HOLDING_HISTORY table.
//

#ifndef HOLDING_HISTORY_LOAD_H
#define HOLDING_HISTORY_LOAD_H

namespace TPCE
{

class CPGSQLHoldingHistoryLoad : public CPGSQLLoader<HOLDING_HISTORY_ROW>
{
public:
	CPGSQLHoldingHistoryLoad(const char *szConnectStr,
			const char *szTable = "holding_history")
			: CPGSQLLoader<HOLDING_HISTORY_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%" PRId64 "%c%" PRId64 "%c%d%c%d\n",
				next_record->HH_H_T_ID, delimiter,
				next_record->HH_T_ID, delimiter,
				next_record->HH_BEFORE_QTY, delimiter,
				next_record->HH_AFTER_QTY);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // HOLDING_HISTORY_LOAD_H

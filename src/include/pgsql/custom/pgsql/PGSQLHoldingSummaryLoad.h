/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

#ifndef PGSQL_HOLDING_SUMMARY_LOAD_H
#define PGSQL_HOLDING_SUMMARY_LOAD_H

namespace TPCE
{

class CPGSQLHoldingSummaryLoad : public CPGSQLLoader<HOLDING_SUMMARY_ROW>
{
public:
	CPGSQLHoldingSummaryLoad(const char *szConnectStr,
			const char *szTable = "holding_summary")
			: CPGSQLLoader<HOLDING_SUMMARY_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%" PRId64 "%c%s%c%d\n",
				next_record->HS_CA_ID, delimiter,
				next_record->HS_S_SYMB, delimiter,
				next_record->HS_QTY);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_HOLDING_SUMMARY_LOAD_H

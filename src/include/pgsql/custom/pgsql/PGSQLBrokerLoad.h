/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for BROKER table.
//

#ifndef PGSQL_BROKER_LOAD_H
#define PGSQL_BROKER_LOAD_H

namespace TPCE
{

class CPGSQLBrokerLoad : public CPGSQLLoader<BROKER_ROW>
{
public:
	CPGSQLBrokerLoad(char *szConnectStr, const char *szTable = "broker")
			: CPGSQLLoader<BROKER_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const BROKER_ROW& next_record) {
		fprintf(p, "%" PRId64 "%c%s%c%s%c%d%c%.2f\n",
				next_record.B_ID, delimiter,
				next_record.B_ST_ID, delimiter,
				next_record.B_NAME, delimiter,
				next_record.B_NUM_TRADES, delimiter,
				next_record.B_COMM_TOTAL);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_BROKER_LOAD_H

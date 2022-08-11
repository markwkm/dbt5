/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for COMMISSION_RATE table.
//

#ifndef PGSQL_COMMISSION_RATE_LOAD_H
#define PGSQL_COMMISSION_RATE_LOAD_H

namespace TPCE
{

class CPGSQLCommissionRateLoad : public CPGSQLLoader<COMMISSION_RATE_ROW>
{
public:
	CPGSQLCommissionRateLoad(const char *szConnectStr,
			const char *szTable = "commission_rate")
			: CPGSQLLoader<COMMISSION_RATE_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const COMMISSION_RATE_ROW& next_record) {
		fprintf(p, "%d%c%s%c%s%c%d%c%d%c%.2f\n",
				next_record.CR_C_TIER, delimiter,
				next_record.CR_TT_ID, delimiter,
				next_record.CR_EX_ID, delimiter,
				next_record.CR_FROM_QTY, delimiter,
				next_record.CR_TO_QTY, delimiter,
				next_record.CR_RATE);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_COMMISSION_RATE_LOAD_H

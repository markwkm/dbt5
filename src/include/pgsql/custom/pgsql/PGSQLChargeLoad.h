/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for CHARGE table.
//

#ifndef PGSQL_CHARGE_LOAD_H
#define PGSQL_CHARGE_LOAD_H

namespace TPCE
{

class CPGSQLChargeLoad : public CPGSQLLoader<CHARGE_ROW>
{
public:
	CPGSQLChargeLoad(const char *szConnectStr, const char *szTable = "charge")
			: CPGSQLLoader<CHARGE_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%s%c%d%c%.2f\n",
				next_record->CH_TT_ID, delimiter,
				next_record->CH_C_TIER, delimiter,
				next_record->CH_CHRG);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif //PGSQL_CHARGE_LOAD_H

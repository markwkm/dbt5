/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for HOLDING table.
//

#ifndef PGSQL_HOLDING_LOAD_H
#define PGSQL_HOLDING_LOAD_H

namespace TPCE
{

class CPGSQLHoldingLoad : public CPGSQLLoader <HOLDING_ROW>
{
private:
	CDateTime h_dts;

public:
	CPGSQLHoldingLoad(const char *szConnectStr, const char *szTable = "holding")
			: CPGSQLLoader<HOLDING_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const HOLDING_ROW& next_record) {
		h_dts = next_record.H_DTS;
		fprintf(p, "%" PRId64 "%c%" PRId64 "%c%s%c%s%c%.2f%c%d\n",
				next_record.H_T_ID, delimiter,
				next_record.H_CA_ID, delimiter,
				next_record.H_S_SYMB, delimiter,
				h_dts.ToStr(iDateTimeFmt), delimiter,
				next_record.H_PRICE, delimiter,
				next_record.H_QTY);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_HOLDING_LOAD_H

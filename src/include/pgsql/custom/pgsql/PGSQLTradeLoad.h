/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for TRADE table.
//

#ifndef PGSQL_TRADE_LOAD_H
#define PGSQL_TRADE_LOAD_H

namespace TPCE
{

class CPGSQLTradeLoad : public CPGSQLLoader<TRADE_ROW>
{
private:
	CDateTime t_dts;

public:
	CPGSQLTradeLoad(const char *szConnectStr, const char *szTable = "trade")
			: CPGSQLLoader<TRADE_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const TRADE_ROW& next_record) {
		t_dts = next_record.T_DTS;

		fprintf(p,
				"%" PRId64 "%c%s%c%s%c%s%c%d%c%s%c%d%c%.2f%c%" PRId64 "%c%s%c%.2f%c%.2f%c%.2f%c%.2f%c%d\n",
				next_record.T_ID, delimiter,
				t_dts.ToStr(iDateTimeFmt), delimiter,
				next_record.T_ST_ID, delimiter,
				next_record.T_TT_ID, delimiter,
				next_record.T_IS_CASH, delimiter,
				next_record.T_S_SYMB, delimiter,
				next_record.T_QTY, delimiter,
				next_record.T_BID_PRICE, delimiter,
				next_record.T_CA_ID, delimiter,
				next_record.T_EXEC_NAME, delimiter,
				next_record.T_TRADE_PRICE, delimiter,
				next_record.T_CHRG, delimiter,
				next_record.T_COMM, delimiter,
				next_record.T_TAX, delimiter,
				next_record.T_LIFO);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}

	virtual void FinishLoad() {
		// FIXME: Can't we call the parent class's FinishLoad to do the COMMIT?
		// End of the COPY.
		fprintf(p, "\\.\n");
		// FIXME: Have blind faith that COPY was successful.
		while (fgetc(p) != EOF) ;

		// COMMIT the COPY.
		fprintf(p, "COMMIT;\n");
		// FIXME: Have blind faith that COMMIT was successful.
		while (fgetc(p) != EOF) ;

		fprintf(p,
				"SELECT SETVAL('seq_trade_id', (SELECT MAX(t_id) FROM trade));\n");
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_TRADE_LOAD_H

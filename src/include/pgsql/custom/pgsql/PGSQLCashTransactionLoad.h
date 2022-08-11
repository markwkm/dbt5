/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for CASH_TRANSACTION table.
//

#ifndef PGSQL_CASH_TRANSACTION_LOAD_H
#define PGSQL_CASH_TRANSACTION_LOAD_H

namespace TPCE
{

class CPGSQLCashTransactionLoad : public CPGSQLLoader<CASH_TRANSACTION_ROW>
{
private:
	CDateTime ct_dts;
public:
	CPGSQLCashTransactionLoad(const char *szConnectStr,
			const char *szTable = "cash_transaction")
			: CPGSQLLoader<CASH_TRANSACTION_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const CASH_TRANSACTION_ROW& next_record) {
		ct_dts = next_record.CT_DTS;
		fprintf(p, "%" PRId64 "%c%s%c%.2f%c%s\n",
				next_record.CT_T_ID, delimiter,
				ct_dts.ToStr(iDateTimeFmt), delimiter,
				next_record.CT_AMT, delimiter,
				next_record.CT_NAME);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif //PGSQL_CASH_TRANSACTION_LOAD_H

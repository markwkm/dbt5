/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for CUSTOMER_ACCOUNT table.
//

#ifndef PGSQL_CUSTOMER_ACCOUNT_LOAD_H
#define PGSQL_CUSTOMER_ACCOUNT_LOAD_H

namespace TPCE
{

class CPGSQLCustomerAccountLoad : public CPGSQLLoader<CUSTOMER_ACCOUNT_ROW>
{
public:
	CPGSQLCustomerAccountLoad(const char *szConnectStr,
			const char *szTable = "customer_account")
			: CPGSQLLoader<CUSTOMER_ACCOUNT_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%" PRId64 "%c%" PRId64 "%c%" PRId64 "%c%s%c%d%c%.2f\n",
				next_record->CA_ID, delimiter,
				next_record->CA_B_ID, delimiter,
				next_record->CA_C_ID, delimiter,
				next_record->CA_NAME, delimiter,
				next_record->CA_TAX_ST, delimiter,
				next_record->CA_BAL);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_CUSTOMER_ACCOUNT_LOAD_H

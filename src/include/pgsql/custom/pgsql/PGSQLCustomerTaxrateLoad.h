/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for CUSTOMER_TAXRATE table.
//

#ifndef PGSQL_CUSTOMER_TAXRATE_LOAD_H
#define PGSQL_CUSTOMER_TAXRATE_LOAD_H

namespace TPCE
{

class CPGSQLCustomerTaxRateLoad : public CPGSQLLoader<CUSTOMER_TAXRATE_ROW>
{
public:
	CPGSQLCustomerTaxRateLoad(const char *szConnectStr,
			const char *szTable = "customer_taxrate")
			: CPGSQLLoader<CUSTOMER_TAXRATE_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%s%c%" PRId64 "\n",
				next_record->CX_TX_ID, delimiter,
				next_record->CX_C_ID);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_CUSTOMER_TAXRATE_LOAD_H

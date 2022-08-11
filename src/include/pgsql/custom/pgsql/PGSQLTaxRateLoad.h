/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for TAXRATE table.
//

#ifndef PGSQL_TAXRATE_LOAD_H
#define PGSQL_TAXRATE_LOAD_H

namespace TPCE
{

class CPGSQLTaxRateLoad : public CPGSQLLoader<TAX_RATE_ROW>
{
public:
	CPGSQLTaxRateLoad(const char *szConnectStr, const char *szTable = "taxrate")
			: CPGSQLLoader<TAX_RATE_ROW>(szConnectStr, szTable) { };

	//copy to the bound location inside this class first
	void WriteNextRecord(const TAX_RATE_ROW& next_record) {
		fprintf(p, "%s%c%s%c%.5f\n",
				next_record.TX_ID, delimiter,
				next_record.TX_NAME, delimiter,
				next_record.TX_RATE);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_TAXRATE_LOAD_H

/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for ADDRESS table.
//

#ifndef PGSQL_ADDRESS_LOAD_H
#define PGSQL_ADDRESS_LOAD_H

namespace TPCE
{

class CPGSQLAddressLoad : public CPGSQLLoader<ADDRESS_ROW>
{
public:
	CPGSQLAddressLoad(const char *szConnectStr, const char *szTable = "address")
			: CPGSQLLoader<ADDRESS_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%" PRId64 "%c%s%c%s%c%s%c%s\n",
				next_record->AD_ID, delimiter,
				next_record->AD_LINE1, delimiter,
				next_record->AD_LINE2, delimiter,
				next_record->AD_ZC_CODE, delimiter,
				next_record->AD_CTRY);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_ADDRESS_LOAD_H

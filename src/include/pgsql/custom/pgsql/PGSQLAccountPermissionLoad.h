/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// PostgreSQL loader class for ACCOUNT_PERMISSION table.
//

#ifndef PGSQL_ACCOUNT_PERMISSION_LOAD_H
#define PGSQL_ACCOUNT_PERMISSION_LOAD_H

namespace TPCE
{

class CPGSQLAccountPermissionLoad : public CPGSQLLoader<ACCOUNT_PERMISSION_ROW>
{
public:
	CPGSQLAccountPermissionLoad(const char *szConnectStr,
			const char *szTable = "account_permission")
			: CPGSQLLoader<ACCOUNT_PERMISSION_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%" PRId64 "%c%s%c%s%c%s%c%s\n",
				next_record->AP_CA_ID, delimiter,
				next_record->AP_ACL, delimiter,
				next_record->AP_TAX_ID, delimiter,
				next_record->AP_L_NAME, delimiter,
				next_record->AP_F_NAME);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_ACCOUNT_PERMISSION_LOAD_H

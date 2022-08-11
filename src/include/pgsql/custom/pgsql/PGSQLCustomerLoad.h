/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

/*
 * Database loader class fot this table.
 */

#ifndef PGSQL_CUSTOMER_LOAD_H
#define PGSQL_CUSTOMER_LOAD_H

namespace TPCE
{

class CPGSQLCustomerLoad : public CPGSQLLoader<CUSTOMER_ROW>
{
private:
	CDateTime c_dob;

public:
	CPGSQLCustomerLoad(const char *szConnectStr,
			const char *szTable = "customer")
			: CPGSQLLoader<CUSTOMER_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const CUSTOMER_ROW& next_record) {
		c_dob = next_record.C_DOB;

		fprintf(p,
				"%" PRId64 "%c%s%c%s%c%s%c%s%c%s%c%c%c%d%c%s%c%" PRId64 "%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s\n",
				next_record.C_ID, delimiter,
				next_record.C_TAX_ID, delimiter,
				next_record.C_ST_ID, delimiter,
				next_record.C_L_NAME, delimiter,
				next_record.C_F_NAME, delimiter,
				next_record.C_M_NAME, delimiter,
				next_record.C_GNDR, delimiter,
				next_record.C_TIER, delimiter,
				c_dob.ToStr(iDateTimeFmt), delimiter,
				next_record.C_AD_ID, delimiter,
				next_record.C_CTRY_1, delimiter,
				next_record.C_AREA_1, delimiter,
				next_record.C_LOCAL_1, delimiter,
				next_record.C_EXT_1, delimiter,
				next_record.C_CTRY_2, delimiter,
				next_record.C_AREA_2, delimiter,
				next_record.C_LOCAL_2, delimiter,
				next_record.C_EXT_2, delimiter,
				next_record.C_CTRY_3, delimiter,
				next_record.C_AREA_3, delimiter,
				next_record.C_LOCAL_3, delimiter,
				next_record.C_EXT_3, delimiter,
				next_record.C_EMAIL_1, delimiter,
				next_record.C_EMAIL_2);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_CUSTOMER_LOAD_H

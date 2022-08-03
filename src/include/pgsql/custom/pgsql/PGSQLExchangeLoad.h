/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010      Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for EXCHANGE table.
//

#ifndef PGSQL_EXCHANGE_LOAD_H
#define PGSQL_EXCHANGE_LOAD_H

namespace TPCE
{

class CPGSQLExchangeLoad : public CPGSQLLoader<EXCHANGE_ROW>
{
public:
	CPGSQLExchangeLoad(const char *szConnectStr,
			const char *szTable = "exchange")
			: CPGSQLLoader<EXCHANGE_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	virtual void WriteNextRecord(PT next_record) {
		fprintf(p, "%s%c%s%c%d%c%d%c%d%c%s%c%" PRId64 "\n",
				next_record->EX_ID, delimiter,
				next_record->EX_NAME, delimiter,
				next_record->EX_NUM_SYMB, delimiter,
				next_record->EX_OPEN, delimiter,
				next_record->EX_CLOSE, delimiter,
				next_record->EX_DESC, delimiter,
				next_record->EX_AD_ID);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_EXCHANGE_LOAD_H

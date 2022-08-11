/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for TRADE_TYPE table.
//

#ifndef PGSQL_TRADE_TYPE_LOAD_H
#define PGSQL_TRADE_TYPE_LOAD_H

namespace TPCE
{

class CPGSQLTradeTypeLoad : public CPGSQLLoader<TRADE_TYPE_ROW>
{
public:
	CPGSQLTradeTypeLoad(const char *szConnectStr,
			const char *szTable = "trade_type")
			: CPGSQLLoader<TRADE_TYPE_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const TRADE_TYPE_ROW& next_record) {
		fprintf(p, "%s%c%s%c%s%c%s\n",
				next_record.TT_ID, delimiter,
				next_record.TT_NAME, delimiter,
				next_record.TT_IS_SELL ? "TRUE" : "FALSE" , delimiter,
				next_record.TT_IS_MRKT ? "TRUE" : "FALSE");
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	}
};

} // namespace TPCE

#endif // PGSQL_TRADE_TYPE_LOAD_H

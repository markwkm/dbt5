/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for NEWS_ITEM table.
//

#ifndef PGSQL_NEWS_ITEM_LOAD_H
#define PGSQL_NEWS_ITEM_LOAD_H

namespace TPCE
{

class CPGSQLNewsItemLoad : public CPGSQLLoader<NEWS_ITEM_ROW>
{
private:
	CDateTime ni_dts;

public:
	CPGSQLNewsItemLoad(const char *szConnectStr,
			const char *szTable = "news_item")
			: CPGSQLLoader<NEWS_ITEM_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const NEWS_ITEM_ROW& next_record) {
		ni_dts = next_record.NI_DTS;
		fprintf(p, "%" PRId64 "%c%s%c%s%c%s%c%s%c%s%c%s\n",
				next_record.NI_ID, delimiter,
				next_record.NI_HEADLINE, delimiter,
				next_record.NI_SUMMARY, delimiter,
				next_record.NI_ITEM, delimiter,
				ni_dts.ToStr(iDateTimeFmt), delimiter,
				next_record.NI_SOURCE, delimiter,
				next_record.NI_AUTHOR);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	};
};

} // namespace TPCE

#endif // PGSQL_NEWS_ITEM_LOAD_H

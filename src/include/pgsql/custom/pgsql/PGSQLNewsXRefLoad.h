/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006      Rilson Nascimento
 *               2010-2022 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for NEWS_XREF table.
//

#ifndef PGSQL_NEWS_XREF_LOAD_H
#define PGSQL_NEWS_XREF_LOAD_H

namespace TPCE
{

class CPGSQLNewsXRefLoad : public CPGSQLLoader<NEWS_XREF_ROW>
{
public:
	CPGSQLNewsXRefLoad(const char *szConnectStr,
			const char *szTable = "news_xref")
			: CPGSQLLoader<NEWS_XREF_ROW>(szConnectStr, szTable) { };

	// copy to the bound location inside this class first
	void WriteNextRecord(const NEWS_XREF_ROW& next_record) {
		fprintf(p, "%" PRId64 "%c%" PRId64 "\n",
				next_record.NX_NI_ID, delimiter,
				next_record.NX_CO_ID);
		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF) ;
	};
};

} // namespace TPCE

#endif // PGSQL_NEWS_XREF_LOAD_H

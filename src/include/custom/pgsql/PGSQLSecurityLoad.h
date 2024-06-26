/*
 * Legal Notice
 *
 * This document and associated source code (the "Work") is a part of a
 * benchmark specification maintained by the TPC.
 *
 * The TPC reserves all right, title, and interest to the Work as provided
 * under U.S. and international laws, including without limitation all patent
 * and trademark rights therein.
 *
 * No Warranty
 *
 * 1.1 TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THE INFORMATION
 *     CONTAINED HEREIN IS PROVIDED "AS IS" AND WITH ALL FAULTS, AND THE
 *     AUTHORS AND DEVELOPERS OF THE WORK HEREBY DISCLAIM ALL OTHER
 *     WARRANTIES AND CONDITIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
 *     INCLUDING, BUT NOT LIMITED TO, ANY (IF ANY) IMPLIED WARRANTIES,
 *     DUTIES OR CONDITIONS OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
 *     PURPOSE, OF ACCURACY OR COMPLETENESS OF RESPONSES, OF RESULTS, OF
 *     WORKMANLIKE EFFORT, OF LACK OF VIRUSES, AND OF LACK OF NEGLIGENCE.
 *     ALSO, THERE IS NO WARRANTY OR CONDITION OF TITLE, QUIET ENJOYMENT,
 *     QUIET POSSESSION, CORRESPONDENCE TO DESCRIPTION OR NON-INFRINGEMENT
 *     WITH REGARD TO THE WORK.
 * 1.2 IN NO EVENT WILL ANY AUTHOR OR DEVELOPER OF THE WORK BE LIABLE TO
 *     ANY OTHER PARTY FOR ANY DAMAGES, INCLUDING BUT NOT LIMITED TO THE
 *     COST OF PROCURING SUBSTITUTE GOODS OR SERVICES, LOST PROFITS, LOSS
 *     OF USE, LOSS OF DATA, OR ANY INCIDENTAL, CONSEQUENTIAL, DIRECT,
 *     INDIRECT, OR SPECIAL DAMAGES WHETHER UNDER CONTRACT, TORT, WARRANTY,
 *     OR OTHERWISE, ARISING IN ANY WAY OUT OF THIS OR ANY OTHER AGREEMENT
 *     RELATING TO THE WORK, WHETHER OR NOT SUCH AUTHOR OR DEVELOPER HAD
 *     ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * Contributors
 * - 2006 Rilson Nascimento
 * - 2010 Mark Wong <markwkm@postgresql.org>
 */

//
// Database loader class for SECURITY table.
//

#ifndef PGSQL_SECURITY_LOAD_H
#define PGSQL_SECURITY_LOAD_H

#include "pgloader.h"

namespace TPCE
{

class CPGSQLSecurityLoad: public CPGSQLLoader<SECURITY_ROW>
{
private:
	CDateTime s_start_date;
	CDateTime s_exch_date;
	CDateTime s_52wk_high_date;
	CDateTime s_52wk_low_date;
	const std::string SecurityRowFmt;

public:
	CPGSQLSecurityLoad(
			const char *szConnectStr, const char *szTable = "security")
	: CPGSQLLoader<SECURITY_ROW>(szConnectStr, szTable),
	  SecurityRowFmt("%s|%s|%s|%s|%s|%" PRId64 "|%" PRId64
					 "|%s|%s|%.2f|%.2f|%s|%.2f|%s|%.2f|%.2f\n"){};

	void
	WriteNextRecord(const SECURITY_ROW &next_record)
	{
		s_start_date = next_record.S_START_DATE;
		s_exch_date = next_record.S_EXCH_DATE;
		s_52wk_high_date = next_record.S_52WK_HIGH_DATE;
		s_52wk_low_date = next_record.S_52WK_LOW_DATE;

		int rc = fprintf(p, SecurityRowFmt.c_str(), next_record.S_SYMB,
				next_record.S_ISSUE, next_record.S_ST_ID, next_record.S_NAME,
				next_record.S_EX_ID, next_record.S_CO_ID,
				next_record.S_NUM_OUT, s_start_date.ToStr(iDateTimeFmt),
				s_exch_date.ToStr(iDateTimeFmt), next_record.S_PE,
				next_record.S_52WK_HIGH, s_52wk_high_date.ToStr(iDateTimeFmt),
				next_record.S_52WK_LOW, s_52wk_low_date.ToStr(iDateTimeFmt),
				next_record.S_DIVIDEND, next_record.S_YIELD);

		if (rc < 0) {
			throw CSystemErr(CSystemErr::eWriteFile,
					"CFlatSecurityLoad::WriteNextRecord");
		}

		// FIXME: Have blind faith that this row of data was built correctly.
		while (fgetc(p) != EOF)
			;
	}
};

} // namespace TPCE

#endif // PGSQL_SECURITY_LOAD_H

/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright The DBT-5 Authors
 *
 * Based on TPC-E Standard Specification Revision 1.10.0.
 */

#include <sys/types.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <postgres.h>
#include <fmgr.h>
#include <utils/numeric.h>
#include <executor/spi.h> /* this should include most necessary APIs */
#include <executor/executor.h> /* for GetAttributeByName() */
#include <funcapi.h> /* for returning set of rows in order_status */
#include <miscadmin.h>
#include <utils/date.h>
#include <utils/builtins.h>
#include <utils/datetime.h>
#include <catalog/pg_type.h>

#include "frame.h"
#include "dbt5common.h"

#define BAD_INPUT_DATA 1

#define SQLMWF1_1                                                             \
	"SELECT wi_s_symb\n"                                                      \
	"FROM   watch_item,\n"                                                    \
	"       watch_list\n"                                                     \
	"WHERE  wi_wl_id = wl_id\n"                                               \
	"       AND wl_c_id = $1"

#define SQLMWF1_2                                                             \
	"SELECT s_symb\n"                                                         \
	"FROM   industry,\n"                                                      \
	"       company,\n"                                                       \
	"       security\n"                                                       \
	"WHERE  in_name = $1\n"                                                   \
	"       AND co_in_id = in_id\n"                                           \
	"       AND co_id BETWEEN $2\n"                                           \
	"                         AND $3\n"                                       \
	"       AND s_co_id = co_id"

#define SQLMWF1_3                                                             \
	"SELECT hs_s_symb\n"                                                      \
	"FROM   holding_summary\n"                                                \
	"WHERE  hs_ca_id = $1"

#define SQLMWF1_4                                                             \
	"SELECT lt_price\n"                                                       \
	"FROM   last_trade\n"                                                     \
	"WHERE  lt_s_symb = $1"

#define SQLMWF1_5                                                             \
	"SELECT s_num_out\n"                                                      \
	"FROM   security\n"                                                       \
	"WHERE  s_symb = $1"

#define SQLMWF1_6                                                             \
	"SELECT dm_close\n"                                                       \
	"FROM   daily_market\n"                                                   \
	"WHERE  dm_s_symb = $1\n"                                                 \
	"       AND dm_date = $2"

#define MWF1_1 MWF1_statements[0].plan
#define MWF1_2 MWF1_statements[1].plan
#define MWF1_3 MWF1_statements[2].plan
#define MWF1_4 MWF1_statements[3].plan
#define MWF1_5 MWF1_statements[4].plan
#define MWF1_6 MWF1_statements[5].plan

static cached_statement MWF1_statements[] = {

	{ SQLMWF1_1, 1, { INT8OID } },

	{ SQLMWF1_2, 3, { TEXTOID, INT8OID, INT8OID } },

	{ SQLMWF1_3, 1, { INT8OID } },

	{ SQLMWF1_4, 1, { TEXTOID } },

	{ SQLMWF1_5, 1, { TEXTOID } },

	{ SQLMWF1_6, 2, { TEXTOID, DATEOID } },

	{ NULL }
};

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/* Prototypes to prevent potential gcc warnings. */
Datum MarketWatchFrame1(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(MarketWatchFrame1);

#ifdef DEBUG
void dump_mwf1_inputs(long, long, long, char *, char *, long);

void
dump_mwf1_inputs(long acct_id, long cust_id, long ending_co_id,
		char *industry_name, char *start_date, long starting_co_id)
{
	elog(DEBUG1, "MWF1: INPUTS START");
	elog(DEBUG1, "MWF1: %ld", acct_id);
	elog(DEBUG1, "MWF1: %ld", cust_id);
	elog(DEBUG1, "MWF1: %ld", ending_co_id);
	elog(DEBUG1, "MWF1: '%s'", industry_name);
	elog(DEBUG1, "MWF1: %s", pstrdup(start_date));
	elog(DEBUG1, "MWF1: %ld", starting_co_id);
	elog(DEBUG1, "MWF1: INPUTS END");
}
#endif /* DEBUG */

/* Clause 3.3.1.3 */
Datum
MarketWatchFrame1(PG_FUNCTION_ARGS)
{
	int i;

	int status = 0;
	double old_mkt_cap = 0.0;
	double new_mkt_cap = 0.0;
	double pct_change = 0.0;

	struct pg_tm tt, *tm = &tt;
	int64 acct_id = PG_GETARG_INT64(0);
	int64 cust_id = PG_GETARG_INT64(1);
	int64 ending_co_id = PG_GETARG_INT64(2);
	char *industry_name_p = (char *) PG_GETARG_TEXT_P(3);
	DateADT start_date_p = PG_GETARG_DATEADT(4);
	int64 starting_co_id = PG_GETARG_INT64(5);

	int ret;
	TupleDesc tupdesc;
	SPITupleTable *tuptable = NULL;
	HeapTuple tuple = NULL;

	char buf[MAXDATELEN + 1];
	char industry_name[IN_NAME_LEN + 1];
	Datum args[3];
	char nulls[3] = { ' ', ' ', ' ' };
	int frame_index = 0;
	j2date(start_date_p + POSTGRES_EPOCH_JDATE, &(tm->tm_year), &(tm->tm_mon),
			&(tm->tm_mday));
	EncodeDateOnly(tm, DateStyle, buf);

	strcpy(industry_name, DatumGetCString(DirectFunctionCall1(
								  textout, PointerGetDatum(industry_name_p))));

#ifdef DEBUG
	dump_mwf1_inputs(acct_id, cust_id, ending_co_id, industry_name, buf,
			starting_co_id);
#endif

	SPI_connect();
	plan_queries(MWF1_statements);
	if (cust_id != 0) {
#ifdef DEBUG
		elog(DEBUG1, "%s", SQLMWF1_1);
#endif /* DEBUG */
		frame_index = 0;
		args[0] = Int64GetDatum(cust_id);
		ret = SPI_execute_plan(MWF1_1, args, nulls, true, 0);
	} else if (industry_name[0] != '\0') {
#ifdef DEBUG
		elog(DEBUG1, "%s", SQLMWF1_2);
#endif /* DEBUG */
		frame_index = 1;
		args[0] = CStringGetTextDatum(industry_name);
		args[1] = Int64GetDatum(starting_co_id);
		args[2] = Int64GetDatum(ending_co_id);
		ret = SPI_execute_plan(MWF1_2, args, nulls, true, 0);
	} else if (acct_id != 0) {
#ifdef DEBUG
		elog(DEBUG1, "%s", SQLMWF1_3);
#endif /* DEBUG */
		frame_index = 2;
		args[0] = Int64GetDatum(acct_id);
		ret = SPI_execute_plan(MWF1_3, args, nulls, true, 0);
	} else {
		// assign ret with NOT equl to SPI_OK_SELECT
		// to log failed frame
		ret = 0;
		status = BAD_INPUT_DATA;
	}
	if (ret != SPI_OK_SELECT) {
#ifdef DEBUG
		dump_mwf1_inputs(acct_id, cust_id, ending_co_id, industry_name, buf,
				starting_co_id);
#endif /* DEBUG */
		FAIL_FRAME(MWF1_statements[frame_index].sql);
	}

	if (status != BAD_INPUT_DATA) {
		int count = SPI_processed;

		TupleDesc tupdesc4;
		SPITupleTable *tuptable4 = NULL;
		HeapTuple tuple4 = NULL;

		char *symbol;
		char *new_price;
		char *old_price;
		char *s_num_out;

		tupdesc = SPI_tuptable->tupdesc;
		tuptable = SPI_tuptable;
		for (i = 0; i < count; i++) {
			tuple = tuptable->vals[i];
			symbol = SPI_getvalue(tuple, tupdesc, 1);
#ifdef DEBUG
			elog(DEBUG1, "%s", SQLMWF1_4);
			elog(DEBUG1, "  symbol = '%s'", symbol);
#endif /* DEBUG */
			frame_index = 3;
			args[0] = CStringGetTextDatum(symbol);
			ret = SPI_execute_plan(MWF1_4, args, nulls, true, 0);

			if (ret != SPI_OK_SELECT || SPI_processed == 0) {
#ifdef DEBUG
				dump_mwf1_inputs(acct_id, cust_id, ending_co_id, industry_name,
						buf, starting_co_id);
#endif /* DEBUG */
				FAIL_FRAME(MWF1_statements[frame_index].sql);
				continue;
			}
			tupdesc4 = SPI_tuptable->tupdesc;
			tuptable4 = SPI_tuptable;
			tuple4 = tuptable4->vals[0];
			new_price = SPI_getvalue(tuple4, tupdesc4, 1);
#ifdef DEBUG
			elog(DEBUG1, "%s", SQLMWF1_4);
			elog(DEBUG1, "  new_price  = %s", new_price);
			elog(DEBUG1, "  new_price  = %f", atof(new_price));
#endif /* DEBUG */
			frame_index = 4;
			args[0] = CStringGetTextDatum(symbol);
			ret = SPI_execute_plan(MWF1_5, args, nulls, true, 0);
			if (ret != SPI_OK_SELECT) {
#ifdef DEBUG
				dump_mwf1_inputs(acct_id, cust_id, ending_co_id, industry_name,
						buf, starting_co_id);
#endif /* DEBUG */
				FAIL_FRAME(MWF1_statements[frame_index].sql);
				elog(DEBUG1, "ERROR: sql not ok = %d", ret);
			}
			tupdesc4 = SPI_tuptable->tupdesc;
			tuptable4 = SPI_tuptable;
			tuple4 = tuptable4->vals[0];
			s_num_out = SPI_getvalue(tuple4, tupdesc4, 1);
#ifdef DEBUG
			elog(DEBUG1, "%s", SQLMWF1_6);
			elog(DEBUG1, "  s_num_out  = %s", s_num_out);
#endif /* DEBUG */
			frame_index = 5;
			args[0] = CStringGetTextDatum(symbol);
			args[1] = DirectFunctionCall1(
					date_in, CStringGetDatum(pstrdup(buf)));
			ret = SPI_execute_plan(MWF1_6, args, nulls, true, 0);
			if (ret != SPI_OK_SELECT) {
#ifdef DEBUG
				dump_mwf1_inputs(acct_id, cust_id, ending_co_id, industry_name,
						buf, starting_co_id);
#endif /* DEBUG */
				FAIL_FRAME(MWF1_statements[frame_index].sql);
			}

			if (SPI_processed == 0) {
				elog(NOTICE, "No rows returned for getting old_price.");
			} else {
				tupdesc4 = SPI_tuptable->tupdesc;
				tuptable4 = SPI_tuptable;
				tuple4 = tuptable4->vals[0];
				old_price = SPI_getvalue(tuple4, tupdesc4, 1);
#ifdef DEBUG
				elog(DEBUG1, "  old_price  = %s", old_price);
				elog(DEBUG1, "  old_price  = %f", atof(old_price));
#endif /* DEBUG */
				old_mkt_cap += atof(s_num_out) * atof(old_price);
			}
			new_mkt_cap += atof(s_num_out) * atof(new_price);

#ifdef DEBUG
			elog(DEBUG1, "old_mkt_cap = %f", old_mkt_cap);
			elog(DEBUG1, "new_mkt_cap = %f", new_mkt_cap);
#endif /* DEBUG */
		}
		pct_change = 100.0 * (new_mkt_cap / old_mkt_cap - 1.0);
#ifdef DEBUG
		elog(DEBUG1, "pct_change = %f", pct_change);
#endif /* DEBUG */
	}

#ifdef DEBUG
	elog(DEBUG1, "MWF1 OUT: 1 %f", pct_change);
#endif /* DEBUG */

	SPI_finish();
	PG_RETURN_FLOAT8(pct_change);
}

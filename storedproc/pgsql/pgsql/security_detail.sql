/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006 Rilson Nascimento
 *               2022 Mark Wong
 *
 * Based on TPC-E Standard Specification Revision 1.14.0
 */

CREATE TYPE SECURITY_DETAIL_DAY
AS (
    _date DATE
  , close S_PRICE_T
  , high S_PRICE_T
  , low S_PRICE_T
  , vol S_PRICE_T
);

CREATE TYPE SECURITY_DETAIL_FIN
AS (
    year INTEGER
  , qtr SMALLINT
  , start_date DATE
  , rev FIN_AGG_T
  , net_earn FIN_AGG_T
  , basic_eps VALUE_T
  , dilut_eps VALUE_T
  , margin VALUE_T
  , invent FIN_AGG_T
  , assets FIN_AGG_T
  , liab FIN_AGG_T
  , out_basic S_COUNT_T
  , out_dilut S_COUNT_T
);

CREATE TYPE SECURITY_DETAIL_NEWS
AS (
    item BYTEA
  , dts TIMESTAMP
  , src VARCHAR(30)
  , auth VARCHAR(30)
  , headline VARCHAR(80)
  , summary VARCHAR(255)
);

-- Clause 3.3.5.3
CREATE OR REPLACE FUNCTION SecurityDetailFrame1 (
						IN access_lob_flag	smallint,
						IN max_rows_to_return	integer,
						IN start_day		timestamp,
    IN symbol VARCHAR(15)
  , OUT x52_wk_high DOUBLE PRECISION
  , OUT x52_wk_high_date DATE
  , OUT x52_wk_low DOUBLE PRECISION
  , OUT x52_wk_low_date DATE
  , OUT ceo_name VARCHAR(60)
  , OUT co_ad_ctry VARCHAR(80)
  , OUT co_ad_div VARCHAR(80)
  , OUT co_ad_line1 VARCHAR(80)
  , OUT co_ad_line2 VARCHAR(80)
  , OUT co_ad_town VARCHAR(80)
  , OUT co_ad_zip VARCHAR(12)
  , OUT co_desc VARCHAR(150)
  , OUT co_st_id VARCHAR(4)
  , OUT cp_co_name VARCHAR(60)[3]
  , OUT cp_in_name VARCHAR(50)[3]
  , OUT day SECURITY_DETAIL_DAY[20]
  , OUT day_len INTEGER
  , OUT divid DOUBLE PRECISION
  , OUT ex_ad_ctry VARCHAR(80)
  , OUT ex_ad_div VARCHAR(80)
  , OUT ex_ad_line1 VARCHAR(80)
  , OUT ex_ad_line2 VARCHAR(80)
  , OUT ex_ad_town VARCHAR(80)
  , OUT ex_ad_zip VARCHAR(12)
  , OUT ex_close INTEGER
  , OUT ex_date DATE
  , OUT ex_desc VARCHAR(150)
  , OUT ex_name VARCHAR(100)
  , OUT ex_num_symb INTEGER
  , OUT ex_open INTEGER
  , OUT fin SECURITY_DETAIL_FIN[20]
  , OUT fin_len INTEGER
  , OUT last_open DOUBLE PRECISION
  , OUT last_price DOUBLE PRECISION
  , OUT last_vol INTEGER
  , OUT news SECURITY_DETAIL_NEWS[2]
  , OUT news_len INTEGER
  , OUT num_out BIGINT
  , OUT open_date DATE
  , OUT pe_ratio FLOAT
  , OUT s_name VARCHAR(70)
  , OUT sp_rate VARCHAR(4)
  , OUT yield DOUBLE PRECISION
) RETURNS RECORD AS $$
<<sdf1>>
DECLARE
    co_id IDENT_T;
    co_name VARCHAR(60);
    start_date DATE;
    exch_date DATE;

	-- variables
	rs		RECORD;
	i		integer;
BEGIN
    SELECT security.s_name
         , company.co_id
         , company.co_name
         , company.co_sp_rate
         , company.co_ceo
         , company.co_desc
         , company.co_open_date
         , company.co_st_id
         , ca.ad_line1
         , ca.ad_line2
         , zca.zc_town
         , zca.zc_div
         , ca.ad_zc_code
         , ca.ad_ctry
         , s_num_out
         , s_start_date
         , s_exch_date
         , s_pe
         , s_52wk_high
         , s_52wk_high_date
         , s_52wk_low
         , s_52wk_low_date
         , s_dividend
         , s_yield
         , zea.zc_div
         , ea.ad_ctry
         , ea.ad_line1
         , ea.ad_line2
         , zea.zc_town
         , ea.ad_zc_code
         , exchange.ex_close
         , exchange.ex_desc
         , exchange.ex_name
         , exchange.ex_num_symb
         , exchange.ex_open
    INTO s_name
       , sdf1.co_id
       , co_name
       , sp_rate
       , ceo_name
       , co_desc
       , open_date
       , co_st_id
       , co_ad_line1
       , co_ad_line2
       , co_ad_town
       , co_ad_div
       , co_ad_zip
       , co_ad_ctry
       , num_out
       , start_date
       , exch_date
       , pe_ratio
       , x52_wk_high
       , x52_wk_high_date
       , x52_wk_low
       , x52_wk_low_date
       , divid
       , yield
       , ex_ad_div
       , ex_ad_ctry
       , ex_ad_line1
       , ex_ad_line2
       , ex_ad_town
       , ex_ad_zip
       , ex_close
       , ex_desc
       , ex_name
       , ex_num_symb
       , ex_open
    FROM security
       , company
       , address ca
       , address ea
       , zip_code zca
       , zip_code zea
       , exchange
    WHERE s_symb = symbol
      AND company.co_id = s_co_id
      AND ca.ad_id = co_ad_id
      AND ea.ad_id = exchange.ex_ad_id
      AND exchange.ex_id = s_ex_id
      AND ca.ad_zc_code = zca.zc_code
      AND ea.ad_zc_code = zea.zc_code
    ;

	-- Should return max_comp_len rows

    i := 0;
	FOR rs IN
        SELECT company.co_name,
			IN_NAME
		FROM	COMPANY_COMPETITOR,
			COMPANY,
			INDUSTRY
        WHERE cp_co_id = sdf1.co_id
          AND company.co_id = CP_COMP_CO_ID AND
			IN_ID = CP_IN_ID
		LIMIT 3
	LOOP
        i := i + 1;
		cp_co_name[i] = rs.co_name;
		cp_in_name[i] = rs.in_name;
	END LOOP;

	-- Should return max_fin_len rows
	
	i = 0;
	FOR rs IN
		SELECT	FI_YEAR,
			FI_QTR,
			fi_qtr_start_date,
			FI_REVENUE,
			FI_NET_EARN,
			FI_BASIC_EPS,
			FI_DILUT_EPS,
			FI_MARGIN,
			FI_INVENTORY,
			FI_ASSETS,
			FI_LIABILITY,
			FI_OUT_BASIC,
			FI_OUT_DILUT
		FROM	FINANCIAL
        WHERE FI_CO_ID = sdf1.co_id
		ORDER BY FI_YEAR asc, FI_QTR
		LIMIT 20
	LOOP
		i = i + 1;
        fin[i].year := rs.fi_year;
        fin[i].qtr := rs.fi_qtr;
        fin[i].start_date := rs.fi_qtr_start_date;
        fin[i].rev := rs.fi_revenue;
        fin[i].net_earn := rs.fi_net_earn;
        fin[i].basic_eps := rs.fi_basic_eps;
        fin[i].dilut_eps := rs.fi_dilut_eps;
        fin[i].margin := rs.fi_margin;
        fin[i].invent := rs.fi_inventory;
        fin[i].assets := rs.fi_assets;
        fin[i].liab := rs.fi_liability;
        fin[i].out_basic := rs.fi_out_basic;
        fin[i].out_dilut := rs.fi_out_dilut;
	END LOOP;

	fin_len = i;

	-- Should return max_rows_to_return rows
	
	i = 0;
	FOR rs IN
        SELECT dm_date,
			DM_CLOSE,
			DM_HIGH,
			DM_LOW,
			DM_VOL
		FROM	DAILY_MARKET
		WHERE	DM_S_SYMB = symbol AND
			DM_DATE >= start_day
		ORDER BY DM_DATE asc
		LIMIT max_rows_to_return
	LOOP
		i = i + 1;
        day[i]._date := rs.dm_date;
        day[i].close := rs.dm_close;
        day[i].high := rs.dm_high;
        day[i].low := rs.dm_low;
        day[i].vol := rs.dm_vol;
	END LOOP;

	day_len = i;

	SELECT	LT_PRICE,
		LT_OPEN_PRICE,
		LT_VOL
	INTO	last_price,
		last_open,
		last_vol
	FROM	LAST_TRADE
    WHERE lt_s_symb = symbol;
	
	-- Should return max_news_len rows
	
	i = 0;
	IF access_lob_flag = 1 THEN
		FOR rs IN
			SELECT	NI_ITEM,
                ni_dts,
				NI_SOURCE,
				NI_AUTHOR
			FROM	NEWS_XREF,
				NEWS_ITEM
			WHERE	NI_ID = NX_NI_ID AND
                  NX_CO_ID = sdf1.co_id
			LIMIT 2
		LOOP
			i = i + 1;
            news[i].item := rs.ni_item;
            news[i].dts := rs.ni_dts;
            news[i].src := rs.ni_source;
            news[i].auth := rs.ni_author;
            news[i].headline := '';
            news[i].summary := '';
		END LOOP;
	ELSE
		FOR rs IN
            SELECT ni_dts,
				NI_SOURCE,
				NI_AUTHOR,
				NI_HEADLINE,
				NI_SUMMARY
			FROM	NEWS_XREF,
				NEWS_ITEM
			WHERE	NI_ID = NX_NI_ID AND
                  NX_CO_ID = sdf1.co_id
			LIMIT 2
		LOOP
			i = i + 1;
            news[i].item := '';
            news[i].dts := rs.ni_dts;
            news[i].src := rs.ni_source;
            news[i].auth := rs.ni_author;
            news[i].headline := rs.ni_headline;
            news[i].summary := rs.ni_summary;
		END LOOP;
	END IF;
	
	news_len = i;
END;
$$ LANGUAGE 'plpgsql';

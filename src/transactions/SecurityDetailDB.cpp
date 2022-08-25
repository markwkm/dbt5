/*
 * This file is released under the terms of the Artistic License.  Please see
 * the file LICENSE, included in this package, for details.
 *
 * Copyright (C) 2006-2007 Rilson Nascimento
 *               2010      Mark Wong
 *
 * 15 July 2006
 */

#include "SecurityDetailDB.h"

// Call Security Detail Frame 1
void CSecurityDetailDB::DoSecurityDetailFrame1(
		const TSecurityDetailFrame1Input *pIn,
		TSecurityDetailFrame1Output *pOut)
{
#ifdef DEBUG
	pthread_t pid = pthread_self();
	cout << pid << " <<< SDF1" << endl;
	cout << pid << " - Security Detail Frame 1 (input)" << endl <<
			pid << " -- access_lob_flag: " << pIn->access_lob_flag << endl <<
			pid << " -- max_rows_to_return: " << pIn->max_rows_to_return <<
					endl <<
			pid << " -- start_day: " << pIn->start_day.year << "-" <<
					pIn->start_day.month << "-" << pIn->start_day.day << " " <<
					pIn->start_day.hour << ":" << pIn->start_day.minute <<
					":" << pIn->start_day.second << endl <<
			pid << " -- symbol: "<<pIn->symbol<<endl;
#endif // DEBUG

	startTransaction();
	// Isolation level required by Clause 7.4.1.3
	setReadCommitted();
	execute(pIn, pOut);
	commitTransaction();

#ifdef DEBUG
	cout << pid << " - Security Detail Frame 1 (output)" << endl <<
			pid << " -- fin_len: " << pOut->fin_len << endl <<
			pid << " -- day_len: " << pOut->day_len << endl <<
			pid << " -- news_len: " << pOut->news_len << endl;
	int i;
	for (i = 0; i < 3; i++) {
		cout << pid << " -- cp_co_name[" << i << "]: " <<
						pOut->cp_co_name[i] << endl <<
				pid << " -- cp_in_name[" << i << "]: " << pOut->cp_in_name[i] <<
						endl;
	}
	for (i = 0; i < pOut->fin_len; i++) {
		cout << pid << " -- fin[" << i << "].year: " << pOut->fin[i].year <<
						endl <<
				pid << " -- fin[" << i << "].qtr: " << pOut->fin[i].qtr <<
						endl <<
				pid << " -- fin[" << i << "].start_date: " <<
						pOut->fin[i].start_date.year << "-" <<
						pOut->fin[i].start_date.month << "-" <<
						pOut->fin[i].start_date.day << endl <<
				pid << " -- fin[" << i << "].rev: "<<pOut->fin[i].rev << endl <<
				pid << " -- fin[" << i << "].net_earn: " <<
						pOut->fin[i].net_earn << endl <<
				pid << " -- fin[" << i << "].basic_eps: " <<
						pOut->fin[i].basic_eps << endl <<
				pid << " -- fin[" << i << "].dilut_eps: " <<
						pOut->fin[i].dilut_eps << endl <<
				pid << " -- fin[" << i << "].margin: " <<
						pOut->fin[i].margin << endl <<
				pid << " -- fin[" << i << "].invent: " <<
						pOut->fin[i].invent << endl <<
				pid << " -- fin[" << i << "].assets: " <<
						pOut->fin[i].assets << endl <<
				pid << " -- fin[" << i << "].liab: " << pOut->fin[i].liab <<
						endl <<
				pid << " -- fin[" << i << "].out_basic: " <<
						pOut->fin[i].out_basic << endl <<
				pid << " -- fin[" << i << "].out_dilut: " <<
						pOut->fin[i].out_dilut << endl;
	}
	for (i = 0; i < pOut->day_len; i++) {
		cout << pid << " -- day[" << i << "].date: " <<
					pOut->day[i].date.year << "-" <<
					pOut->day[i].date.month << "-" <<
					pOut->day[i].date.day << endl <<
			pid << " -- day[" << i << "].close: " << pOut->day[i].close <<
					endl <<
			pid << " -- day[" << i << "].high: " << pOut->day[i].high << endl <<
			pid << " -- day[" << i << "].low: " << pOut->day[i].low << endl <<
			pid << " -- day[" << i << "].vol: " << pOut->day[i].vol << endl;
	}
	for (i = 0; i < pOut->news_len; i++) {
		cout << pid << " -- news[" << i << "].item: " << pOut->news[i].item <<
						endl <<
				pid << " -- news[" << i << "].dts: " <<
						pOut->news[i].dts.year << "-" <<
						pOut->news[i].dts.month << "-" <<
						pOut->news[i].dts.day << " " <<
						pOut->news[i].dts.hour << ":" <<
						pOut->news[i].dts.minute << ":" <<
						pOut->news[i].dts.second << endl <<
				pid << " -- news[" << i << "].src: " << pOut->news[i].src <<
						endl <<
				pid << " -- news[" << i << "].auth: " << pOut->news[i].auth <<
						endl <<
				pid << " -- news[" << i << "].headline: " <<
						pOut->news[i].headline << endl <<
				pid << " -- news[" << i << "].summary: " <<
						pOut->news[i].summary << endl;
	}
	cout << pid << " -- last_price: " << pOut->last_price << endl <<
			pid << " -- last_open: " << pOut->last_open << endl <<
			pid << " -- last_vol: " << pOut->last_vol << endl <<
			pid << " -- s_name: " << pOut->s_name << endl <<
			pid << " -- co_name: " << pOut->co_name << endl <<
			pid << " -- sp_rate: " << pOut->sp_rate << endl <<
			pid << " -- ceo_name: " << pOut->ceo_name << endl <<
			pid << " -- co_desc: " << pOut->co_desc << endl <<
			pid << " -- open_date: " << pOut->open_date.year << "-" <<
					pOut->open_date.month << "-" <<
					pOut->open_date.day << " " <<
					pOut->open_date.hour << ":" <<
					pOut->open_date.minute << ":" <<
					pOut->open_date.second << endl <<
			pid << " -- co_st_id: " << pOut->co_st_id << endl <<
			pid << " -- co_ad_line1: " << pOut->co_ad_line1 << endl <<
			pid << " -- co_ad_line2: " << pOut->co_ad_line2 << endl <<
			pid << " -- co_ad_town: " << pOut->co_ad_town << endl <<
			pid << " -- co_ad_div: " << pOut->co_ad_div << endl <<
			pid << " -- co_ad_zip: " << pOut->co_ad_zip << endl <<
			pid << " -- co_ad_cty: " << pOut->co_ad_cty << endl <<
			pid << " -- num_out: " << pOut->num_out << endl <<
			pid << " -- start_date: " << pOut->start_date.year << "-" <<
					pOut->start_date.month << "-" <<
					pOut->start_date.day << " " <<
					pOut->start_date.hour << ":" <<
					pOut->start_date.minute << ":" <<
					pOut->start_date.second << endl <<
			pid << " -- ex_date: " << pOut->ex_date.year << "-" <<
					pOut->ex_date.month << "-" <<
					pOut->ex_date.day << " " << pOut->ex_date.hour <<
					":" << pOut->ex_date.minute << ":" <<
					pOut->ex_date.second << endl <<
			pid << " -- pe_ratio: " << pOut->pe_ratio << endl <<
			pid << " -- s52_wk_high: " << pOut->s52_wk_high << endl <<
			pid << " -- s52_wk_high_date: " << pOut->s52_wk_high_date.year <<
					"-" << pOut->s52_wk_high_date.month << "-" <<
					pOut->s52_wk_high_date.day << " " <<
					pOut->s52_wk_high_date.hour << ":" <<
					pOut->s52_wk_high_date.minute << ":" <<
					pOut->s52_wk_high_date.second << endl <<
			pid << " -- s52_wk_low: " << pOut->s52_wk_low << endl <<
			pid << " -- s52_wk_low_date: " << pOut->s52_wk_low_date.year <<
					"-" << pOut->s52_wk_low_date.month << "-" <<
					pOut->s52_wk_low_date.day << " " <<
					pOut->s52_wk_low_date.hour << ":" <<
					pOut->s52_wk_low_date.minute << ":" <<
					pOut->s52_wk_low_date.second << endl <<
			pid << " -- divid: " << pOut->divid << endl <<
			pid << " -- yield: " << pOut->yield << endl <<
			pid << " -- ex_ad_div: " << pOut->ex_ad_div << endl <<
			pid << " -- ex_ad_cty: " << pOut->ex_ad_cty << endl <<
			pid << " -- ex_ad_line1: " << pOut->ex_ad_line1 << endl <<
			pid << " -- ex_ad_line2: " << pOut->ex_ad_line2 << endl <<
			pid << " -- ex_ad_town: " << pOut->ex_ad_town << endl <<
			pid << " -- ex_ad_zip: " << pOut->ex_ad_zip << endl <<
			pid << " -- ex_close: " << pOut->ex_close << endl <<
			pid << " -- ex_desc: " << pOut->ex_desc << endl <<
			pid << " -- ex_name: " << pOut->ex_name << endl <<
			pid << " -- ex_num_symb: " << pOut->ex_num_symb << endl <<
			pid << " -- ex_open: " << pOut->ex_open << endl;
	cout << pid << " >>> SDF1" << endl;
#endif // DEBUG
}

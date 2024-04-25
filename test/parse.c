#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "at_parse.h"			/* at_parse_*() */
#include "mutils.h"			/* ITEMS_OF() */


int ok = 0;
int faults = 0;

/* We call ast_log from pdu.c, so we'll fake an implementation here. */
void ast_log(int level, const char* fmt, ...)
{
    /* Silence compiler warnings */
    (void)level;
    (void)fmt;
}

#/* */
void test_parse_cnum()
{
	static const struct test_case {
		const char	* input;
		const char	* result;
	} cases[] = {
		{ "+CNUM: \"*Subscriber Number\",\"+79139131234\",145", "+79139131234" },
		{ "+CNUM: \"Subscriber Number\",\"\",145", "" },
		{ "+CNUM: \"Subscriber Number\",,145", "" },
		{ "+CNUM: \"\",\"+79139131234\",145", "+79139131234" },
		{ "+CNUM: ,\"\",145", "" },
		{ "+CNUM: ,,145", "" },
		{ "+CNUM: \"\",+79139131234\",145", "+79139131234" },
		{ "+CNUM: \"\",+79139131234,145", "+79139131234" },
	};
	unsigned idx = 0;
	char * input;
	const char * res;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_cnum", input);
		res = at_parse_cnum(input);
		if(strcmp(res, cases[idx].result) == 0) {
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = \"%s\"\t%s\n", res, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_cops()
{
	static const struct test_case {
		const char	* input;
		const char	* result;
	} cases[] = {
		{ "+COPS: 0,0,\"TELE2\",0", "TELE2" },
		{ "+COPS: 0,0,\"TELE2,0", "TELE2" },
		{ "+COPS: 0,0,TELE2,0", "TELE2" },
	};
	unsigned idx = 0;
	char * input;
	const char * res;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_cops", input);
		res = at_parse_cops(input);
		if(strcmp(res, cases[idx].result) == 0) {
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = \"%s\"\t%s\n", res, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_creg()
{
	struct result {
		int	res;
		int	gsm_reg;
		int	gsm_reg_status;
		char 	* lac;
		char	* ci;
	};
	static const struct test_case {
		const char	* input;
		struct result 	result;
	} cases[] = {
		{ "+CEREG: 2,1,9110,7E6", { 0, 1, 1, "9110", "7E6"} },
		{ "+CEREG: 2,1,XXXX,AAAA", { 0, 1, 1, "XXXX", "AAAA"} },
	};
	unsigned idx = 0;
	char * input;
	struct result result;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_creg", input);
		result.res = at_parse_creg(input, strlen(input), &result.gsm_reg, &result.gsm_reg_status, &result.lac, &result.ci);
		if(result.res == cases[idx].result.res
			&& result.gsm_reg == cases[idx].result.gsm_reg
			&& result.gsm_reg_status == cases[idx].result.gsm_reg_status
			&& strcmp(result.lac, cases[idx].result.lac) == 0
			&& strcmp(result.ci, cases[idx].result.ci) == 0)
		{
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = %d (%d,%d,\"%s\",\"%s\")\t%s\n", result.res, result.gsm_reg, result.gsm_reg_status, result.lac, result.ci, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_cmti()
{
	static const struct test_case {
		const char	* input;
		int		result;
	} cases[] = {
		{ "+CMTI: \"ME\",41", 41 },
		{ "+CMTI: 0,111", 111 },
		{ "+CMTI: ", -1 },
	};
	unsigned idx = 0;
	char * input;
	int result;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_cmti", input);
		result = at_parse_cmti(input);
		if(result == cases[idx].result) {
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = %d\t%s\n", result, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

int safe_strcmp(const char *a, const char *b)
{
	if (a == NULL && b == NULL) {
		return 0;
	}
	if (a == NULL) {
		return -1;
	}
	if (b == NULL) {
		return 1;
	}
	return strcmp(a, b);
}

#/* */
void test_gsm7()
{
	const char *in = "0123456789";
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 7; ++j) {
			char inin[128];
			strncpy(inin, in, i + 1);
			inin[i + 1] = 0;
			uint8_t pdu[256];
			uint16_t buf16[256];
			int res = utf8_to_ucs2(inin, strlen(inin), buf16, 256);
			res = gsm7_encode(buf16, res, buf16);
			int packedsize = gsm7_pack(buf16, res, pdu, res * 2, j);
			hexify(pdu, (packedsize + 1) / 2, pdu);
			res = unhex(pdu, pdu);
			res = gsm7_unpack_decode(pdu, packedsize, buf16, res, j, 0, 0);
			char rev[256];
			res = ucs2_to_utf8(buf16, res, rev, 256);
			rev[res] = 0;
			if (strcmp(rev, inin) == 0) {
				++ok;
			} else {
				++faults;
				printf("%s != %s %d %d\n", inin, rev, i + 1, res);
			}
		}
	}
}

void test_parse_cmt()
{
	struct result {
		char * res;
		char		* str;
		char 		* oa;
		char		* msg;
		char            * msg_utf8;
		char		* sca;
		int tpdu_type;
		pdu_udh_t udh;
		int mr, st;
		char scts[256], dt[256];
		size_t msg_len;
	};
	static const struct test_case {
		const char	* input;
		struct result 	result;
	} cases[] = {
		{ "+CMT: ,103\r\n0891683110304705F02408A001960900000842405202147023569A8C8BC17801FF1A003700380032003200300036002867096548671F4E3A00355206949F0029FF0C8BF760A8786E8BA4662F673A4E3B672C4EBAFF0C5982975E673A4E3B672C4EBA8BF75FFD75656B644FE1606F30",
			{
				0,
				"9A8C8BC17801FF1A003700380032003200300036002867096548671F4E3A00355206949F0029FF0C8BF760A8786E8BA4662F673A4E3B672C4EBAFF0C5982975E673A4E3B672C4EBA8BF75FFD75656B644FE1606F30",
				"10699000",
				"9A8C8BC17801FF1A003700380032003200300036002867096548671F4E3A00355206949F0029FF0C8BF760A8786E8BA4662F673A4E3B672C4EBAFF0C5982975E673A4E3B672C4EBA8BF75FFD75656B644FE1606F30",
				"验证码：782206(有效期为5分钟)，请您确认是机主本人，如非机主本人请忽略此信息"
			}
		},
	};

	unsigned idx = 0;
	char * input;
	struct result result;
	char oa[200], sca[200];
	const char * msg;

	result.oa = oa;
	result.sca = sca;
	for (; idx < ITEMS_OF(cases); ++idx) {
		char buf[4096];
		int failidx = 0;
		result.str = input = strdup(cases[idx].input);
		result.msg_utf8 = buf;

		fprintf(stderr, "/* %u */ %s(\"%s\")...", idx, "at_parse_cmgr", input);
		result.res = at_parse_cmt(
			result.str, strlen(result.str), &result.tpdu_type, &result.sca, sizeof(sca), result.oa, sizeof(oa), result.scts, &result.mr, &result.st, result.dt,
			result.msg_utf8, &result.msg_len, &result.udh);

		if (++failidx && result.res == cases[idx].result.res &&
		    ++failidx && safe_strcmp(result.oa, cases[idx].result.oa) == 0 &&
		    ++failidx && safe_strcmp(result.msg_utf8, cases[idx].result.msg_utf8) == 0)
		{
			msg = "OK";
			ok++;
			failidx = 0;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = '%s' ('%s','%s') [fail@%d]\n[text=%s] %s\n",
			result.res, result.oa,
			result.msg, failidx, result.msg_utf8, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

void test_parse_cmgr()
{
	struct result {
		char * res;
		char		* str;
		char 		* oa;
		char		* msg;
		char            * msg_utf8;
		char		* sca;
		int tpdu_type;
		pdu_udh_t udh;
		int mr, st;
		char scts[256], dt[256];
		size_t msg_len;
	};
	static const struct test_case {
		const char	* input;
		struct result 	result;
	} cases[] = {
		{ "+CMGR: 0,,136\r\n099164000339541902F02409A101568937F10008421060219522237630104E2D56FD75354FE1301160A86B635728767B5F55201C4E2D56FD75354FE1201D004100500050FF0C9A8C8BC178014E3AFF1A003300380038003900340037FF0C8BF7572800335206949F51858F9351653002907F514D969079C16CC49732FF0C9A8C8BC17801520752FF544A77E54ED64EBA",
			{
				0,
				"30104E2D56FD75354FE1301160A86B635728767B5F55201C4E2D56FD75354FE1201D004100500050FF0C9A8C8BC178014E3AFF1A003300380038003900340037FF0C8BF7572800335206949F51858F9351653002907F514D969079C16CC49732FF0C9A8C8BC17801520752FF544A77E54ED64EBA",
				"106598731",
				"30104E2D56FD75354FE1301160A86B635728767B5F55201C4E2D56FD75354FE1201D004100500050FF0C9A8C8BC178014E3AFF1A003300380038003900340037FF0C8BF7572800335206949F51858F9351653002907F514D969079C16CC49732FF0C9A8C8BC17801520752FF544A77E54ED64EBA",
				"【中国电信】您正在登录“中国电信”APP，验证码为：388947，请在3分钟内输入。避免隐私泄露，验证码切勿告知他人"
			}
		},
		{ "+CMGR: 0,,115\r\n099164000339541902F0240BA10186425495F50008421060312452236030108109810930119A8C8BC17801FF1A0033003500380039FF0C8BE59A8C8BC1780175284E8E6CE8518C81098109FF0C8BF752FF6CC4973230026B228FCE4F7F752881098109FF0C5F00542F5DE54F5C597D5FC360C5FF0C4E484E4854D2",
			{
				0,
				"30108109810930119A8C8BC17801FF1A0033003500380039FF0C8BE59A8C8BC1780175284E8E6CE8518C81098109FF0C8BF752FF6CC4973230026B228FCE4F7F752881098109FF0C5F00542F5DE54F5C597D5FC360C5FF0C4E484E4854D2",
				"10682445595",
				"30108109810930119A8C8BC17801FF1A0033003500380039FF0C8BE59A8C8BC1780175284E8E6CE8518C81098109FF0C8BF752FF6CC4973230026B228FCE4F7F752881098109FF0C5F00542F5DE54F5C597D5FC360C5FF0C4E484E4854D2",
				"【脉脉】验证码：3589，该验证码用于注册脉脉，请勿泄露。欢迎使用脉脉，开启工作好心情，么么哒"
			}
		},
	};

	unsigned idx = 0;
	char * input;
	struct result result;
	char oa[200], sca[200];
	const char * msg;

	result.oa = oa;
	result.sca = sca;
	for (; idx < ITEMS_OF(cases); ++idx) {
		char buf[4096];
		int failidx = 0;
		result.str = input = strdup(cases[idx].input);
		result.msg_utf8 = buf;

		fprintf(stderr, "/* %u */ %s(\"%s\")...", idx, "at_parse_cmgr", input);
		result.res = at_parse_cmgr(
			result.str, strlen(result.str), &result.tpdu_type, &result.sca, sizeof(sca), result.oa, sizeof(oa), result.scts, &result.mr, &result.st, result.dt,
			result.msg_utf8, &result.msg_len, &result.udh);

		if (++failidx && result.res == cases[idx].result.res &&
		    ++failidx && safe_strcmp(result.oa, cases[idx].result.oa) == 0 &&
		    ++failidx && safe_strcmp(result.msg_utf8, cases[idx].result.msg_utf8) == 0)
		{
			msg = "OK";
			ok++;
			failidx = 0;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = '%s' ('%s','%s') [fail@%d]\n[text=%s] %s\n",
			result.res, result.oa,
			result.msg, failidx, result.msg_utf8, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_cusd()
{
	struct result {
		int	res;
		int	type;
		char 	* cusd;
		int	dcs;
	};
	static const struct test_case {
		const char	* input;
		struct result 	result;
	} cases[] = {
		{ "+CUSD: 0,\"CF2135487D2E4130572D0682BB1A\",0", { 0, 0, "CF2135487D2E4130572D0682BB1A", 0} },
		{ "+CUSD: 1,\"CF2135487D2E4130572D0682BB1A\",1", { 0, 1, "CF2135487D2E4130572D0682BB1A", 1} },
		{ "+CUSD: 5", { 0, 5, "", -1} },
	};
	unsigned idx = 0;
	char * input;
	struct result result;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_cusd", input);
		result.res = at_parse_cusd(input, &result.type, &result.cusd, &result.dcs);
		if(result.res == cases[idx].result.res
			&& result.type == cases[idx].result.type
			&& result.dcs == cases[idx].result.dcs
			&& strcmp(result.cusd, cases[idx].result.cusd) == 0)
		{
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = %d (%d,\"%s\",%d)\t%s\n", result.res, result.type, result.cusd, result.dcs, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_cpin()
{
}

#/* */
void test_parse_csq()
{
}

#/* */
void test_parse_rssi()
{
}

#/* */
void test_parse_mode()
{
}

#/* */
void test_parse_csca()
{
}

#/* */
void test_parse_clcc()
{
	struct result {
		int		res;

		unsigned	index;
		unsigned	dir;
		unsigned	stat;
		unsigned	mode;
		unsigned	mpty;
		char		* number;
		unsigned	toa;
	};
	static const struct test_case {
		const char	* input;
		struct result	result;
	} cases[] = {
		{ "+CLCC: 1,1,4,0,0,\"\",145", { 0, 1, 1, 4, 0, 0, "", 145} },
		{ "+CLCC: 1,1,4,0,0,\"+79139131234\",145", { 0, 1, 1, 4, 0, 0, "+79139131234", 145} },
		{ "+CLCC: 1,1,4,0,0,\"+7913913ABCA\",145", { 0, 1, 1, 4, 0, 0, "+7913913ABCA", 145} },
		{ "+CLCC: 1,1,4,0,0,\"+7913913ABCA\"", { -1, 0, 0, 0, 0, 0, "", 0} },
	};
	unsigned idx = 0;
	char * input;
	struct result result;
	const char * msg;
	
	for(; idx < ITEMS_OF(cases); ++idx) {
		input = strdup(cases[idx].input);
		fprintf(stderr, "%s(\"%s\")...", "at_parse_clcc", input);
		result.res = at_parse_clcc(
			input, &result.index, &result.dir, &result.stat, &result.mode,
			&result.mpty, &result.number, &result.toa);
		if(result.res == cases[idx].result.res
			&& result.index == cases[idx].result.index
			&& result.dir == cases[idx].result.dir
			&& result.stat == cases[idx].result.stat
			&& result.mode == cases[idx].result.mode
			&& result.mpty == cases[idx].result.mpty
			&& strcmp(result.number, cases[idx].result.number) == 0
			&& result.toa == cases[idx].result.toa)
		{
			msg = "OK";
			ok++;
		} else {
			msg = "FAIL";
			faults++;
		}
		fprintf(stderr, " = %d (%d,%d,%d,%d,%d,\"%s\",%d)\t%s\n",
			result.res, result.index, result.dir, result.stat, result.mode,
			result.mpty, result.number, result.toa, msg);
		free(input);
	}
	fprintf(stderr, "\n");
}

#/* */
void test_parse_ccwa()
{
}

#/* */
int main()
{
	test_parse_cnum();
	test_parse_cops();
	test_parse_creg();
	test_parse_cmti();
	test_parse_cmt();
	test_parse_cmgr();
	test_parse_cusd();
	test_parse_cpin();
	test_parse_csq();
	test_parse_rssi();
	test_parse_mode();
	test_parse_csca();
	test_parse_clcc();
	test_parse_ccwa();
	test_gsm7();
	
	fprintf(stderr, "done %d tests: %d OK %d FAILS\n", ok + faults, ok, faults);

	if (faults) {
		return 1;
	}
	return 0;
}

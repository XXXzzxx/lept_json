#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "leptjson.h"

static int test_count = 0;
static int test_pass = 0;
static int main_ret = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
	do {\
		test_count++;\
		if(equality){\
			test_pass++;\
		}\
		else {\
			fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n",__FILE__, __LINE__, expect, actual);\
			main_ret++;\
		}\
	} while (0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect == actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect == actual), expect, actual, "%lf")

//测试不符合情况的重构
#define TEST_ERROR(error, json) \
	do {\
		lept_value v;\
		v.type = LEPT_NULL;\
		EXPECT_EQ_INT(error, lept_parse(&v, json));\
	}while(0)

//测试数字的宏
#define TEST_NUMBER(expect, json) \
	do {\
		lept_value v;\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect, lept_get_number(&v));\
	}while(0)

static void text_parse_number() {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000"); /* 位数达到了10000位 容不下 must underflow */ 

	TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}
static void text_parse_false() {
	lept_value v;
	v.type = LEPT_NULL;

	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "   flase"));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
}

static void text_parse_true() {
	lept_value v;
	v.type = LEPT_NULL;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "  true"));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}

static void text_parse_null() {
	lept_value v;
	v.type = LEPT_NULL;
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null    "));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

static void text_parse_expect_value() {
	TEST_ERROR(LEPT_PARSE_EXPEXT_VALUE, " ");
	TEST_ERROR(LEPT_PARSE_EXPEXT_VALUE, "");
}

static void text_parse_invalid_value() {
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "  nul  l");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "??");
	/* invalid number 在strtod()中可以解析  但是对于json是invalid */
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan");
}

static void text_parse_root_not_singular() {
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "23 2"); //不唯一是对的
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "  true 23 3"); //这种的话type测试会出现问题,  因为会先给true
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123");
	
}

static void text_parse_number_too_big() {
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void text_parse() {
	text_parse_false();
	text_parse_true();
	text_parse_null();
	text_parse_number();
	text_parse_expect_value();
	text_parse_invalid_value();
	text_parse_root_not_singular();
	text_parse_number_too_big();
}


int main()
{
	text_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}
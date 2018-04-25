#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;
static void test_parse_null();
static void test_parse();
static void test_parse_true();
static void test_parse_false();
static void test_parse_root_not_singular();
static void test_parse_expect_value();
static void test_parse_invalid_value();
static void test_parse_number();
static void test_parse_number_too_big();

//  !!attention: there must no whitespace between BASE and (
//  在define定义的\ 后不能添加//注释符 且 \ 后面不能有多余空格
// C 语言中的__FILE__用以指示本行语句所在源文件的文件名
// C 语言中的__LINE__用以指示本行语句在源文件中的位置信息
//  #line 200  指定下一行的__LINE__为 200
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
	do { \
		test_count++; \
		if (equality) \
			test_pass++; \
		else { \
			fprintf(stderr, "%s:%d: expect: "format " actual: " format "\n", __FILE__, __LINE__, expect, actual); \
			main_ret = 1; \
		} \
	} while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d") 

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%lf")

#define EXPECT_TEST_ERROR(error, json, expect_type) \
	do { \
		lept_value v; \
		v.type = LEPT_VOID; \
		EXPECT_EQ_INT(error, lept_parse(&v, json)); \
		EXPECT_EQ_INT(expect_type, lept_get_type(&v)); \
	}while(0)

#define EXPECT_TEST_NUMBER(expect, json) \
	do { \
		lept_value v; \
		v.type = LEPT_VOID; \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json)); \
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v)); \
        EXPECT_EQ_DOUBLE(expect, lept_get_number(&v)); \
	}while(0);

static void test_parse_null() {
	EXPECT_TEST_ERROR(LEPT_PARSE_OK, "null", LEPT_NULL);
}

static void test_parse_expect_value() {
	EXPECT_TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ", LEPT_VOID);
}

static void test_parse_invalid_value() {
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?", LEPT_VOID);
	/* invalid number */
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123", LEPT_VOID); /* at least one digit before '.' */
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.", LEPT_VOID);   /* at least one digit after '.' */
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan", LEPT_VOID);
}

static void test_parse_root_not_singular() {
	EXPECT_TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0123", LEPT_VOID); /* after zero should be '.' or nothing */
	EXPECT_TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x0", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "0x123", LEPT_VOID);
}

static void test_parse_true() {
	EXPECT_TEST_ERROR(LEPT_PARSE_OK, "true", LEPT_TRUE);
}

static void test_parse_false() {
	EXPECT_TEST_ERROR(LEPT_PARSE_OK, "false", LEPT_FALSE);
}

static void test_parse_number() {
	EXPECT_TEST_NUMBER(0.0, "0");
	EXPECT_TEST_NUMBER(0.0, "-0");
	EXPECT_TEST_NUMBER(0.0, "-0.0");
	EXPECT_TEST_NUMBER(1.0, "1");
	EXPECT_TEST_NUMBER(-1.0, "-1");
	EXPECT_TEST_NUMBER(1.5, "1.5");
	EXPECT_TEST_NUMBER(-1.5, "-1.5");
	EXPECT_TEST_NUMBER(3.1416, "3.1416");
	EXPECT_TEST_NUMBER(1E10, "1E10");
	EXPECT_TEST_NUMBER(1e10, "1e10");
	EXPECT_TEST_NUMBER(1E+10, "1E+10");
	EXPECT_TEST_NUMBER(1E-10, "1E-10");
	EXPECT_TEST_NUMBER(-1E10, "-1E10");
	EXPECT_TEST_NUMBER(-1e10, "-1e10");
	EXPECT_TEST_NUMBER(-1E+10, "-1E+10");
	EXPECT_TEST_NUMBER(-1E-10, "-1E-10");
	EXPECT_TEST_NUMBER(1.234E+10, "1.234E+10");
	EXPECT_TEST_NUMBER(1.234E-10, "1.234E-10");
	EXPECT_TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
										 /* the smallest number > 1 */
	EXPECT_TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
	/* minimum denormal */
	EXPECT_TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324");
	EXPECT_TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	/* Max subnormal double */
	EXPECT_TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");
	EXPECT_TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	/* Min normal positive double */
	EXPECT_TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");
	EXPECT_TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	/* Max double */
	EXPECT_TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");
	EXPECT_TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_number_too_big() {
	EXPECT_TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309", LEPT_VOID);
}

static void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number();
	test_parse_number_too_big();
}

int main() {
	test_parse();
	printf("%d/%d (%3.2f%%) passed!\n", test_pass, test_count, test_pass * 100.0 / test_count);
	getchar();
	//getchar();
	return main_ret;
}
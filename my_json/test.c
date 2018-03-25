#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)
// 重看  且\后面不能有多余空格  
// C 语言中的__FILE__用以指示本行语句所在源文件的文件名
// C 语言中的__LINE__用以指示本行语句在源文件中的位置信息
//  #line 200  指定下一行的__LINE__为 200
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%f")

#define EXPECT_EQ_STRING(expect, actual) EXPECT_EQ_BASE(!strcmp((expect), (actual)), expect, actual, "%s")

#define EXPECT_EQ_LOGIC(expect, actual) EXPECT_EQ_BASE((expect)==(actual), expect, actual, "%d")

#define TEST_ERROR(error, json, type)\
    do{\
        lept_value v;\
        EXPECT_EQ_INT(error, lept_parse(&v, json));\
		EXPECT_EQ_INT(type, lept_get_type(&v));\
    }while(0)

#define TEST_NUMBER(expect, json)\
    do{\
        lept_value v;\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
		EXPECT_EQ_DOUBLE(expect, lept_get_number(&v));\
    }while(0)

#define TEST_STRING(expect, json)\
    do {\
        lept_value v;\
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
		EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));\
		EXPECT_EQ_STRING(expect, lept_get_string(&v));\
	}while(0)

#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)

static void test_parse();
static void test_parse_null();
static void test_parse_true();
static void test_parse_false();
static void test_parse_expect_value();
static void test_parse_invalid_value();
static void test_parse_root_not_singular();
static void test_parse_number();
static void test_parse_string();
static void test_access_boolean();
static void test_access_number();
static void test_access_string();

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	getchar();
    return main_ret;
}

static void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number();
	test_parse_string();
}

static void test_parse_null() {
	TEST_ERROR(LEPT_PARSE_OK, "null", LEPT_NULL);
}

static void test_parse_true() {
	TEST_ERROR(LEPT_PARSE_OK, "  true   ", LEPT_TRUE);
}

static void test_parse_false() {
	TEST_ERROR(LEPT_PARSE_OK, "  false   ", LEPT_FALSE);
}

static void test_parse_expect_value() {
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "", LEPT_NULL);
	TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ", LEPT_NULL);
}

static void test_parse_invalid_value() {
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "ull", LEPT_NULL);
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?", LEPT_NULL);

	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+0", LEPT_NULL);
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "+1", LEPT_NULL);
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, ".123", LEPT_NULL); /* at least one digit before '.' */
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "1.", LEPT_NULL);   /* at least one digit after '.' */
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "INF", LEPT_NULL);
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "inf", LEPT_NULL);
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "NAN", LEPT_NULL);
	TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nan", LEPT_NULL);

	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "1e309", LEPT_NULL);
	TEST_ERROR(LEPT_PARSE_NUMBER_TOO_BIG, "-1e309", LEPT_NULL);
}

static void test_parse_root_not_singular() {
	TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x", LEPT_NULL);
}

static void test_parse_number() {
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
	TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

								  /* the smallest number > 1 */
	TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
	/* minimum denormal */
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324");
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	/* Max subnormal double */
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	/* Min normal positive double */
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	/* Max double */
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_string() {
	TEST_STRING("", "\"\"");
	TEST_STRING("Hello", "\"Hello\"");
	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

static void test_access_boolean() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "a", 1);
	lept_set_boolean(&v, 1);
	EXPECT_EQ_LOGIC(lept_get_boolean(&v), 1);
	lept_set_boolean(&v, 0);
	EXPECT_EQ_LOGIC(lept_get_boolean(&v), 0);
	lept_free(&v);
}

static void test_access_number() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "a", 1);
	lept_set_number(&v, 1234.5);
	EXPECT_EQ_DOUBLE(1234.5, lept_get_number(&v));
	lept_free(&v);
}

static void test_access_string() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "", 0);
	EXPECT_EQ_STRING("", lept_get_string(&v), lept_get_string_length(&v));
	lept_set_string(&v, "Hello", 5);
	EXPECT_EQ_STRING("Hello", lept_get_string(&v), lept_get_string_length(&v));
	lept_free(&v);
}
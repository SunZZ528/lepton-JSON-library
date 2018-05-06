#define _WINDOWS
#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

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
static void test_access_string();
static void test_parse_missing_quotation_mark();
static void test_parse_invalid_string_escape();
static void test_parse_invalid_string_char();
static void test_access_null();
static void test_access_boolean();
static void test_access_number();
static void test_parse_string();
static void test_access_void();
static void test_parse_array();
static void test_parse_miss_key();
static void test_parse_miss_colon();
static void test_parse_miss_comma_or_curly_bracket();
static void test_parse_object();
static void test_stringify();
static void test_stringify_number();
static void test_stringify_string();
static void test_stringify_array();
static void test_stringify_object();
static void test_parse_miss_quotation_mark();
static void test_parse_invalid_string_escape();
static void test_parse_invalid_string_char();
static void test_parse_invalid_unicode_hex();
static void test_parse_invalid_unicode_hex();
static void test_parse_invalid_unicode_surrogate();
static void test_parse_miss_comma_or_square_bracket();

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

#define	EXPECT_EQ_STRING(expect, actual, len) EXPECT_EQ_BASE(sizeof(expect) - 1 == len && strncmp((expect), (actual), (len) )== 0, expect, actual, "%s")

#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")  

#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

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

#define EXPECT_TEST_STRING(expect, json) \
	do { \
		lept_value v; \
		lept_init(&v); \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));\
		EXPECT_EQ_STRING(expect, lept_get_string(&v), lept_get_len(&v));\
        lept_free(&v);\
}while(0)

#define TEST_ROUNDTRIP(json) \
	do { \
		lept_value v; \
		char *json2; \
		size_t length; \
		lept_init(&v); \
		EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json)); \
		EXPECT_EQ_INT(LEPT_STRINGIFY_OK, lept_stringify(&v, &json2, &length)); \
		EXPECT_EQ_STRING(json, json2, length); \
		lept_free(&v); \
		free(json2); \
}while(0);

#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

static void test_parse_array() {
	size_t i, j;
	lept_value v;
	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ ]"));
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v)); 
	EXPECT_EQ_INT(0, lept_get_array_size(&v));
	lept_free(&v);

	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[null , false , true , 123 , \"abc\"]"));
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
	EXPECT_EQ_INT(5, lept_get_array_size(&v));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(lept_get_array_element(&v, 0)));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(lept_get_array_element(&v, 1)));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(lept_get_array_element(&v, 2)));
	EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_array_element(&v, 3)));
	EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_array_element(&v, 4)));
	EXPECT_EQ_DOUBLE(123.0, lept_get_number(lept_get_array_element(&v, 3)));
	EXPECT_EQ_STRING("abc", lept_get_string(lept_get_array_element(&v, 4)), lept_get_len(lept_get_array_element(&v, 4)));
	lept_free(&v);

	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(&v));
	EXPECT_EQ_SIZE_T(4, lept_get_array_size(&v));
	for (i = 0; i < 4; i++) {
		lept_value* a = lept_get_array_element(&v, i);
		EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(a));
		EXPECT_EQ_SIZE_T(i, lept_get_array_size(a));
		for (j = 0; j < i; j++) {
			lept_value* e = lept_get_array_element(a, j);
			EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
			EXPECT_EQ_DOUBLE((double)j, lept_get_number(e));
		}
	}
	lept_free(&v);
}

static void test_parse_null() {
	EXPECT_TEST_ERROR(LEPT_PARSE_OK, "null", LEPT_NULL);
	lept_value v;
	lept_init(&v);
	lept_set_boolean(&v, 0);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
	lept_free(&v);
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
	lept_value v;
	lept_init(&v);
	lept_set_boolean(&v, 0);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
	lept_free(&v);
}

static void test_parse_false() {
	EXPECT_TEST_ERROR(LEPT_PARSE_OK, "false", LEPT_FALSE);
	lept_value v;
	lept_init(&v);
	lept_set_boolean(&v, 1);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
	lept_free(&v);
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
	test_access_void();
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
	test_parse_string();
	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();
	test_parse_missing_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();
	test_access_null();
	test_access_boolean();
	test_access_number();
	test_access_string();
	test_parse_array();
	test_parse_miss_key();
	test_parse_miss_colon();
	test_parse_miss_comma_or_curly_bracket(); 
	test_parse_object();
	test_stringify();
	test_parse_miss_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();
	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_surrogate();
	test_parse_miss_comma_or_square_bracket();
}

static void test_access_null() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "a", 1);
	lept_set_null(&v);
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
	lept_free(&v);
}

static void test_access_void() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "a", 1);
	lept_set_void(&v);
	EXPECT_EQ_INT(LEPT_VOID, lept_get_type(&v));
	lept_free(&v);
}

static void test_access_boolean() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "a", 1);
	lept_set_boolean(&v, 1);
	EXPECT_TRUE(lept_get_boolean(&v));
	lept_set_boolean(&v, 0);
	EXPECT_FALSE(lept_get_boolean(&v));
	lept_free(&v);
}

static void test_access_number() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "a", 1);
	lept_set_double(&v, 1234.5);
	EXPECT_EQ_DOUBLE(1234.5, lept_get_number(&v));
	lept_free(&v);
}

static void test_access_string() {
	lept_value v;
	lept_init(&v);
	lept_set_string(&v, "", 0);
	EXPECT_EQ_STRING("", lept_get_string(&v), lept_get_len(&v));
	lept_set_string(&v, "Hello", 5);
	EXPECT_EQ_STRING("Hello", lept_get_string(&v), lept_get_len(&v));
	lept_free(&v);
}

static void test_parse_string() {
	EXPECT_TEST_STRING("", "\"\"");
	EXPECT_TEST_STRING("Hello", "\"Hello\"");
	EXPECT_TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	EXPECT_TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
	EXPECT_TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
	EXPECT_TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
	EXPECT_TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
	EXPECT_TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
	EXPECT_TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
	EXPECT_TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_parse_missing_quotation_mark() {
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc", LEPT_VOID);
}

static void test_parse_invalid_string_escape() {
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"", LEPT_VOID);
}

static void test_parse_invalid_string_char() {
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"", LEPT_VOID);
}

static void test_parse_miss_key() {
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_KEY, "{:1,", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_KEY, "{1:1,", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_KEY, "{true:1,", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_KEY, "{false:1,", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_KEY, "{null:1,", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_KEY, "{[]:1,", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_KEY, "{{}:1,", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_KEY, "{\"a\":1,", LEPT_VOID);
}

static void test_parse_miss_colon() {
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\"}", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\",\"b\"}", LEPT_VOID);
}

static void test_parse_miss_comma_or_curly_bracket() {
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"atest_parse_object\":{}", LEPT_VOID);
}

static void test_parse_object() {
	lept_value v;
	size_t i;
	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, " { } "));
	EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(&v));
	EXPECT_EQ_SIZE_T(0, lept_get_object_size(&v));
	lept_free(&v);
	
	lept_init(&v);
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v,
		" { "
		"\"n\" : null , "
		"\"f\" : false , "
		"\"t\" : true , "
		"\"i\" : 123 , "
		"\"s\" : \"abc\", "
		"\"a\" : [ 1, 2, 3 ],"
		"\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
		" } "
	));
	EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(&v));
	EXPECT_EQ_SIZE_T(7, lept_get_object_size(&v));
	EXPECT_EQ_STRING("n", lept_get_object_key(&v, 0), lept_get_object_key_len(&v, 0));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(lept_get_object_value(&v, 0)));
	EXPECT_EQ_STRING("f", lept_get_object_key(&v, 1), lept_get_object_key_len(&v, 1));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(lept_get_object_value(&v, 1)));
	EXPECT_EQ_STRING("t", lept_get_object_key(&v, 2), lept_get_object_key_len(&v, 2));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(lept_get_object_value(&v, 2)));
	EXPECT_EQ_STRING("i", lept_get_object_key(&v, 3), lept_get_object_key_len(&v, 3));
	EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(lept_get_object_value(&v, 3)));
	EXPECT_EQ_DOUBLE(123.0, lept_get_number(lept_get_object_value(&v, 3)));
	EXPECT_EQ_STRING("s", lept_get_object_key(&v, 4), lept_get_object_key_len(&v, 4));
	EXPECT_EQ_INT(LEPT_STRING, lept_get_type(lept_get_object_value(&v, 4)));
	EXPECT_EQ_STRING("abc", lept_get_string(lept_get_object_value(&v, 4)), lept_get_len(lept_get_object_value(&v, 4)));
	EXPECT_EQ_STRING("a", lept_get_object_key(&v, 5), lept_get_object_key_len(&v, 5));
	EXPECT_EQ_INT(LEPT_ARRAY, lept_get_type(lept_get_object_value(&v, 5)));
	EXPECT_EQ_SIZE_T(3, lept_get_array_size(lept_get_object_value(&v, 5)));
	for (i = 0; i < 3; i++) {
		lept_value* e = lept_get_array_element(lept_get_object_value(&v, 5), i);
		EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(e));
		EXPECT_EQ_DOUBLE(i + 1.0, lept_get_number(e));
	}
	EXPECT_EQ_STRING("o", lept_get_object_key(&v, 6), lept_get_object_key_len(&v, 6));
	{
		lept_value* o = lept_get_object_value(&v, 6);
		EXPECT_EQ_INT(LEPT_OBJECT, lept_get_type(o));
		for (i = 0; i < 3; i++) {
			lept_value* ov = lept_get_object_value(o, i);
			EXPECT_TRUE('1' + i == lept_get_object_key(o, i)[0]);
			EXPECT_EQ_SIZE_T(1, lept_get_object_key_len(o, i));
			EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(ov));
			EXPECT_EQ_DOUBLE(i + 1.0, lept_get_number(ov));
		}
	}
	lept_free(&v);
}

static void test_stringify_number() {
	TEST_ROUNDTRIP("0");
	TEST_ROUNDTRIP("-0");
	TEST_ROUNDTRIP("1");
	TEST_ROUNDTRIP("-1");
	TEST_ROUNDTRIP("1.5");
	TEST_ROUNDTRIP("-1.5");
	TEST_ROUNDTRIP("3.25");
	TEST_ROUNDTRIP("1e+20");
	TEST_ROUNDTRIP("1.234e+20");
	TEST_ROUNDTRIP("1.234e-20");

	TEST_ROUNDTRIP("1.0000000000000002"); /* the smallest number > 1 */
	TEST_ROUNDTRIP("4.9406564584124654e-324"); /* minimum denormal */
	TEST_ROUNDTRIP("-4.9406564584124654e-324");
	TEST_ROUNDTRIP("2.2250738585072009e-308");  /* Max subnormal double */
	TEST_ROUNDTRIP("-2.2250738585072009e-308");
	TEST_ROUNDTRIP("2.2250738585072014e-308");  /* Min normal positive double */
	TEST_ROUNDTRIP("-2.2250738585072014e-308");
	TEST_ROUNDTRIP("1.7976931348623157e+308");  /* Max double */
	TEST_ROUNDTRIP("-1.7976931348623157e+308");
}

static void test_stringify_string() {
/*	lept_value v; 
	char *json = "\"\\\" \\\\ / \\b \\f \\n \\r \\t\"";
	char *json2; 
	size_t length; 
	lept_init(&v); 
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, json)); 
	EXPECT_EQ_INT(LEPT_STRINGIFY_OK, lept_stringify(&v, &json2, &length)); 
	EXPECT_EQ_STRING(json, json2, length); 
	lept_free(&v); 
	free(json2); */
	TEST_ROUNDTRIP("\"\"");
	TEST_ROUNDTRIP("\"Hello\"");
	TEST_ROUNDTRIP("\"Hello\\nWorld\"");
	TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
	TEST_ROUNDTRIP("\"Hello\\u0000World\"");
}

static void test_stringify_array() {
	TEST_ROUNDTRIP("[]");
	TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object() {
	TEST_ROUNDTRIP("{}");
	TEST_ROUNDTRIP("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

static void test_stringify() {
	TEST_ROUNDTRIP("null");
	TEST_ROUNDTRIP("false");
	TEST_ROUNDTRIP("true");
	test_stringify_number();
	test_stringify_string();
	test_stringify_array();
	test_stringify_object();
}

static void test_parse_miss_quotation_mark() {
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc", LEPT_VOID);
}

static void test_parse_invalid_unicode_hex() {
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"", LEPT_VOID);
}

static void test_parse_invalid_unicode_surrogate() {
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"", LEPT_VOID);
}

static void test_parse_miss_comma_or_square_bracket() {
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2", LEPT_VOID);
	EXPECT_TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]", LEPT_VOID);
}

int main() {
#ifdef _WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	test_parse();
	printf("%d/%d (%3.2f%%) passed!\n", test_pass, test_count, test_pass * 100.0 / test_count);
	getchar();
	return main_ret;
}
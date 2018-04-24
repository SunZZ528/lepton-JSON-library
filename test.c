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

static void test_parse_null() {
	lept_value v;
	v.type = LEPT_TRUE; // set up a value that different with LETP_NULL
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
	EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

static void test_parse_true() {
	lept_value v;
	v.type = LEPT_NULL; // set up a value that different with LETP_NULL
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
	EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}

static void test_parse_false() {
	lept_value v;
	v.type = LEPT_NULL; // set up a value that different with LETP_NULL
	EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
	EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
}

static void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
}

int main() {
	test_parse();
	printf("%d/%d (%3.2f%%) passed!\n", test_pass, test_count, test_pass * 100.0 / test_count);
	getchar();
	//getchar();
	return main_ret;
}
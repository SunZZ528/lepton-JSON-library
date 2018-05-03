﻿#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include <stddef.h> // size_t

typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT, LEPT_VOID}lept_type;

enum {
	LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG,
	LEPT_PARSE_MISS_QUOTATION_MARK,
	LEPT_PARSE_INVALID_STRING_ESCAPE,
	LEPT_PARSE_INVALID_STRING_CHAR,
	LEPT_PARSE_INVALID_UNICODE_HEX,
	LEPT_PARSE_INVALID_UNICODE_SURROGATE,
	LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
};

typedef struct lept_value lept_value;
// 由于 lept_value 内使用了自身类型的指针，我们必须前向声明（forward declare）此类型
struct lept_value {
	union {
		struct {
			lept_value *e;
			size_t size; // size 是元素的个数，不是字节单位
		}arr; 
		struct {
			char *s;
			size_t len;
		}s;
		double n;
	}u; //  C11 新增了匿名 struct/union 语法,可以省略u
	lept_type type;
};

#define lept_set_void(v) lept_free(v)

#define lept_init(v) \
	do { \
		(v)->type = LEPT_VOID; \
} while(0)

int lept_get_type(const lept_value *);
int lept_parse(lept_value *, char *);
double lept_get_number(const lept_value *);
void lept_set_string(lept_value *v, const char *, size_t);
void lept_free(lept_value *);
void lept_set_boolean(lept_value *, int);
int lept_get_boolean(const lept_value *);
void lept_set_double(lept_value *, double);
double lept_get_double(const lept_value *);
const char* lept_get_string(const lept_value *);
size_t lept_get_len(const lept_value *);
void lept_set_null(lept_value *);
size_t lept_get_array_size(const lept_value *);
lept_value *lept_get_array_element(const lept_value *, size_t);

#endif
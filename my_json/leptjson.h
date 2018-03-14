#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include <stddef.h>  /*size_t*/

typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

/*typedef struct {
	char* s;
	int len;
	double n;
	lept_type type;
}lept_value;*/
// 我们要把之前的 v->n 改成 v->u.n。而要访问字符串的数据，则要使用 v->u.s.s 和 v->u.s.len。这种写法比较麻烦吧，
// 其实 C11 新增了匿名 struct/union 语法，就可以采用 v->n、v->s、v->len 来作访问。

typedef struct {
	union {
		struct { char *s; size_t len; }s;  /* string */
		double n;                       /* number */
	}u;  // 也可匿名
	lept_type type;
}lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG,
};

int lept_parse(const lept_value *v, const char* json);

lept_type lept_get_type(const lept_value *v);

double lept_get_number(const lept_value *v);

#endif /* LEPTJSON_H__ */

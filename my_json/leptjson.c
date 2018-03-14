#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context *);
static int lept_parse_value(lept_context *, lept_value *);
static int lept_parse_test(lept_context *, lept_value *, char *, int);
static int lept_parse_string(lept_context *, lept_value *);
static void lept_set_string(lept_value *, const char *, const size_t);
static void lept_free(lept_value *);
static int lept_parse_number(lept_context *, lept_value *);

lept_type lept_get_type(const lept_value* v) {
	assert(v != NULL);
	return v->type;
}

double lept_get_number(const lept_value *v) {
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->u.n;
}

char * lept_get_string(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.s;
}

int lept_parse(lept_value *v, const char* json) {
	lept_context c;
	int return_v;
	assert(v != NULL);  //assert 的作用是现计算表达式 expression ，如果其值为假（即为 0），那么它先向 stderr 打印一条出错信息，然后通过调用 abort 来终止程序运行
	c.json = json;      // &c 似乎不等于 &c.json
	v->type = LEPT_NULL;
	lept_parse_whitespace(&c);
	return_v = lept_parse_value(&c, v);
	lept_parse_whitespace(&c);
	if (return_v == LEPT_PARSE_OK && *c.json != '\0')
	{
		v->type = LEPT_NULL;
		return LEPT_PARSE_ROOT_NOT_SINGULAR;
	}
	else return return_v;
}

static void lept_parse_whitespace(lept_context *c) {
	// 使用一个指针 p 来表示当前的解析字符位置。这样做有两个好处，一是代码更简单，
	// 二是在某些编译器下性能更好（因为不能确定 c 会否被改变，从而每次更改 c->json 都要做一次间接访问）
	const char *p = c->json;
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		p++;
	c->json = p;
}

static int lept_parse_value(lept_context *c, lept_value *v) {
	switch (*c->json) {
	case 'n':  return lept_parse_test(c, v, "null", LEPT_NULL);
	case 't':  return lept_parse_test(c, v, "true", LEPT_TRUE);
	case 'f':  return lept_parse_test(c, v, "false", LEPT_FALSE);
	case '\"':  return lept_parse_string(c, v);
	case '\0': return LEPT_PARSE_EXPECT_VALUE;
	default:   return lept_parse_number(c, v);  // '0-9' or '-'
	}
}

static int lept_parse_test(lept_context *c, lept_value *v, char *str, int t) {
	// 注意在 C 语言中，数组长度、索引值最好使用 size_t 类型，而不是 int 或 unsigned。
	EXPECT(c, *str);
	str++;
	while (*str != '\0') {
		if (*c->json != *str)
			return LEPT_PARSE_INVALID_VALUE;
		str++;
		c->json++;
	}
	v->type = t;
	return LEPT_PARSE_OK;
}

static int lept_parse_string(lept_context *c, lept_value *v) {
	const char *p = c->json;
	v->u.s.len = 0;
	if (!(*p == '\"')) {
		v->type = LEPT_NULL;
		return LEPT_PARSE_INVALID_VALUE;
	}
	p++;
	while (*p != '\"' && *p != '\0') {
		p++;
		v->u.s.len++;
	}
	if (*p == '\0') {
		v->type = LEPT_NULL;
		return LEPT_PARSE_INVALID_VALUE;
	}
	p++;
	if (*p != '\0') {
		v->type = LEPT_NULL;
		return LEPT_PARSE_INVALID_VALUE;
	}
	else {
		v->type = LEPT_STRING;
		lept_set_string(v, c->json + 1, v->u.s.len);
		c->json += (v->u.s.len + 2);
		return LEPT_PARSE_OK;
	}
}

static void lept_set_string(lept_value *v, const char *s, const size_t len) {
	assert(v != NULL && (s != NULL || len == 0));
	//lept_free(v);
	v->u.s.s = (char *)malloc(len + 1);
	memcpy(v->u.s.s, s, len);
	v->u.s.s[len] = '\0';
	v->u.s.len = len;
}

static void lept_free(lept_value *v) {
	assert(v != NULL);
	if (v->type == LEPT_STRING) {
		free(v->u.s.s);
		v->u.s.len = 0;
	}
	v->type = LEPT_NULL;
}

static int lept_parse_number(lept_context *c, lept_value *v) {
	char *end;  // ???
	const char *p = c->json;
	// valid
	if (*p == '-') p++;
	if (*p == '0' && (!ISDIGIT(*(p + 1)))) p++;
	else if (!ISDIGIT(*p)) {
		v->type = LEPT_NULL;
		return LEPT_PARSE_INVALID_VALUE;
	}
	while (ISDIGIT(*p)) p++;  // for (p++; ISDIGIT(*p); p++);
	if (*p == '.') {
		p++;
		if (!ISDIGIT(*p)) {
			v->type = LEPT_NULL;
			return LEPT_PARSE_INVALID_VALUE;
		}
		while (ISDIGIT(*p)) p++;
	}
	if (*p == 'E' || *p == 'e') {
		p++;
		if (*p == '+' || *p == '-') p++;
		while (ISDIGIT(*p)) p++;
	}
	if (*p != '\0') {
		v->type = LEPT_NULL;
		return LEPT_PARSE_INVALID_VALUE;
	}
	errno = 0;
	v->u.n = strtod(c->json, &end);
	if (c->json == end)  // 在校验成功以后，我们不再使用 end 指针去检测 strtod() 的正确性，第二个参数可传入 NULL
		return LEPT_PARSE_INVALID_VALUE;
	if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL)) {
	    v->type = LEPT_NULL;
		return LEPT_PARSE_NUMBER_TOO_BIG;
    }
	c->json = end;
	v->type = LEPT_NUMBER;
	return LEPT_PARSE_OK;
}
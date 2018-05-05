#define _WINDOWS
#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "leptjson.h"
#include <assert.h>  /* assert */
#include <stdlib.h>  /* NULL strtod() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <string.h>  /* memcpy*/

typedef struct {
	const char *json;
	void *stack;
	size_t size, top;
}lept_context;

static int lept_parse_value(lept_context *, lept_value *);
static void lept_parse_whitespace(lept_context *);
static int lept_parse_number(lept_context *, lept_value *);
static int lept_parse_literal(lept_context *, lept_value *, const char *, const lept_type);
static void *lept_context_push(lept_context *, size_t);
static void *lept_context_pop(lept_context *, size_t);
static int lept_parse_string(lept_context *, lept_value *);
static char *lept_parse_hex4(char *, unsigned *);
static int lept_encode_utf8(lept_context *, const unsigned);
static int lept_parse_array(lept_context *, lept_value *);
static int lept_parse_string_raw(lept_context *, char **, size_t *);
static int lept_parse_object(lept_context *, lept_value *);

#define EXPECT(c, ch) \
	do { \
		assert(*c->json == (ch)); \
		c->json++; \
	} while(0)

#define ISDIGIT0_9(ch) ( (ch) >= '0' && (ch) <= '9')

#define ISDIGIT1_9(ch) ( (ch) >= '1' && (ch) <= '9')

#define ISHEX(ch) (((ch) >= '0' && (ch) <= '9') || ((ch) >= 'a' && (ch) <= 'f') || ((ch) >= 'A' && (ch) <= 'F'))

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

#define PUTC(c, ch) \
	do {  \
		* (char *)lept_context_push(c, sizeof(char)) = (ch); \
} while(0)
// 这段理解了很久,假设lept_context_push返回一个char指针p, 再另*p = ch
/*void PUTC(lept_context *c, char ch) {
//	*(char *)lept_context_push(c, sizeof(char)) = (ch);
	char *ret;
	ret = lept_context_push(c, sizeof(char));
	*ret = ch;
}*/

#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

int lept_get_type(const lept_value *v) {
	assert(v != NULL);
	return v->type;
}

int lept_parse(lept_value *v, char *json) {
	lept_context c;
	int ret = 0;
	assert(v != NULL);
	c.json = json;
	c.stack = NULL;
	c.size = c.top = 0;
	lept_init(v);
	v->type = LEPT_VOID;
	lept_parse_whitespace(&c);
	if((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
		lept_parse_whitespace(&c);
		if (*c.json != '\0') {
			ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
			v->type = LEPT_VOID;
		}
	}
	if(c.top != 0);
		free(c.stack);
	return ret;
}

static int lept_parse_value(lept_context *c, lept_value *v) {
	switch (*c->json) {
		case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
		case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
		case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
		case '\0': return LEPT_PARSE_EXPECT_VALUE;
		case '\"': return lept_parse_string(c, v);
		case '[': return lept_parse_array(c, v);
		case '{': return lept_parse_object(c, v);
		default: return lept_parse_number(c, v);
	}
}

static void lept_parse_whitespace(lept_context *c) {
	const char *p = c->json;
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		p++;
	c->json = p;
}

double lept_get_number(const lept_value *v) {
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->u.n;
}

static int lept_parse_number(lept_context *c, lept_value *v) {
	const char *p = c->json;
	char *end;
	if (*p == '-')
		p++;
	if (*p == '0')
		p++;
	else {
		if (!ISDIGIT1_9(*p))
			return LEPT_PARSE_INVALID_VALUE;
		while (ISDIGIT0_9(*p))
			p++;
	}
	if (*p == '.') {
		p++;
		if(!ISDIGIT0_9(*(p))) 
			return LEPT_PARSE_INVALID_VALUE;
		while (ISDIGIT0_9(*p))
			p++;
	}
	if (*p == 'E' || *p == 'e') {
		p++;
		if (*p == '+' || *p == '-')
			p++;
		if (!ISDIGIT0_9(*(p)))
			return LEPT_PARSE_INVALID_VALUE;
		while (ISDIGIT0_9(*p))
			p++;
	}
	errno = 0;
	v->u.n = strtod(c->json, &end);
	if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL))
		return LEPT_PARSE_NUMBER_TOO_BIG;
	if (c->json == end)
		return LEPT_PARSE_INVALID_VALUE;
	c->json = p;
	v->type = LEPT_NUMBER;
	return LEPT_PARSE_OK;
}

static int lept_parse_literal(lept_context *c, lept_value *v, const char *json, const lept_type type) {
	size_t i = 0;
	while (*(json + i) == *(c->json + i) && *(json + i) != '\0')
			i++;
	if (*(json + i) != '\0')
		return LEPT_PARSE_INVALID_VALUE;
	else {
		c->json += i;
		v->type = type;
	}
	return LEPT_PARSE_OK;
}

void lept_set_string(lept_value *v, const char *s, size_t len) {
	assert(v != NULL && (s != NULL || len == 0));
	lept_free(v);
	v->u.s.s = (char *)malloc(len + 1);
	memcpy(v->u.s.s, s, len);
	v->u.s.s[len] = '\0';
	v->u.s.len = len;
	v->type = LEPT_STRING;
}

void lept_free(lept_value *v) {
	assert(v != NULL);
	size_t i;
	switch (v->type) {
		case LEPT_STRING:
			free(v->u.s.s);
			break;
		case LEPT_ARRAY:
			for (i = 0; i < v->u.arr.size; i++)
				lept_free(&v->u.arr.e[i]);
			free(v->u.arr.e);
			break;
		case LEPT_OBJECT:
			for (i = 0; i < v->u.o.size; i++) {
				free(v->u.o.m[i].k);
				lept_free(&v->u.o.m[i].v);
			}
			free(v->u.o.m);
			break;
		default: break;
	}
	v->type = LEPT_VOID;
}

void lept_set_boolean(lept_value *v, int num) {
	assert(v != NULL);
	lept_free(v);
	v->type = num ? LEPT_TRUE : LEPT_FALSE;
}

int lept_get_boolean(const lept_value *v) {
	assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
	if (v->type == LEPT_TRUE)
		return 1;
	else return 0;
}

void lept_set_double(lept_value *v, double num) {
	assert(v != NULL );
	lept_free(v);
	v->type = LEPT_NUMBER;
	v->u.n = num;
}

double lept_get_double(const lept_value *v) {
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->u.n;
}

const char *lept_get_string(const lept_value *v) {
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.s;
}

size_t lept_get_len(const lept_value *v) {
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.s.len;
}

static void *lept_context_push(lept_context *c, size_t size) {
	void *ret;
	assert(size > 0);
	if (c->top + size >= c->size) {
		if (c->size == 0)
			c->size = LEPT_PARSE_STACK_INIT_SIZE;
		while (c->top + size >= c->size) {
			c->size += c->size >> 1;  // 扩大1.5倍
			//如果增长因子是2，则必然不能用到之前分配的地址空间（因为2^n>2^0+2^1+...+2^(n-1)）,在缓存上不友好，故建议增长因子最好少于2，例如1.5
		}
		c->stack = (char *)realloc(c->stack, c->size); 
		// realloc(NULL, size) 的行为是等价于 malloc(size) ,所以我们不需要为第一次分配内存作特别处理。
	}
	ret = (char *)c->stack + c->top;
	c->top += size;
	return ret;
}

static void *lept_context_pop(lept_context *c, size_t size) {
	assert(c->top >= size);
	return (char *)c->stack + (c->top -= size);
}

static int lept_parse_string(lept_context *c, lept_value *v) {
	int ret;
	size_t len;
	char *s;
	if ((ret = lept_parse_string_raw(c, &s, &len)) == LEPT_PARSE_OK)
		lept_set_string(v, s, len);
	return ret;
}
/* 解析 JSON 字符串，把结果写入 str 和 len */
/* str 指向 c->stack 中的元素，需要在 c->stack  */
static int lept_parse_string_raw(lept_context *c, char **str, size_t *size) {
	size_t head = c->top;
	char *p;
	unsigned u, u_low;
	EXPECT(c, '\"');
	p = c->json;
	while (1) {
		char ch = *p++;
		switch (ch) {
		case '\"':
			*size = c->top - head;
			//lept_set_string(v, (const char *)lept_context_pop(c, len), len);
			*str = (char *)lept_context_pop(c, *size);
			c->json = p;
			return LEPT_PARSE_OK;
		case '\0':
			STRING_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK);
		case '\\':
			switch (*p++) {
			case '\"': PUTC(c, '\"'); break;
			case '\\': PUTC(c, '\\'); break;
			case '/':  PUTC(c, '/'); break;
			case 'b':  PUTC(c, '\b'); break;
			case 'f':  PUTC(c, '\f'); break;
			case 'n':  PUTC(c, '\n'); break;
			case 'r':  PUTC(c, '\r'); break;
			case 't':  PUTC(c, '\t'); break;
			case 'u':
				if (!(p = lept_parse_hex4(p, &u)))
					STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
				if (u >= 0xD800 && u <= 0xD8FF) {
					if (*p++ != '\\')
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					if (*p++ != 'u')
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					if (!(p = lept_parse_hex4(p, &u_low)))
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
					if (u_low < 0xDC00 || u_low > 0xDFFF)
						STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
					u = 0x10000 + (u - 0xD800) * 0x400 + (u_low - 0xDC00);
				}
				lept_encode_utf8(c, u);
				break;
			default:
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE);
			}
			break;
		default:
			if ((unsigned char)ch < 0x20) {
				STRING_ERROR(LEPT_PARSE_INVALID_STRING_CHAR);
			}
			PUTC(c, ch);
		}
	}
}

int lept_encode_utf8(lept_context *c, const unsigned u) {
	if (u >= 0x0000 && u <= 0x007F) {
		PUTC(c, u & 0x007F);
	}
	else  if (u >= 0x0080 && u <= 0x07FF) {
		PUTC(c, 0xC0 | ((u >> 6) & 0x001F));
		PUTC(c, 0x80 | (u & 0x003F));
	}
	else if (u >= 0x0800 && u <= 0xFFFF) {
		PUTC(c, 0xE0 | ((u >> 12) & 0x000F));
		PUTC(c, 0x80 | ((u >> 6) & 0x003F));
		PUTC(c, 0x80 | (u & 0x003F));
	}
	else if (u >= 0x10000 && u <= 0x10FFFF) {
		PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
		PUTC(c, 0x80 | ((u >> 12) & 0x3F));
		PUTC(c, 0x80 | ((u >> 6) & 0x3F));
		PUTC(c, 0x80 | (u & 0x3F));
	}
	else return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
	//else if(*u >= 0x10000 && *u <= 0x10FFFF)
	return LEPT_PARSE_OK;
}

size_t lept_get_array_size(const lept_value *v) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	return v->u.arr.size;
}

lept_value * lept_get_array_element(const lept_value *v, size_t n) {
	assert(v != NULL && v->type == LEPT_ARRAY);
	assert(n <= v->u.arr.size);
	return &v->u.arr.e[n];
}

char *lept_parse_hex4(char *p, unsigned *u) {
	size_t i;
	*u = 0x00;
	for (i = 0; i < 4; i++) {
		if (!(ISHEX(*(p + i)))) {
			return NULL;
		}
		else {
			*u = *u << 4;
			//*u += (*(p + i) - '0' >= 10) ? (*(p + i) - 'A' + 10) : (*(p + i) - '0');
			if (*(p + i) >= '0' && *(p + i) <= '9')  *u += *(p + i) - '0';
			else if (*(p + i) >= 'A' && *(p + i) <= 'F')  *u += *(p + i) - ('A' - 10);
			else if (*(p + i) >= 'a' && *(p + i) <= 'f')  *u += *(p + i) - ('a' - 10);
/* 如果使用strtol
static const char* lept_parse_hex4(const char* p, unsigned* u) {
	char* end;
	*u = (unsigned)strtol(p, &end, 16);
	return end == p + 4 ? end : NULL;
}
但这个实现会错误地接受 "\u 123" 这种不合法的 JSON，
因为 strtol() 会跳过开始的空白。要解决的话，还需要检测第一个字符是否 [0-9A-Fa-f]，或者 !isspace(*p)*/
		}
	}
	return p += 4;
}

void lept_set_null(lept_value *v) {
	assert(v != NULL);
	lept_free(v);
	v->type = LEPT_NULL;
}

static int lept_parse_array(lept_context *c, lept_value *v) {
	size_t size = 0, i;
	int ret;
	EXPECT(c, '[');
	lept_parse_whitespace(c);
	if (*c->json == ']') {
		c->json++;
		v->type = LEPT_ARRAY;
		v->u.arr.size = 0;
		v->u.arr.e = NULL;
		return LEPT_PARSE_OK;
	}
	while (1) {
		lept_value e;
		lept_init(&e);
		lept_parse_whitespace(c);
		if (ret = lept_parse_value(c, &e) != LEPT_PARSE_OK)
			break;
		memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value));
		size++;
		lept_parse_whitespace(c);
		if (*c->json == ',')
			c->json++;
		else if (*c->json == ']') {
			c->json++;
			v->type = LEPT_ARRAY;
			v->u.arr.size = size;
			size *= sizeof(lept_value);
			memcpy(v->u.arr.e = (lept_value *)malloc(size), lept_context_pop(c, size), size);
			return LEPT_PARSE_OK;
		}
		else {
			ret = LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			break;
		}
	}
	/* Pop and free values on the stack */
	for (i = 0; i < size; i++)
		lept_free((lept_value*)lept_context_pop(c, sizeof(lept_value)));
	v->type = LEPT_VOID;
	return ret;
}
/*
我们把这个指针调用 lept_parse_value(c, e)，
这里会出现问题，因为 lept_parse_value() 及之下的函数都需要调用 lept_context_push()，
而 lept_context_push() 在发现栈满了的时候会用 realloc() 扩容。
这时候，我们上层的 e 就会失效，变成一个悬挂指针（dangling pointer），
而且 lept_parse_value(c, e) 会通过这个指针写入解析结果，造成非法访问。
在使用 C++ 容器时，也会遇到类似的问题。从容器中取得的迭代器（iterator）后，
如果改动容器内容，之前的迭代器会失效。这里的悬挂指针问题也是相同的。

但这种 bug 有时可能在简单测试中不能自动发现，因为问题只有堆栈满了才会出现。
从测试的角度看，我们需要一些压力测试（stress test），测试更大更复杂的数据。
但从编程的角度看，我们要谨慎考虑变量的生命周期，尽量从编程阶段避免出现问题

例如把 lept_context_push() 的 API 改为：
static void lept_context_push(
lept_context* c, const void* data, size_t size);

这样就确把数据压入栈内，避免了返回指针的生命周期问题。但我们之后会发现，原来的 API 设计在一些情况会更方便一些，
例如在把字符串值转化（stringify）为 JSON 时，我们可以预先在堆栈分配字符串所需的最大空间，而当时是未有数据填充进去的。
无论如何，我们编程时都要考虑清楚变量的生命周期，特别是指针的生命周期。
*/

static int lept_parse_object(lept_context *c, lept_value *v) {
	int ret;
	size_t size, i;
	lept_member m;
	EXPECT(c, '{');
	lept_parse_whitespace(c);
	if (*c->json == '}') {
		c->json++;
		v->type = LEPT_OBJECT;
		v->u.o.m = 0;
		v->u.o.size = 0;
		return LEPT_PARSE_OK;
	}
	m.k = NULL;
	size = 0;
	while (1) {
		char* str;
		lept_init(&m.v);
		lept_parse_whitespace(c);
		if (*c->json != '"') {
			ret = LEPT_PARSE_MISS_KEY;
			break;
		}
		if ((ret = lept_parse_string_raw(c, &str, &m.klen)) != LEPT_PARSE_OK)
			break;
		memcpy(m.k = (char*)malloc(m.klen + 1), str, m.klen);
		m.k[m.klen] = '\0';
		lept_parse_whitespace(c);
		if (*c->json != ':') {
			ret = LEPT_PARSE_MISS_COLON;
			break;
		}
		c->json++;
		lept_parse_whitespace(c);
		if ((ret = lept_parse_value(c, &m.v)) != LEPT_PARSE_OK)
			break;
		memcpy(lept_context_push(c, sizeof(lept_member)), &m, sizeof(lept_member));
		size++;
		m.k = NULL;
		lept_parse_whitespace(c);
		if (*c->json == ',')
			c->json++;
		else if (*c->json == '}') {
			c->json++;
			v->type = LEPT_OBJECT;
			v->u.o.size = size;
			size *= sizeof(lept_member);
			memcpy(v->u.o.m = (lept_member *)malloc(size), lept_context_pop(c, size), size);
			return LEPT_PARSE_OK;
		}
		else {
			ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
			break;
		}
	}
	free(m.k);
	for (i = 0; i < size; i++) {
		lept_member *m;
		m = (lept_member *)lept_context_pop(c, sizeof(lept_member));
		free(m->k);
		lept_free(&(m->v));
	}
	v->type = LEPT_VOID;
	return ret;
}

size_t lept_get_object_size(const lept_value *v) {
	assert(v != NULL && v->type == LEPT_OBJECT);
	return v->u.o.size;
}

const char *lept_get_object_key(const lept_value *v, const size_t size) {
	assert(v != NULL && v->type == LEPT_OBJECT && size <= v->u.o.size);
	return v->u.o.m[size].k;
}

const size_t lept_get_object_key_len(const lept_value *v, const size_t size) {
	assert(v != NULL && v->type == LEPT_OBJECT && size <= v->u.o.size);
	return v->u.o.m[size].klen;
}

const lept_value *lept_get_object_value(const lept_value *v, const size_t size) {
	assert(v != NULL && v->type == LEPT_OBJECT && size <= v->u.o.size);
	return &v->u.o.m[size].v;
}
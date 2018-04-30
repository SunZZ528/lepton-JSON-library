﻿#define _WINDOWS
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
	char *stack;
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

int lept_parse(lept_value *v, const char *json) {
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
	assert(c.top == 0);
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
	if (v->type == LEPT_STRING) 
		free(v->u.s.s);
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
	ret = c->stack + c->top;
	c->top += size;
	return ret;
}

static void *lept_context_pop(lept_context *c, size_t size) {
	assert(c->top >= size);
	return c->stack + (c->top -= size);
}

static int lept_parse_string(lept_context *c, lept_value *v) {
	size_t head = c->top, len;
	const char *p;
	unsigned u, u_low;
	EXPECT(c, '\"');
	p = c->json;
	while (1) {
		char ch = *p++;
		switch (ch) {
			case '\"':
				len = c->top - head;
				lept_set_string(v, (const char *)lept_context_pop(c, len), len);
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
						if(!(p = lept_parse_hex4(p, &u)))
							STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
						if (u >= 0xD800 && u <= 0xD8FF) {
							if (*p++ != '\\')
								STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
							if(*p++ != 'u')
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
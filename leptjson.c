#include "leptjson.h"
#include <assert.h>  /* assert */
#include <stdlib.h>  /* NULL strtod() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */

typedef struct {
	const char *json;
}lept_context;

static int lept_parse_value(lept_context *, lept_value *);
static void lept_parse_whitespace(lept_context *);
static int lept_parse_number(lept_context *, lept_value *);
static int lept_parse_literal(lept_context *, lept_value *, const char *, const lept_type);

#define EXPECT(c, ch) \
	do { \
		assert(*c->json == (ch)); \
		c->json++; \
	} while(0)

#define ISDIGIT0_9(ch) ( (ch) >= '0' && (ch) <= '9')

#define ISDIGIT1_9(ch) ( (ch) >= '1' && (ch) <= '9')

int lept_get_type(const lept_value *v) {
	assert(v != NULL);
	return v->type;
}

int lept_parse(lept_value *v, const char *json) {
	lept_context c;
	int ret = 0;
	assert(v != NULL);
	c.json = json;
	v->type = LEPT_VOID;
	lept_parse_whitespace(&c);
	if((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
		lept_parse_whitespace(&c);
		if (*c.json != '\0') {
			ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
			v->type = LEPT_VOID;
		}
	}
	return ret;
}

static int lept_parse_value(lept_context *c, lept_value *v) {
	switch (*c->json) {
		case 'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
		case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
		case 'f': return lept_parse_literal(c, v, "false", LEPT_FALSE);
		case '\0': return LEPT_PARSE_EXPECT_VALUE;
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
	return v->n;
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
	v->n = strtod(c->json, &end);
	if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
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
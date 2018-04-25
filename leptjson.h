#ifndef LEPTJSON_H__
#define LEPTJSON_H__


typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT, LEPT_VOID}lept_type;

enum {
	LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG,
};

typedef struct {
	double n;
	lept_type type;
}lept_value;

int lept_get_type(const lept_value *);
int lept_parse(lept_value *, const char *);
double lept_get_number(const lept_value *);

#endif
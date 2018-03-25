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
// ����Ҫ��֮ǰ�� v->n �ĳ� v->u.n����Ҫ�����ַ��������ݣ���Ҫʹ�� v->u.s.s �� v->u.s.len������д���Ƚ��鷳�ɣ�
// ��ʵ C11 ���������� struct/union �﷨���Ϳ��Բ��� v->n��v->s��v->len �������ʡ�

typedef struct {
	union {
		struct { char *s; size_t len; }s;  /* string */
		double n;                       /* number */
	}u;  // Ҳ������
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
void lept_free(lept_value *);
int lept_get_boolean(const lept_value *);
void lept_set_boolean(lept_value *, int);
double lept_get_number(const lept_value *);
void lept_set_number(lept_value *, const double);
const char* lept_get_string(const lept_value *);
size_t lept_get_string_length(const lept_value *);
void lept_set_string(lept_value *, const char *, const size_t);

#endif /* LEPTJSON_H__ */

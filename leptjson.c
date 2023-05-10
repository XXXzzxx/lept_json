#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include "leptjson.h"
#define _CRT_SECURE_NO_WARNINGS
#define check09(ch) ((ch) >= '0' && (ch) <= '9')
#define check19(ch) ((ch) >= '1' && (ch) <= '9')

typedef struct 
{
	const char* json;
}lept_context;

void lept_parse_white(lept_context* c) {
	const char* p = c->json;
	while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r')
		p++;
	c->json = p;
}

int lept_parse_number(lept_context* c, lept_value* v) {
	//�������ֵĽ���
	//firsr ����(option) 
	//   f.1 ������0 ����һ������  + (һ��С����   +  һ������)(option)
	//   f.2 ��1������� + e/E + (һ������0��ͷ����)
	const char* p = c->json;
	if (*p == '-')
		p++;
	if (*p == '0')
		p++;
	else {
		if (!check09(*p))
			return LEPT_PARSE_INVALID_VALUE;
		while (check09(*p))
			p++;
	}
	if (*p == '.') {
		p++;
		if (!check09(*p))
			return LEPT_PARSE_INVALID_VALUE;
		while (check09(*p))
			p++;
	}
	if (*p == 'e' || *p == 'E') {
		p++;
		if (*p == '+' || *p == '-')
			p++;
		if (!check19(*p))
			return LEPT_PARSE_INVALID_VALUE;
		while (check09(*p))
			p++;
	}

	errno = 0;
	char* end;
	v->n = strtod(c->json, &end);
	if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
		return LEPT_PARSE_NUMBER_TOO_BIG;
	}
	v->type = LEPT_NUMBER;
	c->json = p;
	return LEPT_PARSE_OK;
	
}

int lept_parse_literal(lept_context* c, lept_value* v, const char* str, int type) {
	const char* p = c->json;
	while (*str != '\0') {
		if (*p != *str) {
			return LEPT_PARSE_INVALID_VALUE;
		}
		p++, str++;
	}
	c->json = p;
	v->type = type;
	return LEPT_PARSE_OK;
}

int lept_parse_value(lept_context* c, lept_value* v) {
	const char* p = c->json;
	switch (*p)
	{
	case'f': return lept_parse_literal(c, v, "flase", LEPT_FALSE);
	case'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
	case't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
	case'\0': return LEPT_PARSE_EXPEXT_VALUE; //Ϊ��
	default: return lept_parse_number(c, v);
		return ;
	}
}

int lept_parse(lept_value* v, const char* json) {
	lept_context c;
	c.json = json;
	printf("1\n");
	v->type = LEPT_NULL;
	lept_parse_white(&c);
	int ret = -1;
	if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
		lept_parse_white(&c);
		if (*c.json != '\0') {
			return LEPT_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	return ret;
}

//��ȡjson��type
lept_type lept_get_type(lept_value* v) {
	assert(v != NULL);
	return v->type;
}


//��������ֻ�ȡ����
double lept_get_number(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->n;
}
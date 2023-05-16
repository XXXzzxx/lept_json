#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include "leptjson.h"

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

#define _CRT_SECURE_NO_WARNINGS
#define check09(ch) ((ch) >= '0' && (ch) <= '9')
#define check19(ch) ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch) do{*(char *)lept_context_push(c, sizeof(char)) = (ch); }while(0)   
//*(char *) ���������ǿ��ת��, �����*�ǽ�����

typedef struct
{
	const char* json;
	char* stack; //��̬��ջ
	size_t size, top;
}lept_context;


static void* lept_context_push(lept_context* c, size_t len) {
	//������  �ڴ������дbug
	void* ret;
	assert(len >= 0);
	while (1) {
		if (c->top + len >= c->size) {
			if (c->size == 0)
				c->size = LEPT_PARSE_STACK_INIT_SIZE;
			while (c->top + len >= c->size) {
				c->size += c->size << 1; //����ϵ��1.5
			}
			c->stack = (char*)realloc(c->stack, c->size);
		}
		ret = c->stack + c->top; //ջ���ĺ�һ��λ��
		c->top += len; //top����push��ȥ�ĳ���
		return ret;
	}
}

static void* lept_context_pop(lept_context* c, size_t len) {
	assert(c->top >= len);

	//������Ҫ���ַ����ĵ�һλ��ַ
	return c->stack + (c->top -= len);
}


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
		if (!check09(*p)) //��߱�֤��һ���Ϸ�����ֻ�������ֿ�ͷ
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
	v->u.n = strtod(c->json, &end);
	if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL)) {
		return LEPT_PARSE_NUMBER_TOO_BIG;
	}
	v->type = LEPT_NUMBER;
	c->json = p;
	return LEPT_PARSE_OK;
	
}

//false true null  ����һ
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

int lept_parse_string(lept_context* c, lept_value* v) {
	const char* p = c->json;
	p++;
	size_t head = c->top;
	size_t len = 0;
	while (1) {
		char ch = *(p++);
		switch (ch) {
		case'\0':
			c->top = head;
			return LEPT_PARSE_MISSING_QUOTATION_MARK;
		case'\"':
			len = c->top - head;
			lept_set_string(v, (const char*)lept_context_pop(c, len), len);
			c->json = p;
			return LEPT_PARSE_OK;
		case'\\':
		{
			// ��ת���ַ�����ת��
			ch = *(p++);
			switch (ch){
			case 'n': PUTC(c, '\n'); break;
			case 't': PUTC(c, '\t'); break;
			case 'b': PUTC(c, '\b'); break;
			case 'r': PUTC(c, '\r'); break;
			case 'f': PUTC(c, '\f'); break;
			case '/': PUTC(c, '\/'); break;
			case '\\': PUTC(c, '\\'); break;
			case '\"': PUTC(c, '\"'); break;
			default:
				c->top = head;
				return LEPT_PARSE_INVALID_STRING;
			}
			break;  //switch ����break������ Ȼ��ͽ�����default
		}
		default:
			if (ch < 0x20) {
				c->top = head;
				return LEPT_PARSE_INVALID_STRING_CHAR;
			}
			PUTC(c, ch);
		}
	}
}

int lept_parse_value(lept_context* c, lept_value* v) {
	const char* p = c->json;
	switch (*p)
	{
	case'f': return lept_parse_literal(c, v, "flase", LEPT_FALSE);
	case'n': return lept_parse_literal(c, v, "null", LEPT_NULL);
	case't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
	case'\0': return LEPT_PARSE_EXPEXT_VALUE; //Ϊ��
	case'"': return lept_parse_string(c, v);
	default: return lept_parse_number(c, v);
	}
}

int lept_parse(lept_value* v, const char* json) {
	assert(v != NULL);
	lept_context c;
	c.stack = NULL;
	c.top = c.size = 0;
	c.json = json;

	v->type = LEPT_NULL;
	lept_parse_white(&c);
	int ret = -1;	
	if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
		lept_parse_white(&c);
		if (*c.json != '\0') {
			return LEPT_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	assert(c.top == 0);
	free(c.stack);
	return ret;
}

//��ȡjson��type
lept_type lept_get_type(lept_value* v) {
	assert(v != NULL);
	return v->type;
}

void lept_free(lept_value* v) {
	assert(v != NULL);
	if (v->type == LEPT_STRING) {
		free(v->u.str.s);
	}
	v->type = LEPT_NULL;
}

//��������ֻ�ȡ����
double lept_get_number(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->u.n;
}
void lept_set_number(lept_value* v, double n) {
	assert(v != NULL);
	v->u.n = n;
	v->type = LEPT_NUMBER;
}

int lept_get_boolean(const lept_value* v) {
	assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
	if (v->type == LEPT_TRUE)
		return 1;
	return 0;
}

void lept_set_boolean(lept_value* v, int b) {
	assert(v != NULL); //b��������0 1 ��
	if (b)
		v->type = LEPT_TRUE;
	else
		v->type = LEPT_FALSE;

}


const char* lept_get_string(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.str.s;

}
size_t lept_get_string_length(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_STRING);
	return v->u.str.len;
}
void lept_set_string(lept_value* v, const char* s, size_t len) {
	assert(v != NULL && (s != NULL || len == 0));
	lept_free(v);
	v->u.str.s = (char*)malloc(len + 1);
	memcpy(v->u.str.s, s, len);
	v->u.str.s[len] = '\0';
	v->u.str.len = len;
	v->type = LEPT_STRING;

}
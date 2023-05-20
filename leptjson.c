#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


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
#define return_error(re) do{c->top = head; return re;}while(0)
//*(char *) 括号里面的强制转化, 外面的*是解引用

typedef struct
{
	const char* json;
	char* stack; //动态堆栈
	size_t size, top;
}lept_context;


static void* lept_context_push(lept_context* c, size_t len) {
	//麻了捏  内存分配老写bug
	void* ret;
	assert(len >= 0);
	while (1) {
		if (c->top + len >= c->size) {
			if (c->size == 0)
				c->size = LEPT_PARSE_STACK_INIT_SIZE;
			while (c->top + len >= c->size) {
				c->size += c->size << 1; //扩容系数1.5
			}
			c->stack = (char*)realloc(c->stack, c->size);
		}
		ret = c->stack + c->top; //栈顶的后一个位置
		c->top += len; //top加上push进去的长度
		return ret;
	}
}

static void* lept_context_pop(lept_context* c, size_t len) {
	assert(c->top >= len);

	//返回需要的字符串的第一位地址
	return c->stack + (c->top -= len);
}


void lept_parse_white(lept_context* c) {
	const char* p = c->json;
	while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r')
		p++;
	c->json = p;
}

int lept_parse_number(lept_context* c, lept_value* v) {
	//对于数字的解析
	//firsr 负号(option) 
	//   f.1 纯单个0 或者一串数字  + (一个小数点   +  一串数字)(option)
	//   f.2 在1的情况下 + e/E + (一串不以0开头的数)
	const char* p = c->json;
	if (*p == '-')
		p++;
	if (*p == '0')
		p++;
	else {
		if (!check09(*p)) //这边保证了一个合法数字只能以数字开头
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

//false true null  三合一
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

static const char* lept_parse_hex4(const char* p, unsigned* u) {
	int t = 4;
	*u = 0;
	// u是一个无符号整数  将十六进制转化成10进制数
	while (t--)
	{
		char ch = *(p++);
		*u = *u << 4;  //每次*16
		if (ch >= '0' && ch <= '9')
			*u = *u | (ch - '0');
		else if (ch >= 'A' && ch <= 'F')
			*u = *u | (ch - 'A' + 10);
		else if (ch >= 'a' && ch <= 'f')
			*u = *u | (ch - 'a' + 10);
		else
			return NULL;
	}
	return p;
}

static void lept_encode_utf8(lept_context* c, unsigned u) {
	assert(u >= 0x0000 && u <= 0x10FFFF);

	if (u <= 0x007F && u >= 0x0000)
		PUTC(c, u & 0xFF); // 因为u < 127  所以转化成char类型 不会丢失数据;

	else if (u <= 0x07FF && u >= 0x0080) {
		PUTC(c, 0xC0 | ((u >> 6) & 0xDF));
		PUTC(c, 0x80 | ( u       & 0x3F));
	}
	else if (u <= 0xFFFF && u >= 0x0800) {
		PUTC(c, 0xE0 | ((u >> 12) & 0xEF));
		PUTC(c, 0x80 | ((u >> 6)  & 0x3F));
		PUTC(c, 0x80 | ( u        & 0x3F));
	}
	else if (u <= 0x10FFFF && u >= 0x10000) {
		PUTC(c, 0xF0 | ((u >> 18) & 0xF7));
		PUTC(c, 0x80 | ((u >> 12) & 0x3F));
		PUTC(c, 0x80 | ((u >> 6)  & 0x3F));
		PUTC(c, 0x80 | ( u        & 0x3F));
	}

}

int lept_parse_string(lept_context* c, lept_value* v) {
	const char* p = c->json;
	p++;
	size_t head = c->top;
	size_t len = 0;
	unsigned  ul = 0, u = 0, uh = 0;
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
			// 对转义字符进行转换
			ch = *(p++);
			switch (ch) {
				case 'n': PUTC(c, '\n'); break;
				case 't': PUTC(c, '\t'); break;
				case 'b': PUTC(c, '\b'); break;
				case 'r': PUTC(c, '\r'); break;
				case 'f': PUTC(c, '\f'); break;
				case '/': PUTC(c, '/'); break;
				case '\\': PUTC(c, '\\'); break;
				case '\"': PUTC(c, '\"'); break;
				case 'u': 
					if (!(p = lept_parse_hex4(p, &uh))) {
						return_error(LEPT_PARSE_INVALID_UNICODE_HEX); //十六进制格式错误
					}
					u = uh;//格式正确

					//一下是有高低代理项
					if (u <= 0xDBFF && u >= 0xD800) {
						ch = *(p++);		
						if (ch != '\\')
							return_error(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
				
						ch = *(p++);
						if (ch != 'u')
							return_error(LEPT_PARSE_INVALID_UNICODE_SURROGATE);

						if (!(p = lept_parse_hex4(p, &ul)))
							return_error(LEPT_PARSE_INVALID_UNICODE_HEX);

						if (ul < 0xDC00 || ul > 0xDFFF)
							return_error(LEPT_PARSE_INVALID_UNICODE_SURROGATE);

						// 这是一个计算码点
						u = 0x10000 + (uh - 0xD800) * 0x400 + (ul - 0xDC00);

					}
					lept_encode_utf8(c, u);
					break;
				default:
					c->top = head;
					return LEPT_PARSE_INVALID_STRING;
				}
				break;  //switch 语句的break老忘记 然后就进入了default
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
	case'\0': return LEPT_PARSE_EXPEXT_VALUE; //为空
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

//获取json的type
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

//如果是数字获取数字
double lept_get_number(const lept_value* v) {
	assert(v != NULL && v->type == LEPT_NUMBER);
	return v->u.n;
}
void lept_set_number(lept_value* v, double n) {
	lept_free(v);
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
	lept_free(v);
	assert(v != NULL); //b不限制在0 1 中
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
#ifndef LEPTJSON_H__
#define LEPTJSON_H__

typedef enum {
	LEPT_NULL,
	LEPT_FALSE,
	LEPT_TRUE,
	LEPT_NUMBER,
	LEPT_STRING,
	LEPT_ARRAY,
	LEPT_OBJECT,
	LEPT_BOOLEAN,
}lept_type;

typedef struct lept_value lept_value;

struct lept_value {
	union {
		double n;

		struct { lept_value* e; size_t size;}arr;

		struct { char* s; size_t len;}str;
	}u;

	lept_type type;
};

enum {
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPEXT_VALUE,		//1 为空
	LEPT_PARSE_INVALID_VALUE,		//2 非法值  就是解释的时候可能开头对上了,后面字符没有对上
	LEPT_PARSE_INVALID_STRING,		//3 string非法  比如"\l" 这个就是非法的无法解析
	LEPT_PARSE_ROOT_NOT_SINGULAR,	//4 不止一种类型
	LEPT_PARSE_NUMBER_TOO_BIG,		//5 数值超范围
	LEPT_PARSE_MISSING_QUOTATION_MARK, //6 stirng缺少另一个引号
	LEPT_PARSE_INVALID_STRING_CHAR,	//7 因为ASCLL码中小于0x20的字符都是非法的
	LEPT_PARSE_INVALID_UNICODE_HEX,	//8 unicode十六进制格式错误
	LEPT_PARSE_INVALID_UNICODE_SURROGATE,	//9 unicode高代理项缺少低代理项
	LEPT_ENCODE_UTF8_ERROR,			//10 
	LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET	//11 数组缺少]
};

#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)
#define lept_set_null(v) lept_free(v)

int lept_parse(lept_value* v, const char* json);

lept_type lept_get_type(lept_value* v);

//boolean
int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);

//数字
double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double n);

//字符串
const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);
void lept_free(lept_value* v);

//数组
size_t lept_get_arr_length(const lept_value* v);
lept_value* lept_get_arr_element(const lept_value* v, size_t index);
#endif // !LEPTJSON_H__

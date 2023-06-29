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
	LEPT_PARSE_EXPEXT_VALUE,		//1 Ϊ��
	LEPT_PARSE_INVALID_VALUE,		//2 �Ƿ�ֵ  ���ǽ��͵�ʱ����ܿ�ͷ������,�����ַ�û�ж���
	LEPT_PARSE_INVALID_STRING,		//3 string�Ƿ�  ����"\l" ������ǷǷ����޷�����
	LEPT_PARSE_ROOT_NOT_SINGULAR,	//4 ��ֹһ������
	LEPT_PARSE_NUMBER_TOO_BIG,		//5 ��ֵ����Χ
	LEPT_PARSE_MISSING_QUOTATION_MARK, //6 stirngȱ����һ������
	LEPT_PARSE_INVALID_STRING_CHAR,	//7 ��ΪASCLL����С��0x20���ַ����ǷǷ���
	LEPT_PARSE_INVALID_UNICODE_HEX,	//8 unicodeʮ�����Ƹ�ʽ����
	LEPT_PARSE_INVALID_UNICODE_SURROGATE,	//9 unicode�ߴ�����ȱ�ٵʹ�����
	LEPT_ENCODE_UTF8_ERROR,			//10 
	LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET	//11 ����ȱ��]
};

#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)
#define lept_set_null(v) lept_free(v)

int lept_parse(lept_value* v, const char* json);

lept_type lept_get_type(lept_value* v);

//boolean
int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);

//����
double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double n);

//�ַ���
const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);
void lept_free(lept_value* v);

//����
size_t lept_get_arr_length(const lept_value* v);
lept_value* lept_get_arr_element(const lept_value* v, size_t index);
#endif // !LEPTJSON_H__

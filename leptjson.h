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

typedef struct {
	union {
		double n;
		struct {
			char* s;
			size_t len;
		}str;
	}u;
	
	lept_type type;
}lept_value;

enum {
	LEPT_PARSE_OK = 0,
	LEPT_PARSE_EXPEXT_VALUE,
	LEPT_PARSE_INVALID_VALUE,
	LEPT_PARSE_INVALID_STRING,
	LEPT_PARSE_ROOT_NOT_SINGULAR,
	LEPT_PARSE_NUMBER_TOO_BIG,
	LEPT_PARSE_MISSING_QUOTATION_MARK, 
	LEPT_PARSE_INVALID_STRING_CHAR,
	LEPT_PARSE_INVALID_UNICODE_HEX,
	LEPT_PARSE_INVALID_UNICODE_SURROGATE,
	LEPT_ENCODE_UTF8_ERROR
};

#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)
#define lept_set_null(v) lept_free(v)

int lept_parse(lept_value* v, const char* json);

lept_type lept_get_type(lept_value* v);

//boolean
int lept_get_boolean(const lept_value* v);
void lept_set_boolean(lept_value* v, int b);

//Êý×Ö
double lept_get_number(const lept_value* v);
void lept_set_number(lept_value* v, double n);

//×Ö·û´®
const char* lept_get_string(const lept_value* v);
size_t lept_get_string_length(const lept_value* v);
void lept_set_string(lept_value* v, const char* s, size_t len);
void lept_free(lept_value* v);

#endif // !LEPTJSON_H__

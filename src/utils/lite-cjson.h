#ifndef __LITE_CJSON_H__
#define __LITE_CJSON_H__

#ifndef _IN_
#define _IN_
#endif

#ifndef _OU_
#define _OU_
#endif

/* cJSON Types: */
#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7) /* raw json */

#ifndef LITE_CJSON_NESTING_LIMIT
#define LITE_CJSON_NESTING_LIMIT 1000
#endif

/* The cJSON structure: */
typedef struct lite_cjson_st {
    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==cJSON_String  and type == cJSON_Raw */
    char* value;
    int value_length;

	/* The item's size, if type == cJSON_Array and type == cJSON_Object */
    int size;

	double value_double;
	int value_int;
} lite_cjson_t;

int lite_cjson_parse(_IN_ const char *src, _IN_ int src_len, _OU_ lite_cjson_t *lite);

int lite_cjson_is_false(_IN_ lite_cjson_t *lite);
int lite_cjson_is_true(_IN_ lite_cjson_t *lite);
int lite_cjson_is_null(_IN_ lite_cjson_t *lite);
int lite_cjson_is_number(_IN_ lite_cjson_t *lite);
int lite_cjson_is_string(_IN_ lite_cjson_t *lite);
int lite_cjson_is_array(_IN_ lite_cjson_t *lite);
int lite_cjson_is_object(_IN_ lite_cjson_t *lite);

int lite_cjson_array_item(_IN_ lite_cjson_t *lite, _IN_ int index, _OU_ lite_cjson_t *lite_item);
int lite_cjson_object_item(_IN_ lite_cjson_t *lite, _IN_ const char *key, _IN_ int key_len, _OU_ lite_cjson_t *lite_item);

#endif


#ifndef JSON_PARSER_H
#define JSON_PARSER_H

/**
The descriptions of the json value node type
**/
enum JSONTYPE {
    JNONE = -1,
    JSTRING = 0,
    JOBJECT,
    JARRAY,
    JNUMBER,
    JBOOLEAN,
    JTYPEMAX
};

/**
The error codes produced by the JSON parsers
**/
enum JSON_PARSE_CODE {
    JSON_PARSE_ERR,
    JSON_PARSE_OK,
    JSON_PARSE_FINISH
};

/**
The return codes produced by the JSON parsers
**/
enum JSON_PARSE_RESULT {
    JSON_RESULT_ERR = -1,
    JSON_RESULT_OK
};

#define JSON_DEBUG 0
typedef int (*json_parse_cb)(const char* p_cName, int iNameLen, const char* p_cValue, int iValueLen, int iValueType, void* p_Result);

/**
* @brief Parse the JSON string, and iterate through all keys and values,
* then handle the keys and values by callback function.
*
* @param[in]  p_cJsonStr @n  The JSON string
* @param[in]  iStrLen    @n  The JSON string length
* @param[in]  pfnCB      @n  Callback function
* @param[out] p_CBData   @n  User data
* @return JSON_RESULT_OK success, JSON_RESULT_ERR failed
* @see None.
* @note None.
**/
int json_parse_name_value(const char* p_cJsonStr, int iStrLen, json_parse_cb pfnCB, void* p_CBData);

/**
* @brief Get the value by a specified key from a json string
*
* @param[in]  p_cJsonStr   @n the JSON string
* @param[in]  iStrLen      @n the JSON string length
* @param[in]  p_cName      @n the specified key string
* @param[out] p_iValueLen  @n the value length
* @param[out] p_iValueType @n the value type
* @return A pointer to the value
* @see None.
* @note None.
**/
const char *json_get_value_by_name(const char *p_cJsonStr, int iStrLen, const char *p_cName, int *p_iValueLen, int *p_iValueType);


/**
* @brief Get the value by a full key(like "topkey.subkey") from a json string.
*        If you have a JSON string like {{k1:v1,k2:{k21:v21,k22:v22}}},
*        you can get the value of k22 of k2 by:
*        json_get_value_by_fullname(p_cJsonStr, iStrLen, "k2.k22", NULL, NULL);
*
* @param[in]  p_cJsonStr   @n the JSON string
* @param[in]  iStrLen      @n the JSON string length
* @param[in]  p_cName      @n the specified key string
* @param[out] p_iValueLen  @n the value length
* @param[out] p_iValueType @n the value type
* @return A pointer to the value
* @see None.
* @note None.
**/
const char *json_get_value_by_fullname(const char *p_cJsonStr, int iStrLen, const char *p_cName, int *p_iValueLen, int *p_iValueType);


/**
* @brief Get the length of a json string
*
* @param[in]  json_str @n The JSON string
* @param[in]  str_len  @n The JSON string length
* @returns Array size
* @see None.
* @note None.
**/
int json_get_array_size(const char *json_str, int str_len);

/**
 * @brief Get the JSON object point associate with a given type.
 *
 * @param[in] type @n The object type
 * @param[in] str  @n The JSON string
 * @returns The json object point with the given field type.
 * @see None.
 * @note None.
 */
const char *json_get_object(const char *pjson, size_t json_len, int type);
const char *json_get_next_object(const char *pjson, size_t json_len, int type, const char *str, const char **key, int *key_len, const char **val, int *val_len, int *val_type);
/**
 * @brief retrieve each key&value pair from the json string
 *
 * @param[in]  str   @n Json string to revolve
 * @param[in]  pos   @n cursor
 * @param[out] key   @n pointer to the next Key object
 * @param[out] klen  @n Key object length
 * @param[out] val   @n pointer to the next Value object
 * @param[out] vlen  @n Value object length
 * @param[out] vtype @n Value object type(digital, string, object, array)
 * @see None.
 * @note None.
 */
#define json_object_for_each_kv(json, json_len, pos, key, klen, val, vlen, vtype) \
    for (pos = json_get_object(json, json_len, JOBJECT); \
         (pos != 0) && (*pos != 0) && ((pos - json) < json_len) && (pos = json_get_next_object(json, json_len, JOBJECT, pos, &key, &klen, &val, &vlen, &vtype))!=0; )

/**
 * @brief retrieve each entry from the json array
 *
 * @param[in]  str   @n Json array to revolve
 * @param[in]  pos   @n cursor
 * @param[out] entry @n pointer to the next entry from the array
 * @param[out] len   @n entry length
 * @param[out] type  @n entry type(digital, string, object, array)
 * @see None.
 * @note None.
 */
#define json_array_for_each_entry(json, json_len, pos, entry, len, type) \
    for (pos = json_get_object(json, json_len, JARRAY); \
        (pos != 0) && (*pos != 0) && ((pos - json) < json_len) && (pos = json_get_next_object(json, json_len, JARRAY, ++pos, 0, 0, &entry, &len, &type))!=0; )

#endif

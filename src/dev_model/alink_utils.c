/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"

char *alink_utils_strdup(const char *string, uint32_t string_len)
{
    char *copy;

    if (string == NULL /*|| string_len == 0*/) {        /* TODO */
        return NULL;
    }

    copy = alink_malloc(string_len + 1);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, string, string_len);
    *(copy + string_len) = '\0';

    return copy;
}

int alink_utils_json_parse(const char *payload, uint32_t payload_len, uint32_t type, lite_cjson_t *lite)
{
    int res = 0;

    if (payload == NULL || payload_len == 0 || type == cJSON_Invalid || lite == NULL) {
        return IOTX_CODE_PARAMS_INVALID;
    }
    memset(lite, 0, sizeof(lite_cjson_t));

    res = lite_cjson_parse(payload, payload_len, lite);
    if (res != SUCCESS_RETURN) {
        memset(lite, 0, sizeof(lite_cjson_t));
        return FAIL_RETURN;
    }

    if (lite->type != type) {
        memset(lite, 0, sizeof(lite_cjson_t));
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

int alink_utils_json_object_item(lite_cjson_t *lite, const char *key, uint32_t key_len, uint32_t type, lite_cjson_t *lite_item)
{
    int res = 0;

    /* TODO */
    if (lite == NULL || lite->type != cJSON_Object || key == NULL || key_len == 0 || type == cJSON_Invalid || lite_item == NULL) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    memset(lite_item, 0, sizeof(lite_cjson_t));

    res = lite_cjson_object_item(lite, key, key_len, lite_item);
    if (res != SUCCESS_RETURN) {
        memset(lite_item, 0, sizeof(lite_cjson_t));
        return FAIL_RETURN;
    }

    if (lite_item->type != type) {
        memset(lite_item, 0, sizeof(lite_cjson_t));
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

void *alink_utils_malloc(uint32_t size)
{
    return alink_malloc(size);
}

void alink_utils_free(void *ptr)
{
    alink_free(ptr);
}
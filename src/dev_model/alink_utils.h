/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_UTILS_H__
#define __ALINK_UTILS_H__

#include "iotx_alink_internal.h"

char *alink_utils_strdup(const char *string, uint32_t string_len);
int alink_utils_json_parse(const char *payload, uint32_t payload_len, uint32_t type, lite_cjson_t *lite);
int alink_utils_json_object_item(lite_cjson_t *lite, const char *key, uint32_t key_len, uint32_t type, lite_cjson_t *lite_item);
void *alink_utils_malloc(uint32_t size);
void alink_utils_free(void *ptr);

#endif /* #ifndef __ALINK_UTILS_H__ */


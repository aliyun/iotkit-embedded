/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_UTILS_H__
#define __ALINK_UTILS_H__

#include "iotx_alink_internal.h"

typedef struct _hash_node *uri_hash_table_t;

typedef struct {
    const char *uri_string;
    alink_downstream_handle_func_t  handle_func;
} alink_uri_handle_pair_t;

typedef struct _hash_node {
    const alink_uri_handle_pair_t *pair;
    struct _hash_node *next;
} uri_hash_node_t;


int utils_uri_hash_init(const alink_uri_handle_pair_t *uri_handle_pair, uint8_t pair_num, uri_hash_table_t *table);
uri_hash_node_t *utils_uri_hash_search(const char *uri_string, uint8_t uri_len, uri_hash_table_t *table);
void utils_uri_hash_destroy(uri_hash_table_t *table);

char *alink_utils_strdup(const char *string, uint32_t string_len);
int alink_utils_json_parse(const char *payload, uint32_t payload_len, uint32_t type, lite_cjson_t *lite);
int alink_utils_json_object_item(lite_cjson_t *lite, const char *key, uint32_t key_len, uint32_t type, lite_cjson_t *lite_item);

#endif /* #ifndef __ALINK_UTILS_H__ */
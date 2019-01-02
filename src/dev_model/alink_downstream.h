/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_DOWNSTREAM_H__
#define __ALINK_DOWNSTREAM_H__

#include "iotx_alink_internal.h"


/** TODO **/
#define ALINK_URI_MAX_LEN           50
#define HASH_TABLE_SIZE_MAX         29 /* 13, 17, 19 ,23, 29, 31, 37, 41, 43, 47, 53, 59*/



typedef void (*alink_downstream_handle_func_t)(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);


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





int alink_downstream_hash_table_init(void);
void alink_downstream_hash_table_deinit(void);
alink_downstream_handle_func_t alink_downstream_get_handle_func(const char *uri_string, uint8_t uri_len);


#endif /* #ifndef __ALINK_DOWNSTREAM_H__ */


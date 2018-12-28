/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_DOWNSTREAM_H__
#define __ALINK_DOWNSTREAM_H__

#include "iotx_alink_internal.h"


typedef void (*alink_downstream_handle_func_t)(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

int alink_downstream_hash_table_init(void);
void alink_downstream_hash_table_deinit(void);
alink_downstream_handle_func_t alink_downstream_get_handle_func(const char *uri_string, uint8_t uri_len);


#endif /* #ifndef __ALINK_DOWNSTREAM_H__ */


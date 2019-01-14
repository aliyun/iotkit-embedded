/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_DOWNSTREAM_H__
#define __ALINK_DOWNSTREAM_H__

#include "infra_types.h"
#include "alink_format.h"

typedef void (*alink_downstream_handle_func_t)(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

int alink_msg_list_init(void);
void alink_msg_list_deinit(void);
int alink_msg_event_list_handler(void);

int alink_uri_hash_table_init(void);
void alink_uri_hash_table_deinit(void);
alink_downstream_handle_func_t alink_get_uri_handle_func(const char *uri_string, uint8_t uri_len);

#endif /* #ifndef __ALINK_DOWNSTREAM_H__ */


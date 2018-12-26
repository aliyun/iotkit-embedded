/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_CORE_H__
#define __ALINK_CORE_H__

#include "iotx_alink_internal.h"

#include "infra_defs.h"
#include "alink_api.h"      // TODO


int alink_core_open(iotx_dev_meta_info_t *dev_info);
int alink_core_connect_cloud(void);
int alink_core_yield(uint32_t timeout_ms);
void alink_core_get_dev_info(char **product_key, char **device_name);

int alink_subdev_open(iotx_dev_meta_info_t *dev_info);
int alink_core_subdev_connect_cloud(uint32_t devid);
uint32_t alink_core_get_msgid(void);
int alink_core_send_req_msg(char *uri, const uint8_t *payload, uint32_t len);
int alink_core_subscribe_downstream(void);



linkkit_event_cb_t alink_get_event_callback(iotx_linkkit_event_type_t event_id);
int alink_set_event_callback(iotx_linkkit_event_type_t event_id, linkkit_event_cb_t callback);


/* sudev funtions prototypes */
int alink_subdev_search_devid_by_pkdn(const char *product_key, const char *device_name, uint32_t *devid);






#endif /* #ifndef __ALINK_CORE_H__ */
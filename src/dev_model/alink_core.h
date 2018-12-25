/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_CORE_H__
#define __ALINK_CORE_H__

#include "iotx_alink_internal.h"

#include "infra_defs.h"



int alink_core_open(iotx_dev_meta_info_t *dev_info);
int alink_core_connect_cloud(void);
int alink_core_yield(uint32_t timeout_ms);

int alink_subdev_open(iotx_dev_meta_info_t *dev_info);
int alink_core_subdev_connect_cloud(uint32_t devid);

uint32_t alink_core_get_msgid(void);

int alink_core_send_req_msg(char *uri, uint8_t *payload, uint32_t len);


int alink_core_subscribe_downstream(void);



#endif /* #ifndef __ALINK_CORE_H__ */
/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_CORE_H__
#define __ALINK_CORE_H__

#include "iotx_alink_internal.h"

#include "infra_defs.h"



int alink_core_open(iotx_dev_meta_info_t *dev_info);
int alink_core_connect_cloud(void);

int alink_core_send_msg(alink_msg_uri_metadata_t *uri_meta, uint8_t *payload, uint32_t len);


#endif /* #ifndef __ALINK_CORE_H__ */
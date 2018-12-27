/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_CORE_H__
#define __ALINK_CORE_H__

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"
#include "alink_api.h"


typedef enum {
    ALINK_DEV_STATUS_UNAUTHORIZED,    /* Subdev Created */
    ALINK_DEV_STATUS_AUTHORIZED,      /* Receive Topo Add Notify */
    ALINK_DEV_STATUS_REGISTERED,      /* Receive Subdev Registered */
    ALINK_DEV_STATUS_ATTACHED,        /* Receive Subdev Topo Add Reply */
    ALINK_DEV_STATUS_LOGINED,         /* Receive Subdev Login Reply */
    ALINK_DEV_STATUS_ONLINE,          /* After All Topic Subscribed */
    ALINK_DEV_STATUS_ENABLE,          /* enabled */
    ALINK_DEV_STATUS_DISABLE          /* disabled by cloud */
} alink_device_status_t;


int alink_core_open(iotx_dev_meta_info_t *dev_info);
int alink_core_close(void);
int alink_core_connect_cloud(void);
int alink_core_yield(uint32_t timeout_ms);


int alink_subdev_open(iotx_dev_meta_info_t *dev_info);
int alink_subdev_connect_cloud(uint32_t devid);

uint32_t alink_core_get_msgid(void);
int alink_core_send_req_msg(char *uri, const uint8_t *payload, uint32_t len);



linkkit_event_cb_t alink_get_event_callback(iotx_linkkit_event_type_t event_id);
int alink_set_event_callback(iotx_linkkit_event_type_t event_id, linkkit_event_cb_t callback);









#endif /* #ifndef __ALINK_CORE_H__ */
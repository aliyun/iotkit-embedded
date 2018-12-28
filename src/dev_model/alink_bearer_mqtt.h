/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_BEARER_MQTT__
#define __ALINK_BEARER_MQTT__

#include "iotx_alink_internal.h"


/**
 * mqtt link status
 */
typedef enum {
    BEARER_MQTT_STATUS_OPENED,
    BEARER_MQTT_STATUS_CLOSED,
    BEARER_MQTT_STATUS_CONNECTED,
    BEARER_MQTT_STATUS_DISCONNECTED,
    BEARER_MQTT_STATUS_RECONNECTING
} alink_bearer_mqtt_status_t;

/**
 * mqtt link context
 */
typedef struct {
    alink_bearer_mqtt_status_t      status;
    iotx_mqtt_region_types_t        region;
    iotx_dev_meta_info_t           *dev_info;
    alink_bearer_node_t             bearer;
} alink_bearer_mqtt_ctx_t;


int32_t alink_bearer_mqtt_open(alink_bearer_mqtt_ctx_t *p_bearer_mqtt_ctx);


#endif /* #ifndef __ALINK_BEARER_MQTT__ */


/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_BEARER_MQTT__
#define __ALINK_BEARER_MQTT__

#include "alink_wrapper.h"

typedef struct {


} alink_bearer_mqtt_t;


int alink_bearer_mqtt_open(alink_bearer_ctx_t *p_bearer_ctx);


/* copy from cm, TODO */
typedef enum {
    /* cloud connected */
    IOTX_CM_EVENT_CLOUD_CONNECTED  = 0,
    /* cloud: disconnect */
    /* event_msg is null */
    IOTX_CM_EVENT_CLOUD_CONNECT_FAILED,
    /* cloud: disconnect */
    /* event_msg is null */
    IOTX_CM_EVENT_CLOUD_DISCONNECT,
    /* event_msg is iotx_cm_event_result_pt */
    IOTX_CM_EVENT_SUBCRIBE_SUCCESS,
    IOTX_CM_EVENT_SUBCRIBE_FAILED,
    IOTX_CM_EVENT_UNSUB_SUCCESS,
    IOTX_CM_EVENT_UNSUB_FAILED,
    IOTX_CM_EVENT_PUBLISH_SUCCESS,
    IOTX_CM_EVENT_PUBLISH_FAILED,
    /* Maximum number of event */
    IOTX_CM_EVENT_MAX
} alink_bearer_event_type_t;


#endif /* #ifndef __ALINK_BEARER_MQTT__ */
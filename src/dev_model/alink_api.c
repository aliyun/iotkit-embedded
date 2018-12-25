/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_api.h"

static linkkit_event_cb_t g_linkkit_event_array[ITE_EVENT_NUM] = { NULL };
/**
 * 
 */
int IOT_Linkkit_Open(iotx_linkkit_dev_type_t dev_type, iotx_dev_meta_info_t *meta_info)
{
    int res = FAIL_RETURN;

    if (dev_type < 0 || dev_type >= IOTX_LINKKIT_DEV_TYPE_MAX || meta_info == NULL) {
        return res;
    }

    if (IOTX_LINKKIT_DEV_TYPE_MASTER == dev_type) {
        res = alink_core_open(meta_info);
    }
    else if (IOTX_LINKKIT_DEV_TYPE_SLAVE == dev_type) {
#ifdef DEVICE_MODEL_GATEWAY        
        res = alink_subdev_open(meta_info);
#else
        res = IOTX_CODE_GATEWAY_UNSUPPORTED;
#endif
    }

    return res;
}

int IOT_Linkkit_Connect(int devid)
{
    if (devid < 0) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    int res = FAIL_RETURN;

    if (devid == IOTX_LINKKIT_DEV_TYPE_MASTER) {
        res = alink_core_connect_cloud();
    } else {
#ifdef DEVICE_MODEL_GATEWAY
        res = alink_core_subdev_connect_cloud(devid);
#else
        res = IOTX_CODE_GATEWAY_UNSUPPORTED;
#endif
    }

    return res;
}

void IOT_Linkkit_Yield(int timeout_ms)
{
    if (timeout_ms < 0) {
        return;
    }

    alink_core_yield(timeout_ms);
    return;
}

int IOT_Linkkit_Close(int devid)
{
    if (devid < 0) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    int res = FAIL_RETURN;

    if (devid == IOTX_LINKKIT_DEV_TYPE_MASTER) {
        ;       // TODO
    } else {
#ifdef DEVICE_MODEL_GATEWAY
        ;       // TODO
#else
        res = IOTX_CODE_GATEWAY_UNSUPPORTED;
#endif
    }

    return res;
}

int IOT_Linkkit_Report(int devid, iotx_linkkit_msg_type_t msg_type, unsigned char *payload,
                                   int payload_len)
{
    int res = FAIL_RETURN;

    if (devid < 0 || msg_type < 0 || msg_type >= IOTX_LINKKIT_MSG_MAX) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    switch (msg_type) {
        case ITM_MSG_POST_PROPERTY: {
            if (payload == NULL || payload_len <= 0) {
                return IOTX_CODE_PARAMS_INVALID;
            }
            res = alink_upstream_thing_property_post_req("pk1", "dn2", payload, payload_len);
        } break;
        case ITM_MSG_DEVICEINFO_UPDATE: {

        } break;
        case ITM_MSG_DEVICEINFO_DELETE:
        case ITM_MSG_POST_RAW_DATA:
        case ITM_MSG_LOGIN:
        case ITM_MSG_LOGOUT:
        default: break;
    }
    
    return res;
}

int IOT_Linkkit_Query(int devid, iotx_linkkit_msg_type_t msg_type, unsigned char *payload,
                                  int payload_len)
{
    return 0;
}

int IOT_Linkkit_TriggerEvent(int devid, char *eventid, int eventid_len, char *payload, int payload_len)
{
    return 0;
}

int IOT_RegisterCallback(iotx_linkkit_event_typde_t event_id, linkkit_event_cb_t callback)
{
    if (event_id < 0 || event_id >= ITE_EVENT_NUM || callback == NULL) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    g_linkkit_event_array[event_id] = callback;
    return SUCCESS_RETURN;
}

linkkit_event_cb_t alink_get_event_callback(iotx_linkkit_event_typde_t event_id)
{
    if (event_id < 0 || event_id >= ITE_EVENT_NUM) {
        return NULL;
    }

    return g_linkkit_event_array[event_id];
}
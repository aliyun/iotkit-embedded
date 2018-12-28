/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_api.h"


/** **/
int IOT_Linkkit_Open(iotx_linkkit_dev_type_t dev_type, iotx_dev_meta_info_t *meta_info)
{
    int res = FAIL_RETURN;

    if (dev_type < 0 || dev_type >= IOTX_LINKKIT_DEV_TYPE_MAX || meta_info == NULL) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    if (strlen(meta_info->product_key) > IOTX_PRODUCT_KEY_LEN || strlen(meta_info->product_secret) > IOTX_PRODUCT_SECRET_LEN
        || strlen(meta_info->device_name) > IOTX_DEVICE_NAME_LEN || strlen(meta_info->device_secret) > IOTX_DEVICE_SECRET_LEN) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    if (strlen(meta_info->product_key) == 0 || strlen(meta_info->device_name) == 0) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    if (IOTX_LINKKIT_DEV_TYPE_MASTER == dev_type) {
        res = alink_core_open(meta_info);
        if (SUCCESS_RETURN == res) {
#ifdef DEVICE_MODEL_GATEWAY             
            res = alink_subdev_mgr_init();       /* TODO!!! */
            if (SUCCESS_RETURN == res)
#endif            
                res = ALINK_DEVICE_SELF_ID;
        }
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
        res = alink_subdev_connect_cloud(devid);
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
        res = alink_core_close();
    } 
    else {
#ifdef DEVICE_MODEL_GATEWAY
        ;       /* TODO */
#else
        res = IOTX_CODE_GATEWAY_UNSUPPORTED;
#endif
    }

    return res;
}

int IOT_Linkkit_Report(int devid, iotx_linkkit_msg_type_t msg_type, unsigned char *payload, int payload_len)
{
    int res = FAIL_RETURN;
    char product_key[IOTX_PRODUCT_KEY_LEN] = { 0 };
    char device_name[IOTX_DEVICE_NAME_LEN] = { 0 };
#ifdef DEVICE_MODEL_GATEWAY    
    char device_secret[IOTX_DEVICE_SECRET_LEN] = { 0 };
#endif

    /* parameters check */
    if (devid < 0 || msg_type < 0 || msg_type >= IOTX_LINKKIT_MSG_MAX) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    /* get device meta info while it's a subdev */
    if (devid != ALINK_DEVICE_SELF_ID) {
#ifdef DEVICE_MODEL_GATEWAY
        (void)device_secret;
        
#else
        return IOTX_CODE_GATEWAY_UNSUPPORTED;
#endif
    }

    switch (msg_type) {
        case ITM_MSG_POST_PROPERTY: {
            if (payload == NULL || payload_len <= 0) {
                return IOTX_CODE_PARAMS_INVALID;
            }
            res = alink_upstream_thing_property_post_req(product_key, device_name, (char *)payload, payload_len);
        } break;

        case ITM_MSG_DEVICEINFO_UPDATE: {
            if (payload == NULL || payload_len <= 0) {
                return IOTX_CODE_PARAMS_INVALID;
            }
            res = alink_upstream_thing_deviceinfo_post_req(product_key, device_name, (char *)payload, payload_len);
        } break;

        case ITM_MSG_DEVICEINFO_GET: {
            res = alink_upstream_thing_deviceinfo_get_req(product_key, device_name);
        } break;

        case ITM_MSG_DEVICEINFO_DELETE: {
            if (payload == NULL || payload_len <= 0) {
                return IOTX_CODE_PARAMS_INVALID;
            }
            res = alink_upstream_thing_deviceinfo_delete_req(product_key, device_name, (char *)payload, payload_len);
        } break;

        case ITM_MSG_POST_RAW_DATA: {
            if (payload == NULL || payload_len <= 0) {
                return IOTX_CODE_PARAMS_INVALID;
            }
            res = alink_upstream_thing_raw_post_req(product_key, device_name, payload, payload_len);
        } break;

#ifdef DEVICE_MODEL_GATEWAY
        case ITM_MSG_LOGIN: {


        } break;
        case ITM_MSG_LOGOUT: {

        } break;
#endif
        default: {
            res = IOTX_CODE_UNKNOWN_MSG_TYPE;
        }break;
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
    int res = FAIL_RETURN;
    char product_key[IOTX_PRODUCT_KEY_LEN] = { 0 };
    char device_name[IOTX_DEVICE_NAME_LEN] = { 0 };

    /* parameters check */
    if (devid < 0 || eventid == NULL || payload == NULL || 0 == payload_len) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    if (devid != ALINK_DEVICE_SELF_ID) {
#ifdef DEVICE_MODEL_GATEWAY 


#else
        return IOTX_CODE_GATEWAY_UNSUPPORTED;
#endif
    }

    res = alink_upstream_thing_event_post_req(product_key, device_name, eventid, eventid_len, payload, payload_len);
    return res;
}

int IOT_RegisterCallback(iotx_linkkit_event_type_t event_id, linkkit_event_cb_t callback)
{
    return alink_set_event_callback(event_id, callback);
}



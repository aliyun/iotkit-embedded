/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "dev_reset_internal.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

static int g_dev_reset_sub_flag = 0;

int IOT_DevReset_Report(iotx_dev_meta_info_t *meta_info, iotx_mqtt_event_handle_func_fpt handle, void *extended)
{
    int res = 0;
    const char *reset_fmt = "/sys/%s/%s/thing/reset";
    const char *reset_reply_fmt = "/sys/%s/%s/thing/reset_reply";
    const char *payload_fmt = "{\"id\":%d, \"version\":\"1.0\", \"method\":\"thing.reset\", \"params\":{}}";
    char topic[IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 30] = {0};
    char payload[128] = {0};

    if (meta_info == NULL || handle== NULL ) {
        return FAIL_RETURN;
    }

    memset(topic, 0, IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 30);
    HAL_Snprintf(topic,IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 30, reset_reply_fmt, meta_info->product_key, meta_info->device_name);

    if (g_dev_reset_sub_flag == 0) {
        res = IOT_MQTT_Subscribe_Sync(NULL, topic, IOTX_MQTT_QOS0, handle, NULL, 5000);
        if (res < 0 ) {
            return FAIL_RETURN;
        }
        g_dev_reset_sub_flag = 1;
    }

    memset(topic, 0, IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 30);
    HAL_Snprintf(topic, IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 30, reset_fmt, meta_info->product_key, meta_info->device_name);

    memset(payload, 0, 128);
    HAL_Snprintf(payload, 128, payload_fmt, iotx_report_id());

    res = IOT_MQTT_Publish_Simple(NULL, topic, IOTX_MQTT_QOS0, payload, strlen(payload));

    return res;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif


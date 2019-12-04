/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "infra_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "wrappers.h"
#include "dev_sign_api.h"
#include "shadow_api.h"


#define SHADOW_MQTT_MSGLEN      (1024)

char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1MZxOdcBnO";
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "h4I4dneEFp7EImTv";
char g_device_name[IOTX_DEVICE_NAME_LEN + 1]       = "example1";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "jiPvZkVO9lhNj2Q9f2KdP4Yln7ACJI3X";

#define SHADOW_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

/**
 * @brief This is a callback function when a control value coming from server.
 *
 * @param [in] pattr: attribute structure pointer
 * @return none
 * @see none.
 * @note none.
 */
static void _device_shadow_cb_light(iotx_shadow_attr_pt pattr)
{

    /*
     * ****** Your Code ******
     */

    SHADOW_TRACE("----");
    SHADOW_TRACE("Attrbute Name: '%s'", pattr->pattr_name);
    SHADOW_TRACE("Attrbute Value: %d", *(int32_t *)pattr->pattr_data);
    SHADOW_TRACE("----");
}


/* Device shadow demo entry */
int demo_device_shadow(void)
{
    char buf[1024];
    void *h_shadow;
    iotx_mqtt_region_types_t region = IOTX_CLOUD_REGION_SHANGHAI;
    iotx_dev_meta_info_t meta;
    iotx_sign_mqtt_t sign_mqtt;
    iotx_shadow_para_t shadow_para;

    memset(&meta, 0, sizeof(iotx_dev_meta_info_t));
    memcpy(meta.product_key, g_product_key, strlen(g_product_key));
    memcpy(meta.product_secret, g_product_secret, strlen(g_product_secret));
    memcpy(meta.device_name, g_device_name, strlen(g_device_name));
    memcpy(meta.device_secret, g_device_secret, strlen(g_device_secret));

    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, g_product_key);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_NAME, g_device_name);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_SECRET, g_device_secret);

    if (IOT_Sign_MQTT(region, &meta, &sign_mqtt) < 0) {
        return -1;
    }

    /* Construct a device shadow */
    memset(&shadow_para, 0, sizeof(iotx_shadow_para_t));

    shadow_para.mqtt.port = sign_mqtt.port;
    shadow_para.mqtt.host = sign_mqtt.hostname;
    shadow_para.mqtt.client_id = sign_mqtt.clientid;
    shadow_para.mqtt.username = sign_mqtt.username;
    shadow_para.mqtt.password = sign_mqtt.password;

    shadow_para.mqtt.request_timeout_ms = 2000;
    shadow_para.mqtt.clean_session = 0;
    shadow_para.mqtt.keepalive_interval_ms = 60000;
    shadow_para.mqtt.read_buf_size = SHADOW_MQTT_MSGLEN;
    shadow_para.mqtt.write_buf_size = SHADOW_MQTT_MSGLEN;

    shadow_para.mqtt.handle_event.h_fp = NULL;
    shadow_para.mqtt.handle_event.pcontext = NULL;

    h_shadow = IOT_Shadow_Construct(&shadow_para);
    if (NULL == h_shadow) {
        SHADOW_TRACE("construct device shadow failed!");
        return -1;
    }


    /* Define and add two attribute */

    int32_t light = 1000, temperature = 1001;
    iotx_shadow_attr_t attr_light, attr_temperature;

    memset(&attr_light, 0, sizeof(iotx_shadow_attr_t));
    memset(&attr_temperature, 0, sizeof(iotx_shadow_attr_t));

    /* Initialize the @light attribute */
    attr_light.attr_type = IOTX_SHADOW_INT32;
    attr_light.mode = IOTX_SHADOW_RW;
    attr_light.pattr_name = "switch";
    attr_light.pattr_data = &light;
    attr_light.callback = _device_shadow_cb_light;

    /* Initialize the @temperature attribute */
    attr_temperature.attr_type = IOTX_SHADOW_INT32;
    attr_temperature.mode = IOTX_SHADOW_READONLY;
    attr_temperature.pattr_name = "temperature";
    attr_temperature.pattr_data = &temperature;
    attr_temperature.callback = NULL;


    /* Register the attribute */
    /* Note that you must register the attribute you want to synchronize with cloud
     * before calling IOT_Shadow_Pull() */
    IOT_Shadow_RegisterAttribute(h_shadow, &attr_light);
    IOT_Shadow_RegisterAttribute(h_shadow, &attr_temperature);


    /* synchronize the device shadow with device shadow cloud */
    IOT_Shadow_Pull(h_shadow);

    do {
        format_data_t format;

        /* Format the attribute data */
        IOT_Shadow_PushFormat_Init(h_shadow, &format, buf, 1024);
        IOT_Shadow_PushFormat_Add(h_shadow, &format, &attr_temperature);
        IOT_Shadow_PushFormat_Add(h_shadow, &format, &attr_light);
        IOT_Shadow_PushFormat_Finalize(h_shadow, &format);

        /* Update attribute data */
        IOT_Shadow_Push(h_shadow, format.buf, format.offset, 10);

        /* Sleep 1000 ms */
        HAL_SleepMs(1000);
    } while (0);


    /* Delete the two attributes */
    IOT_Shadow_DeleteAttribute(h_shadow, &attr_temperature);
    IOT_Shadow_DeleteAttribute(h_shadow, &attr_light);

    IOT_Shadow_Destroy(h_shadow);

    return 0;
}


int main()
{
    IOT_OpenLog("shadow");
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    /**< end*/

    demo_device_shadow();

    SHADOW_TRACE("out of demo!");
    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_CloseLog();

    return 0;
}


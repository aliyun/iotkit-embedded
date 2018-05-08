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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "lite-utils.h"

#include "iot_import.h"
#include "iot_export.h"
#include "iot_export_cm.h"

#ifdef ON_DAILY
#define IOTX_PRODUCT_KEY        "gsYfsxQJgeD"
#define IOTX_DEVICE_NAME        "DailyEnvDN"
#define IOTX_DEVICE_SECRET      "y1vzFkEgcuXnvkAfm627pwarx4HRNikX"
#define IOTX_PRODUCT_SECRET		""
#define IOTX_DEVICE_ID          "IoTxHttpTestDev_001"
#else
#define IOTX_PRODUCT_KEY        "yfTuLfBJTiL"
#define IOTX_DEVICE_NAME        "TestDeviceForDemo"
#define IOTX_DEVICE_SECRET      "fSCl9Ns5YPnYN8Ocg0VEel1kXFnRlV6c"
#define IOTX_PRODUCT_SECRET		""
#define IOTX_DEVICE_ID          "IoTxHttpTestDev_001"
#endif

    
/* These are pre-defined topics */
#define TOPIC_UPDATE            "/"IOTX_PRODUCT_KEY"/"IOTX_DEVICE_NAME"/update"
#define TOPIC_DATA              "/"IOTX_PRODUCT_KEY"/"IOTX_DEVICE_NAME"/data"


#define MQTT_MSGLEN             (1024)

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)


static void _register_func(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void *user_data)
{
    EXAMPLE_TRACE("source %s:%s\n", source->product_key, source->device_name);

    EXAMPLE_TRACE("type %d\n", msg->message_type);
    EXAMPLE_TRACE("URI %s\n", msg->URI);
    EXAMPLE_TRACE("code %d\n", msg->code);
    EXAMPLE_TRACE("id %d\n", msg->id);
    EXAMPLE_TRACE("method %s\n", msg->method);
    EXAMPLE_TRACE("parameter %s\n", (char*)msg->parameter);
}


static void _event_handle(void *pcontext, iotx_cm_event_msg_t* msg, void *user_data)
{
    EXAMPLE_TRACE("event %d\n", msg->event_id);

    if (IOTX_CM_EVENT_REGISTER_RESULT == msg->event_id) {
        iotx_cm_event_result_pt result = (iotx_cm_event_result_pt)msg->msg;

        EXAMPLE_TRACE("register result\n");
        EXAMPLE_TRACE("result %d\n", result->result);
        EXAMPLE_TRACE("URI %s\n", result->URI);
    } else if  (IOTX_CM_EVENT_UNREGISTER_RESULT == msg->event_id) {
        iotx_cm_event_result_pt result = (iotx_cm_event_result_pt)msg->msg;

        EXAMPLE_TRACE("unregister result\n");
        EXAMPLE_TRACE("result %d\n", result->result);
        EXAMPLE_TRACE("URI %s\n", result->URI);
    } else if (IOTX_CM_EVENT_NEW_DATA_RECEIVED == msg->event_id) {

        iotx_cm_new_data_t* new_data = (iotx_cm_new_data_t*)msg->msg;
        _register_func(new_data->peer, new_data->message_info, user_data);
    }
}


int cm_client()
{
    int rc = -1;
    int user_data = 10;
    iotx_cm_init_param_t param = {0};
    iotx_cm_register_param_t register_param = {0};
    iotx_cm_send_peer_t cloud_peer;
    iotx_cm_message_info_t message_info = {0};
    iotx_cm_unregister_param_t unregister_param = {0};
    param.domain_type = IOTX_CM_CLOUD_DOMAIN_ShangHai;

    param.event_func = _event_handle;
    param.user_data = &user_data;
#ifdef SUPPORT_PRODUCT_SECRET
    param.secret_type = IOTX_CM_DEVICE_SECRET_PRODUCT;
#else
    param.secret_type = IOTX_CM_DEVICE_SECRET_DEVICE;
#endif /* SUPPORT_PRODUCT_SECRET */

    EXAMPLE_TRACE("init\n");
    rc = IOT_CM_Init(&param, NULL);

    if (FAIL_RETURN == rc) {
        printf("init fail\n");
        IOT_CM_Deinit(NULL);
        return FAIL_RETURN;
    }

    EXAMPLE_TRACE("register\n");
    register_param.URI = TOPIC_DATA;
    register_param.message_type = IOTX_CM_MESSAGE_REQUEST;
    register_param.register_func = _register_func;
    register_param.user_data = &user_data;
    rc = IOT_CM_Register(&register_param, NULL);

    if (FAIL_RETURN == rc) {
        printf("register fail\n");
        IOT_CM_Deinit(NULL);
        return FAIL_RETURN;
    }

    EXAMPLE_TRACE("register success \n");

#ifndef CM_SUPPORT_MULTI_THREAD
    rc = IOT_CM_Yield(200, NULL);
    if (FAIL_RETURN == rc) {
        EXAMPLE_TRACE("yield fail\n");
        IOT_CM_Deinit(NULL);
        return FAIL_RETURN;
    }
#else
    HAL_SleepMs(2000);
#endif

    EXAMPLE_TRACE("send\n");
    memset(&cloud_peer, 0x0, sizeof(iotx_cm_send_peer_t));

    HAL_GetProductKey(cloud_peer.product_key);
    HAL_GetDeviceName(cloud_peer.device_name);

    message_info.id = 0;
    message_info.message_type = IOTX_CM_MESSAGE_REQUEST;
    message_info.URI = LITE_malloc(strlen(TOPIC_DATA) + 1);
    memset(message_info.URI, 0x0, strlen(TOPIC_DATA) + 1);
    strncpy(message_info.URI, TOPIC_DATA, strlen(TOPIC_DATA));
    message_info.method = LITE_malloc(strlen("thing.data") + 1);
    memset(message_info.method, 0x0, strlen("thing.data") + 1);
    strncpy(message_info.method, "thing.data", strlen("thing.data"));
    message_info.parameter = LITE_malloc(strlen("{hello world!}"));
    memset(message_info.parameter, 0x0, strlen("{hello world!}") + 1);
    strncpy(message_info.parameter, "{hello world!}", strlen("{hello world!}"));
    message_info.parameter_length = strlen(message_info.parameter);

    rc = IOT_CM_Send(&cloud_peer, &message_info, NULL);

    LITE_free(message_info.URI);
    LITE_free(message_info.method);
    LITE_free(message_info.parameter);

    if (FAIL_RETURN == rc) {
        EXAMPLE_TRACE("send fail\n");
        IOT_CM_Deinit(NULL);
        return FAIL_RETURN;
    }

#ifndef CM_SUPPORT_MULTI_THREAD
    rc = IOT_CM_Yield(200, NULL);
    if (FAIL_RETURN == rc) {
        EXAMPLE_TRACE("register fail\n");
        IOT_CM_Deinit(NULL);
        return FAIL_RETURN;
    }
#else
    HAL_SleepMs(2000);
#endif


    EXAMPLE_TRACE("unregister\n");
    unregister_param.URI = TOPIC_DATA;
    rc = IOT_CM_Unregister(&unregister_param, NULL);
    if (FAIL_RETURN == rc) {
        EXAMPLE_TRACE("unregister fail\n");
        IOT_CM_Deinit(NULL);
        return FAIL_RETURN;
    }


#ifndef CM_SUPPORT_MULTI_THREAD
    rc = IOT_CM_Yield(200, NULL);
    if (FAIL_RETURN == rc) {
        EXAMPLE_TRACE("register fail\n");
        IOT_CM_Deinit(NULL);
        return FAIL_RETURN;
    }
#else
    HAL_SleepMs(2000);
#endif

    IOT_CM_Deinit(NULL);

    return SUCCESS_RETURN;
}


int main(int argc, char **argv)
{
    IOT_OpenLog("cm\n");
    IOT_SetLogLevel(IOT_LOG_DEBUG);
    /**< set device info*/
    HAL_SetProductKey(IOTX_PRODUCT_KEY);
    HAL_SetDeviceName(IOTX_DEVICE_NAME);
#ifndef SUPPORT_PRODUCT_SECRET
    HAL_SetDeviceSecret(IOTX_DEVICE_SECRET);
#else
    HAL_SetProductSecret(IOTX_PRODUCT_SECRET);
#endif /**< SUPPORT_PRODUCT_SECRET*/
    /**< end*/
    EXAMPLE_TRACE("start!\n");

    cm_client();

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_CloseLog();

    EXAMPLE_TRACE("out of sample!\n");

    return 0;
}


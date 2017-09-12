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
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "iot_import.h"
#include "iot_export.h"

#if defined(TEST_HTTP_DAILY)

    /* Daily Environment */
    #define IOTX_PRODUCT_KEY         "ZG1EvTEa7NN"
    #define IOTX_DEVICE_NAME         "NlwaSPXsCpTQuh8FxBGH"
    #define IOTX_DEVICE_SECRET       "MRG3gZwzIdbrSuUpE8D4h9hkkCo7z4py"
    #define IOTX_DEVICE_ID           "mylight10000021503888519478"

#else
    /* Pre-Online or Online Environment */
    #define IOTX_PRODUCT_KEY         "sKfsu06Xvz4"
    #define IOTX_DEVICE_NAME         "pq9D2vUSFzVsYvZsoscZ"
    #define IOTX_DEVICE_SECRET       "6TNWzt2QBDaTqFm2Wi6Zr4scagmx6jJ7"
    #define IOTX_DEVICE_ID           "IoTxHttpTestDev_001"
#endif

static iotx_device_info_t deviceinfo;

static int iotx_post_data_to_server(void *param)
{
    char path[IOTX_URI_MAX_LEN + 1] = {0};
    char request_buf[1024];

    void *p_ctx = (void *)param;
    iotx_http_message_param_t msg_param;
    msg_param.request_payload = (char *)"{\"name\":\"hello world\"}";
    msg_param.response_payload = request_buf;
    msg_param.timeout_ms = 5000;
    msg_param.request_payload_len = strlen(msg_param.request_payload) + 1;
    msg_param.response_payload_len = 1024;
    msg_param.topic_path = path;

    HAL_Snprintf(msg_param.topic_path, IOTX_URI_MAX_LEN, "/topic/%s/%s/update", (char *)deviceinfo.product_key,
                 (char *)deviceinfo.device_name);
    if (0 == IOT_HTTP_SendMessage(p_ctx, &msg_param)) {
        HAL_Printf("message response is %s\r\n", msg_param.response_payload);
    } else {
        HAL_Printf("error\r\n");
    }

    return 0;
}


int main(int argc, char **argv)
{
    IOT_OpenLog("http");
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    memset(&deviceinfo, 0x00, sizeof(iotx_device_info_t));
    strncpy(deviceinfo.product_key,  IOTX_PRODUCT_KEY, IOTX_PRODUCT_KEY_LEN);
    strncpy(deviceinfo.device_secret, IOTX_DEVICE_SECRET, IOTX_DEVICE_SECRET_LEN);
    strncpy(deviceinfo.device_name,  IOTX_DEVICE_NAME, IOTX_DEVICE_NAME_LEN);
    strncpy(deviceinfo.device_id,  IOTX_DEVICE_ID, IOTX_DEVICE_ID_LEN);

    HAL_Printf("[HTTP-Client]: Enter HTTP Client\r\n");

    void *p_ctx = NULL;
    p_ctx = IOT_HTTP_Init(&deviceinfo);
    if (NULL != p_ctx) {
        IOT_HTTP_DeviceNameAuth(p_ctx);
        iotx_post_data_to_server(p_ctx);
        HAL_Printf("IoTx HTTP Message Sent\r\n");
    } else {
        HAL_Printf("IoTx HTTP init failed\r\n");
    }

    IOT_HTTP_DeInit();

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_CloseLog();

    return 0;
}


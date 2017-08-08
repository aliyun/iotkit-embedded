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
 */


#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#include "iot_import.h"
#include "iot_export.h"


#define IOTX_PRE_DTLS_SERVER_URI "coaps://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5684"
#define IOTX_PRE_NOSEC_SERVER_URI "coap://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5683"

char m_coap_client_running = 0;


static void coap_error_handler(unsigned error_code, void * p_message)
{
    printf("coap error code %d\r\n", error_code);
}

static void iotx_response_handler(void * arg, void * p_response)
{
    int            len       = 0;
    unsigned char *p_payload = NULL;
    iotx_coap_resp_code_t resp_code;
    IOT_CoAP_GetMessageCode(p_response, &resp_code);
    IOT_CoAP_GetMessagePayload(p_response, &p_payload, &len);
    printf("[APPL]: Message response code: %d\r\n", resp_code);
    printf("[APPL]: Len: %d, Payload: %s, \r\n", len, p_payload);
}


static void *iotx_post_data_2_server(void *param)
{
    char               uri[IOTX_URI_MAX_LEN+1] = {0};
    iotx_message_t     message;
    iotx_deviceinfo_t  devinfo;

    HAL_GetDeviceInfo(&devinfo);
    message.p_payload = "{\"name\":\"hello world\"}";
    message.payload_len = strlen("{\"name\":\"hello world\"}");
    message.resp_callback = iotx_response_handler;
    iotx_coap_context_t *p_ctx = (iotx_coap_context_t *)param;

    snprintf(uri, IOTX_URI_MAX_LEN, "/topic/%s/%s/update/", devinfo.product_key,
                                            devinfo.device_name);

    while(1){
        sleep(1);
        IOT_CoAP_SendMessage(p_ctx, uri, &message);
    }
}

static void *iotx_post_data_to_server(void *param)
{
    char               uri[IOTX_URI_MAX_LEN+1] = {0};
    iotx_message_t     message;
    iotx_deviceinfo_t  devinfo;
    message.p_payload = "{\"name\":\"hello world\"}";
    message.payload_len = strlen("{\"name\":\"hello world\"}");
    message.resp_callback = iotx_response_handler;
    message.msg_type = IOTX_MESSAGE_CON;
    iotx_coap_context_t *p_ctx = (iotx_coap_context_t *)param;

    HAL_GetDeviceInfo(&devinfo);
    snprintf(uri, IOTX_URI_MAX_LEN, "/topic/%s/%s/update/", devinfo.product_key,
                                            devinfo.device_name);

    IOT_CoAP_SendMessage(p_ctx, uri, &message);
}

int main(int argc, char **argv)
{
    int ret;
    int opt;
    int count = 0;
    char secur[32] = {0};

    printf("[COAP-Client]: Enter Coap Client\r\n");
    iotx_coap_config_t config;
    while ((opt = getopt(argc, argv, "s:l")) != -1){
        switch(opt){
            case 's':
                strncpy(secur, optarg, strlen(optarg));
                break;
            case 'l':
                m_coap_client_running = 1;
                break;
            default:
                break;
        }
    }

    if(0 == strncmp(secur, "dtls", strlen("dtls"))){
        config.p_uri = IOTX_PRE_DTLS_SERVER_URI;
    }
    else{
        config.p_uri = IOTX_PRE_NOSEC_SERVER_URI;
    }

    iotx_coap_context_t *p_ctx = NULL;
    p_ctx = IOT_CoAP_Init(&config);
    if(NULL != p_ctx){
        iotx_get_well_known(p_ctx);
        IOT_CoAP_DeviceNameAuth(p_ctx);
        do{
            iotx_post_data_to_server((void *)p_ctx);
            IOT_CoAP_Yield(p_ctx);
        }while(m_coap_client_running);

        IOT_CoAP_Deinit(&p_ctx);
    }
    else{
        printf("IoTx CoAP init failed\r\n");
    }

    return 0;
}


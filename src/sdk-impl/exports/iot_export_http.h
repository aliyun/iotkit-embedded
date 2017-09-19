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

#ifndef _IOT_EXPORT_HTTP_H_
#define _IOT_EXPORT_HTTP_H_

/* IoTx http context */
typedef struct {
    char               *p_auth_token;
    uint32_t           auth_token_len;
    char                is_authed;
    const char         *version;
    const char         *signmethod;
    const char         *sign;
    iotx_device_info_t *p_devinfo;
    const char         *timestamp;
} iotx_http_t, *iotx_http_pt;

/* IoTx http message definition 
 * request_payload and response_payload need to be allocate in order to save memory.
 * topic_path specify the topic url you want to publish message.
 */
typedef struct {
    char       *topic_path;
    uint32_t   request_payload_len;
    char       *request_payload;
    uint32_t   response_payload_len;
    char       *response_payload;
    uint32_t   timeout_ms;
} iotx_http_message_param_t;

/* The response code from sever */
typedef enum {
    IOTX_HTTP_SUCCESS = 0,
    IOTX_HTTP_COMMON_ERROR = 10000,
    IOTX_HTTP_PARAM_ERROR = 10001,
    IOTX_HTTP_AUTH_CHECK_ERROR = 20000,
    IOTX_HTTP_TOKEN_EXPIRED_ERROR = 20001,
    IOTX_HTTP_TOKEN_NULL_ERROR = 20002,
    IOTX_HTTP_TOKEN_CHECK_ERROR = 20003,
    IOTX_HTTP_UPDATE_SESSION_ERROR = 20004,
    IOTX_HTTP_PUBLISH_MESSAGE_ERROR = 30001,
    IOTX_HTTP_REQUEST_TOO_MANY_ERROR = 40000,
} iotx_http_upstream_response_t;

/**
 * @brief 	Initialize the HTTP client
 *        This function initialize the data.
 *
 * @param p_devinfo  Specify the device infomation.
 *
 * @return NULL, initialize failed; NOT NULL, the contex of HTTP client.
 */
void   *IOT_HTTP_Init(iotx_device_info_t *p_devinfo);

/**
 * @brief   De-initialize the HTTP client
 *        This function release the related resource.
 *
 * @param none
 *
 * @return void
 */
void    IOT_HTTP_DeInit(void);

/**
 * @brief   Handle device name authentication with remote server.
 *
 * @param p_context  Pointer of contex, specify the HTTP client.
 *
 * @return 0   Authenticate success.
 *        -1   Authenticate failed.
 */
int     IOT_HTTP_DeviceNameAuth(void *p_context);

/**
 * @brief   Send a message with specific path to server.
 *        Client must authentication with server before send message.
 *
 * @param p_context     Pointer of contex, specify the HTTP client.
 * @param msg_param     Specify the topic path and http payload configuration.
 *
 * @return -1 success
 *			0 failed
 */
int     IOT_HTTP_SendMessage(void *p_context, iotx_http_message_param_t *msg_param);

/*
TEST MACROS
    Daily Test Environment:         -DTEST_HTTP_DAILY
    Pre-Online Test Environment:    -DTEST_HTTP_PRE
*/

#endif

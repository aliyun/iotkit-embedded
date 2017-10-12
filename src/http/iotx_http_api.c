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

#include "iot_import.h"
#include "iot_export.h"

#include "lite-utils.h"
#include "utils_hmac.h"
#include "utils_httpc.h"
#include "ca.h"
#include "utils_epoch_time.h"
#include "sdk-impl_internal.h"
#include "device.h"
/*
#define IOTX_HTTP_TIMESTAMP_OPTIONAL_ENABLE
*/
#define IOTX_HTTP_SIGN_LENGTH       (41)
#define IOTX_HTTP_SIGN_SOURCE_LEN   (256)
#define IOTX_HTTP_AUTH_TOKEN_LEN    (192+1)
#define IOTX_HTTP_URL_LEN_MAX       (135)

#ifdef IOTX_HTTP_TIMESTAMP_OPTIONAL_ENABLE
#define IOTX_HTTP_SIGN_SRC_STR          "clientId%sdeviceName%sproductKey%stimestamp%s"
#define IOTX_HTTP_AUTH_DEVICENAME_STR   \
    "{" \
    "\"version\":\"%s\", \"clientId\":\"%s\"," \
    "\"signmethod\":\"%s\",\"sign\":\"%s\"," \
    "\"productKey\":\"%s\",\"deviceName\":\"%s\"," \
    "\"timestamp\":\"%s\"" \
    "}"
#else
#define IOTX_HTTP_SIGN_SRC_STR          "clientId%sdeviceName%sproductKey%s"
#define IOTX_HTTP_AUTH_DEVICENAME_STR   \
    "{" \
    "\"version\":\"%s\", \"clientId\":\"%s\"," \
    "\"signmethod\":\"%s\",\"sign\":\"%s\"," \
    "\"productKey\":\"%s\",\"deviceName\":\"%s\"" \
    "}"
#endif

#define IOTX_HTTP_AUTH_STR "auth"

#if defined(TEST_HTTP_DAILY)
    #define IOTX_HTTP_ONLINE_SERVER_URL     "http://10.101.83.159"
    #define IOTX_HTTP_ONLINE_SERVER_PORT    80
    #define IOTX_HTTP_CA_GET                NULL
#else
    #define IOTX_HTTP_ONLINE_SERVER_URL     "https://iot-as-http.cn-shanghai.aliyuncs.com"
    #define IOTX_HTTP_ONLINE_SERVER_PORT    443
    #define IOTX_HTTP_CA_GET                iotx_ca_get()
#endif

#define IOTX_SHA_METHOD                     "hmacsha1"
#define IOTX_MD5_METHOD                     "hmacmd5"

/* By default we use hmac-md5 algorithm for hmac in PK/DN/DS case */
#define USING_SHA1_IN_HMAC      (1)

#define IOTX_HTTP_HEADER_KEEPALIVE_STR  "Connection: Keep-Alive\r\n"
#define IOTX_HTTP_HEADER_PASSWORD_STR   "password:"
#define IOTX_HTTP_UPSTREAM_HEADER_STR   \
    IOTX_HTTP_HEADER_KEEPALIVE_STR \
    IOTX_HTTP_HEADER_PASSWORD_STR \
    "%s" \
    IOTX_HTTP_HEADER_END_STR
#define IOTX_HTTP_HEADER_END_STR "\r\n"

#define HTTP_AUTH_RESP_MAX_LEN   (256)

static iotx_http_t *p_iotx_http = NULL;

/*
  Http server url: https://iot-as-http.cn-shanghai.aliyuncs.com
  Only https protocal is supported at present.
  The /auth interface is used to get access token before any data transform which need to be requested just one time.
  The device must call /auth to get the token before transfer any data.
  Every data transfer need to be with access token.
  You need to request for a new token if the current token have been expired.
  Token can be cached in local devices, which has 48 hours term of validity.
  POST /auth HTTP/1.1
  Host: iot-as-http.cn-shanghai.aliyuncs.com
  Content-Type: application/json
  body: {"version":"default","clientId":"xxxxx","signmethod":"hmacsha1","sign":"xxxxxxxxxx","productKey":"xxxxxx","deviceName":"xxxxxxx","timestamp":"xxxxxxx"}
*/

static int iotx_calc_sign(const char *p_device_secret, const char *p_msg, char *sign)
{
#if USING_SHA1_IN_HMAC
    log_info("| method: %s", IOTX_SHA_METHOD);
    utils_hmac_sha1(p_msg, strlen(p_msg), sign, p_device_secret, strlen(p_device_secret));
#else
    log_info("| method: %s", IOTX_MD5_METHOD);
    utils_hmac_md5(p_msg, strlen(p_msg), sign, p_device_secret, strlen(p_device_secret));
#endif
    return IOTX_SUCCESS;
}

static int calc_snprintf_string_length(char *fmt, ...)
{
    va_list args;
    int     rc = 0;
    char *p = NULL;
    char *sval = NULL;

    if (NULL == fmt) {
        return -1;
    }

    va_start(args, fmt);

    for (p = fmt; *p; p++) {
        if (*p != '%') {
            rc++;
            continue;
        }
        switch (*++p) {
            case 's':
                for (sval = va_arg(args, char *); *sval; sval++) {
                    rc++;
                }
                break;
            default:
                rc++;
                break;
        }
    }

    va_end(args);

    return rc;
}

static int construct_full_http_authenticate_url(char *buf)
{
    LITE_snprintf(buf, IOTX_HTTP_URL_LEN_MAX,
                  "%s/%s", IOTX_HTTP_ONLINE_SERVER_URL, IOTX_HTTP_AUTH_STR);
    log_info("get_http_authenticate_url is %s", buf);
    return 0;
}

static int construct_full_http_upstream_url(char *buf, const char *topic_path)
{
    LITE_snprintf(buf, IOTX_HTTP_URL_LEN_MAX,
                  "%s%s", IOTX_HTTP_ONLINE_SERVER_URL, topic_path);
    log_info("construct_full_http_upstream_url is %s", buf);
    return 0;
}

void *IOT_HTTP_Init(iotx_device_info_t *p_devinfo)
{
    if (NULL == p_devinfo) {
        log_err("Invalid argument, p_dev_info = %p", p_devinfo);
        return NULL;
    }

    if (NULL == p_iotx_http) {
        p_iotx_http = LITE_malloc(sizeof(iotx_http_t));

        if (NULL == p_iotx_http) {
            log_err(" Allocate memory for p_iotx_http failed\r\n");
            return NULL;
        }
    } else {
        return p_iotx_http;
    }

    memset(p_iotx_http, 0x00, sizeof(iotx_http_t));

    p_iotx_http->p_auth_token = LITE_malloc(IOTX_HTTP_AUTH_TOKEN_LEN);
    if (NULL == p_iotx_http->p_auth_token) {
        log_err(" Allocate memory for auth token failed\r\n");
        goto err;
    }
    memset(p_iotx_http->p_auth_token, 0x00, IOTX_HTTP_AUTH_TOKEN_LEN);
    p_iotx_http->is_authed = 0;
    p_iotx_http->auth_token_len = IOTX_HTTP_AUTH_TOKEN_LEN;

    /*Get deivce information*/
    p_iotx_http->p_devinfo = LITE_malloc(sizeof(iotx_device_info_t));
    if (NULL == p_iotx_http->p_devinfo) {
        log_err(" Allocate memory for p_iotx_http->p_devinfo failed\r\n");
        goto err;
    }
    memset(p_iotx_http->p_devinfo, 0x00, sizeof(iotx_device_info_t));

    /*It should be implement by the user*/
    memset(p_iotx_http->p_devinfo, 0x00, sizeof(iotx_device_info_t));
    strncpy(p_iotx_http->p_devinfo->device_id,    p_devinfo->device_id,   IOTX_DEVICE_ID_LEN);
    strncpy(p_iotx_http->p_devinfo->product_key,  p_devinfo->product_key, IOTX_PRODUCT_KEY_LEN);
    strncpy(p_iotx_http->p_devinfo->device_secret, p_devinfo->device_secret, IOTX_DEVICE_SECRET_LEN);
    strncpy(p_iotx_http->p_devinfo->device_name,  p_devinfo->device_name, IOTX_DEVICE_NAME_LEN);

    iotx_device_info_init();
    iotx_device_info_set(p_iotx_http->p_devinfo->product_key, p_iotx_http->p_devinfo->device_name,
                         p_iotx_http->p_devinfo->device_secret);

    return (void *)p_iotx_http;

err:
    /* Error, release the memory */
    if (NULL != p_iotx_http) {
        if (NULL != p_iotx_http->p_devinfo) {
            LITE_free(p_iotx_http->p_devinfo);
        }
        if (NULL != p_iotx_http->p_auth_token) {
            LITE_free(p_iotx_http->p_auth_token);
        }

        p_iotx_http->auth_token_len = 0;
        LITE_free(p_iotx_http);
    }
    return NULL;
}

void IOT_HTTP_DeInit()
{
    if (NULL != p_iotx_http) {
        if (NULL != p_iotx_http->p_devinfo) {
            LITE_free(p_iotx_http->p_devinfo);
        }
        if (NULL != p_iotx_http->p_auth_token) {
            LITE_free(p_iotx_http->p_auth_token);
        }

        p_iotx_http->auth_token_len = 0;
        LITE_free(p_iotx_http);
    }
}


int IOT_HTTP_DeviceNameAuth(void *p_context)
{
    int                 ret = -1;
    int                 ret_code = 0;
    char               *pvalue = NULL;
    char               *response_message = NULL;
    char                sign[IOTX_HTTP_SIGN_LENGTH]   = {0};
    iotx_http_t        *p_iotx_http = NULL;
    char                http_url[IOTX_HTTP_URL_LEN_MAX] = {0};
    char                timestamp[14] = {0};
    httpclient_t        httpc;
    httpclient_data_t   httpc_data;
    char               *requ_payload = NULL;
    char               *resp_payload = NULL;
    int                 len = 0;
    char                p_msg_unsign[IOTX_HTTP_SIGN_SOURCE_LEN] = {0};
    /*
        //    body:
        //    {
        //      "version": "default",//默认default
        //      "clientId": "xxxxxx",//必填
        //      "signmethod": "hmacsha1",//只支持hmacmd5和hmacsha1，默认hmacmd5
        //      "sign": "xxxxxxxxxxxxx",//必填，signmethod(deviceSecret,content), content = 将所有提交给服务器的参数（version,sign,signmethod除外）, 按照字母顺序排序, 然后将参数值依次拼接，无拼接符号
        //      "productKey": "xxxxxx",//必填
        //      "deviceName": "xxxxxx",//必填
        //      "timestamp": "xxxxxx"//选填  13byte
        //    }
    */
    if (NULL == p_context) {
        log_err("p_context parameter is Null,failed!");
        goto do_exit;
    }
    p_iotx_http = (iotx_http_t *)p_context;
    p_iotx_http->is_authed = 0;

    /* FIXME:some compile error when calling this function. Get TimeStamp */
    /*
    if(!utils_get_epoch_time_from_ntp(timestamp, sizeof(timestamp)))
    {
            log_info("http time response: \r\n\r\n%s\r\n", timestamp);
            goto do_exit;
    }
    */

    /* Calculate Sign */
    memset(sign,  0x00, IOTX_HTTP_SIGN_LENGTH);
    memset(p_msg_unsign, 0x00, IOTX_HTTP_SIGN_SOURCE_LEN);

    LITE_snprintf(p_msg_unsign, IOTX_HTTP_SIGN_SOURCE_LEN,
                  IOTX_HTTP_SIGN_SRC_STR
                  , p_iotx_http->p_devinfo->device_id
                  , p_iotx_http->p_devinfo->device_name
                  , p_iotx_http->p_devinfo->product_key
#ifdef IOTX_HTTP_TIMESTAMP_OPTIONAL_ENABLE
                  , timestamp
#endif
                 );

    iotx_calc_sign(p_iotx_http->p_devinfo->device_secret, p_msg_unsign, sign);

    /* to save stack memory*/
    len = calc_snprintf_string_length(IOTX_HTTP_AUTH_DEVICENAME_STR,
                                      "default",
                                      p_iotx_http->p_devinfo->device_id,
#if USING_SHA1_IN_HMAC
                                      IOTX_SHA_METHOD,
#else
                                      IOTX_MD5_METHOD,
#endif
                                      sign,
                                      p_iotx_http->p_devinfo->product_key,
                                      p_iotx_http->p_devinfo->device_name,
                                      timestamp
                                     );

    if (len < 0) {
        goto do_exit;
    }

    requ_payload = (char *)LITE_malloc(len + 1);
    memset(requ_payload, 0, len + 1);

    log_debug("len = %d,requ_payload: \r\n%s\r\n", len, requ_payload);

    len = LITE_snprintf(requ_payload, len + 1,
                        IOTX_HTTP_AUTH_DEVICENAME_STR,
                        "default",
                        p_iotx_http->p_devinfo->device_id,
#if USING_SHA1_IN_HMAC
                        IOTX_SHA_METHOD,
#else
                        IOTX_MD5_METHOD,
#endif
                        sign,
                        p_iotx_http->p_devinfo->product_key,
                        p_iotx_http->p_devinfo->device_name,
                        timestamp
                       );
    log_debug("len = %d,requ_payload: \r\n%s\r\n", len, requ_payload);

    /* Malloc Http Response Payload */
    resp_payload = (char *)LITE_malloc(HTTP_AUTH_RESP_MAX_LEN);
    if (NULL == resp_payload) {
        log_err("Allocate HTTP resp_payload buf failed!");
        goto do_exit;
    }
    memset(resp_payload, 0, HTTP_AUTH_RESP_MAX_LEN);

    /* Construct Auth Url */
    construct_full_http_authenticate_url(http_url);

    /* Set httpclient and httpclient_data */
    memset(&httpc, 0, sizeof(httpclient_t));
    memset(&httpc_data, 0, sizeof(httpclient_data_t));

    httpc_data.post_content_type = "application/json";
    httpc_data.post_buf = requ_payload;
    httpc_data.post_buf_len = strlen(requ_payload);
    httpc_data.response_buf = resp_payload;
    httpc_data.response_buf_len = HTTP_AUTH_RESP_MAX_LEN;

    httpc.header = "Connection: Keep-Alive\r\n";

    /*
    Test Code
    p_iotx_http->p_auth_token = "eyJ0eXBlIjoiSldUIiwiYWxnIjoiaG1hY3NoYTEifQ.eyJleHBpcmUiOjE1MDQ3ODE4MzQ5MDAsInRva2VuIjoiM2EyZTRmYzMyNjk5NDE0Y2E3MDFjNzIzNzI1YjIyNDgifQ.e87AFhkvNKiqF5xdgm1P47f9DwY";
    p_iotx_http->is_authed = 1;
    ret = 0;
    goto do_exit;
    Test Code
    */

    /* Send Request and Get Response */
    if (0 != iotx_post(&httpc,
                       http_url,
                       IOTX_HTTP_ONLINE_SERVER_PORT,
                       IOTX_HTTP_CA_GET,
                       5000,
                       &httpc_data)) {
        goto do_exit;
    }

    /*
    body:
    {
      "code": 0,//业务状态码
      "message": "success",//业务信息
      "info": {
        "token": "eyJ0eXBlIjoiSldUIiwiYWxnIjoiaG1hY3NoYTEifQ.eyJleHBpcmUiOjE1MDI1MzE1MDc0NzcsInRva2VuIjoiODA0ZmFjYTBiZTE3NGUxNjliZjY0ODVlNWNiNDg3MTkifQ.OjMwu29F0CY2YR_6oOyiOLXz0c8"
      }
    }
    */
    log_info("http response: \r\n\r\n%s\r\n", httpc_data.response_buf);

    pvalue = LITE_json_value_of("code", httpc_data.response_buf);
    if (!pvalue) {
        goto do_exit;
    }
    ret_code = atoi(pvalue);
    log_info("ret_code = %d", ret_code);
    LITE_free(pvalue);
    pvalue = NULL;

    pvalue = LITE_json_value_of("message", httpc_data.response_buf);
    if (NULL == pvalue) {
        goto do_exit;
    }
    response_message = pvalue;
    log_info("response_message: %s", response_message);
    LITE_free(pvalue);
    pvalue = NULL;

    switch (ret_code) {
        case IOTX_HTTP_SUCCESS:
            break;
        case IOTX_HTTP_COMMON_ERROR:
        case IOTX_HTTP_PARAM_ERROR:
        case IOTX_HTTP_AUTH_CHECK_ERROR:
        case IOTX_HTTP_UPDATE_SESSION_ERROR:
        case IOTX_HTTP_REQUEST_TOO_MANY_ERROR:
        default:
            goto do_exit;
    }

    pvalue = LITE_json_value_of("info.token", httpc_data.response_buf);
    if (NULL == pvalue) {
        log_err("can't get token from json, Abort!\r\n");
        goto do_exit;
    }

    strcpy(p_iotx_http->p_auth_token, pvalue);
    p_iotx_http->is_authed = 1;
    LITE_free(pvalue);
    pvalue = NULL;

    log_info("iotToken: %s", p_iotx_http->p_auth_token);
    ret = 0;

do_exit:
    if (pvalue) {
        LITE_free(pvalue);
        pvalue = NULL;
    }
    if (requ_payload) {
        LITE_free(requ_payload);
        requ_payload = NULL;
    }
    if (resp_payload) {
        LITE_free(resp_payload);
        resp_payload = NULL;
    }

    return ret;
}

int IOT_HTTP_SendMessage(void *p_context, iotx_http_message_param_t *msg_param)
{
    int                 ret = -1;
    int                 response_code = 0;
    char               *pvalue = NULL;
    iotx_http_t        *p_iotx_http = NULL;
    char                http_url[IOTX_HTTP_URL_LEN_MAX] = {0};
    httpclient_t        httpc;
    httpclient_data_t   httpc_data;
    char               *messageId = NULL;
    char               *user_data = NULL;
    char               *response_message = NULL;
    int len = 0;
    /*
        POST /topic/${topic} HTTP/1.1
        Host: iot-as-http.cn-shanghai.aliyuncs.com
        password:${token}
        Content-Type: application/octet-stream
        body: ${your_data}
    */

    /* Set httpclient and httpclient_data */
    memset(&httpc, 0, sizeof(httpclient_t));
    memset(&httpc_data, 0, sizeof(httpclient_data_t));

    if (NULL == p_context) {
        log_err("p_context parameter is Null,failed!");
        goto do_exit;
    }
    p_iotx_http = (iotx_http_t *)p_context;


    if (NULL == msg_param) {
        log_err("msg_param parameter is Null,failed!");
        goto do_exit;
    }

    if (0 == p_iotx_http->is_authed) {
        log_err("Device is not authed");
        /* IOT_HTTP_DeviceNameAuth(p_iotx_http); */
        goto do_exit;
    }

    if (NULL == msg_param->request_payload) {
        log_err("IOT_HTTP_SendMessage request_payload NULL!");
        goto do_exit;
    }

    if (NULL == msg_param->response_payload) {
        log_err("IOT_HTTP_SendMessage response_payload NULL!");
        goto do_exit;
    }

    if (NULL == msg_param->topic_path) {
        log_err("IOT_HTTP_SendMessage topic_path NULL!");
        goto do_exit;
    }

    /* Construct Auth Url */
    construct_full_http_upstream_url(http_url, msg_param->topic_path);

    len = strlen(IOTX_HTTP_HEADER_PASSWORD_STR) + strlen(p_iotx_http->p_auth_token) + strlen(
                      IOTX_HTTP_HEADER_KEEPALIVE_STR) + strlen(IOTX_HTTP_HEADER_END_STR);
    httpc.header = LITE_malloc(len + 1);
    LITE_snprintf(httpc.header, len + 1,
                  IOTX_HTTP_UPSTREAM_HEADER_STR, p_iotx_http->p_auth_token);
    log_info("httpc.header = %s", httpc.header);

    httpc_data.post_content_type = "application/octet-stream";
    httpc_data.post_buf = msg_param->request_payload;
    httpc_data.post_buf_len = msg_param->request_payload_len;
    httpc_data.response_buf = msg_param->response_payload;
    httpc_data.response_buf_len = msg_param->response_payload_len;

    log_info("request_payload: \r\n\r\n%s\r\n", httpc_data.post_buf);

    /* Send Request and Get Response */
    if (iotx_post(&httpc,
                  http_url,
                  IOTX_HTTP_ONLINE_SERVER_PORT,
                  IOTX_HTTP_CA_GET,
                  msg_param->timeout_ms,
                  &httpc_data)) {
        goto do_exit;
    }

    /*
        body:
        {
          "code": 0,
          "message": "success",
          "info": {
            "messageId": 892687627916247040,
            "data": byte[]//may be NULL
          }
        }
    */
    log_info("http response: \r\n\r\n%s\r\n", httpc_data.response_buf);

    pvalue = LITE_json_value_of("code", httpc_data.response_buf);
    if (!pvalue) {
        goto do_exit;
    }

    response_code = atoi(pvalue);
    LITE_free(pvalue);
    pvalue = NULL;
    log_info("response code: %d", response_code);

    pvalue = LITE_json_value_of("message", httpc_data.response_buf);
    if (NULL == pvalue) {
        goto do_exit;
    }
    response_message = LITE_strdup(pvalue);
    log_info("response_message: %s", response_message);
    LITE_free(pvalue);
    pvalue = NULL;

    switch (response_code) {
        case IOTX_HTTP_SUCCESS:
            break;
        case IOTX_HTTP_TOKEN_EXPIRED_ERROR:
            p_iotx_http->is_authed = false;
            IOT_HTTP_DeviceNameAuth((iotx_http_t *)p_iotx_http);
        case IOTX_HTTP_COMMON_ERROR:
        case IOTX_HTTP_PARAM_ERROR:
        case IOTX_HTTP_AUTH_CHECK_ERROR:
        case IOTX_HTTP_TOKEN_NULL_ERROR:
        case IOTX_HTTP_TOKEN_CHECK_ERROR:
        case IOTX_HTTP_UPDATE_SESSION_ERROR:
        case IOTX_HTTP_PUBLISH_MESSAGE_ERROR:
        case IOTX_HTTP_REQUEST_TOO_MANY_ERROR:
        default:
            goto do_exit;
    }

    /* info.messageId */
    pvalue = LITE_json_value_of("info.messageId", httpc_data.response_buf);
    if (NULL == pvalue) {
        log_err("messageId: NULL");
        goto do_exit;
    }
    messageId = pvalue;
    log_info("messageId: %s", messageId);
    LITE_free(pvalue);
    pvalue = NULL;

    /* info.data */
    pvalue = LITE_json_value_of("info.data", httpc_data.response_buf);
    user_data = pvalue;

    /* Maybe NULL */
    if (user_data) {
        log_info("user_data: %s", user_data);
    } else {
        log_info("user_data: %p", user_data);
    }
    if (NULL != pvalue) {
        LITE_free(pvalue);
    }
    pvalue = NULL;

    ret = 0;

do_exit:
    if (pvalue) {
        LITE_free(pvalue);
    }

    if (response_message) {
        LITE_free(response_message);
    }

    if (httpc.header) {
        LITE_free(httpc.header);
    }

    return ret;
}



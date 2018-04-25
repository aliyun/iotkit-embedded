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

#include "iotx_cmp_common.h"
#include "iotx_cmp_mqtt_direct.h"
#include "iotx_cmp_cloud_conn.h"
#include "utils_httpc.h"
#include "utils_hmac.h"
#include "iot_export.h"
#include "iot_import.h"

#define IOTX_CMP_MESSAGE_ID_MAX     (65535)


#ifdef CMP_MAPPING_USE_POOL
static iotx_cmp_mapping_t g_cmp_mapping_list[CMP_SUPPORT_MAX_MAPPING_SIZE];
#endif

#ifdef CMP_PROCESS_NODE_USE_POOL
static iotx_cmp_process_list_node_t g_cmp_cloud_process_node_list[CMP_SUPPORT_MAX_PROCESS_NODE_SIZE];
#endif

#ifdef CMP_SUPPORT_LOCAL_CONN
#ifdef CMP_PROCESS_NODE_USE_POOL
static iotx_cmp_process_list_node_t g_cmp_local_process_node_list[CMP_SUPPORT_MAX_PROCESS_NODE_SIZE];
#endif
#endif


static const char string_id[] CMP_READ_ONLY = "id";
static const char string_code[] CMP_READ_ONLY = "code";
static const char string_data[] CMP_READ_ONLY = "data";
static const char string_params[] CMP_READ_ONLY = "params";
static const char string_method[] CMP_READ_ONLY = "method";
static const char string_pk[] CMP_READ_ONLY = "data.productKey";
static const char string_dn[] CMP_READ_ONLY = "data.deviceName";
static const char string_ds[] CMP_READ_ONLY = "data.deviceSecret";
static const char string_response_format[] CMP_READ_ONLY = "{\"id\":\"%u\",\"code\":%d,\"data\":%s}";
static const char string_request_format[] CMP_READ_ONLY = "{\"id\":\"%u\",\"version\":\"1.0\",\"params\":%s,\"method\":\"%s\"}";
static const char string_SYS_URI[] CMP_READ_ONLY = "/sys/%s/%s/";
static const char string_EXT_URI[] CMP_READ_ONLY = "/ext/%s/%s/";
static const char string_SYS_URI_1[] CMP_READ_ONLY = "/sys/%s/%s/%s";
static const char string_EXT_URI_1[] CMP_READ_ONLY = "/ext/%s/%s/%s";
static const char string_SHA_METHOD[] CMP_READ_ONLY = "hmacsha1";
static const char string_MD5_METHOD[] CMP_READ_ONLY = "hmacmd5";
static const char string_TIMESTAMP[] CMP_READ_ONLY = "2524608000000";
static const char string_AUTH_URL[] CMP_READ_ONLY = "https://iot-auth.cn-shanghai.aliyuncs.com/auth/register/device";
static const char string_AUTH_CONTENT_TYPE[] CMP_READ_ONLY = "application/x-www-form-urlencoded";
static const char string_hmac_format[] CMP_READ_ONLY = "deviceName%s" "productKey%s" "random%s";
static const char string_auth_req_format[] CMP_READ_ONLY = "productKey=%s&" "deviceName=%s&" "signMethod=%s&" "sign=%s&" "version=default&" "clientId=%s&" "random=%s&" "resources=mqtt";
static const int  int_random_length = 15;

static char* genRandomString(int length)
{
    int flag, i;
    char* str;
    if ((str = (char*) LITE_malloc(length)) == NULL )
    {
    	CMP_ERR(cmp_log_error_memory);
        return NULL;
    }

    for (i = 0; i < length - 1; i++)
    {
        flag = rand() % 3;
        switch (flag)
        {
            case 0:
                str[i] = 'A' + HAL_Random(26);
                break;
            case 1:
                str[i] = 'a' + HAL_Random(26);
                break;
            case 2:
                str[i] = '0' + HAL_Random(10);
                break;
            default:
                str[i] = 'x';
                break;
        }
    }
    str[length - 1] = '\0';
    return str;
}
static int iotx_cmp_get_next_message_id(iotx_cmp_conntext_pt cmp_pt)
{
    if (cmp_pt->cmp_message_id == IOTX_CMP_MESSAGE_ID_MAX)
        cmp_pt->cmp_message_id = 1;

    cmp_pt->cmp_message_id++;

    return cmp_pt->cmp_message_id;
}

#if 1
int _http_response(char *payload,
                   const int payload_len,
                   const char *request_string,
                   const char *url,
                   const int port_num,
                   const char *pkey
                  )
{
#define HTTP_POST_MAX_LEN   (1024)
#define HTTP_RESP_MAX_LEN   (1024)

    int ret = SUCCESS_RETURN;
    char *requ_payload = NULL;
    char *resp_payload = NULL;
    httpclient_t httpc;
    httpclient_data_t httpc_data;

    memset(&httpc, 0, sizeof(httpclient_t));
    memset(&httpc_data, 0, sizeof(httpclient_data_t));

    /* todo */
    httpc.header = "Accept: text/xml,text/javascript,text/html,application/json\r\n";

    requ_payload = (char *)CMP_malloc(HTTP_POST_MAX_LEN);
    if (NULL == requ_payload) {
        CMP_ERR(cmp_log_error_memory);
        return ERROR_MALLOC;
    }
    memset(requ_payload, 0, HTTP_POST_MAX_LEN);

    HAL_Snprintf(requ_payload,
                 HTTP_POST_MAX_LEN,
                 "%s",
                 request_string);
    CMP_INFO(cmp_log_info_auth_payload_req, payload);

    resp_payload = (char *)LITE_malloc(HTTP_RESP_MAX_LEN);
    if (!resp_payload) {
        ret = FAIL_RETURN;
        goto RETURN;
    }
    LITE_ASSERT(resp_payload);
    memset(resp_payload, 0, HTTP_RESP_MAX_LEN);

    httpc_data.post_content_type = (char*)string_AUTH_CONTENT_TYPE;
    httpc_data.post_buf = requ_payload;
    httpc_data.post_buf_len = strlen(requ_payload);
    httpc_data.response_buf = resp_payload;
    httpc_data.response_buf_len = HTTP_RESP_MAX_LEN;

    ret = httpclient_common(&httpc,
                            url,
                            port_num,
                            pkey,
                            HTTPCLIENT_POST,
                            CONFIG_GUIDER_AUTH_TIMEOUT,
                            &httpc_data);
    if (ret != 0) {
        ret = FAIL_RETURN;
        goto RETURN;
    }

    memcpy(payload, httpc_data.response_buf, payload_len);
    CMP_INFO(cmp_log_info_auth_payload_rsp, payload);

RETURN:
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


static char *_set_auth_req_str(const char *product_key, const char *device_name, const char *client_id, const char *sign, const char *ts)
{
#define AUTH_STRING_MAXLEN  (1024)

    char *req = NULL;

    req = CMP_malloc(AUTH_STRING_MAXLEN);
    memset(req, 0, AUTH_STRING_MAXLEN);

    HAL_Snprintf(req
                 , AUTH_STRING_MAXLEN
                 , string_auth_req_format
                 , product_key
                 , device_name
#ifdef USING_SHA1_IN_HMAC
                 , string_SHA_METHOD
#else
                 , string_MD5_METHOD
#endif
                 , sign
                 , client_id
                 , ts);
    return req;
}

static int _get_device_secret(const char *product_key, const char *device_name, const char *client_id, const char *guider_addr, const char *request_string)
{
    char payload[512] = {0};
    int ret = -1;
    int ret_code = 0;
    const char *pvalue;

    /*
        {
            "code" : 200,
            "data" : {
                "productKey" : "42Ze0mk3556498a1AlTP",
                "deviceName" : "0d7fdeb9dc1f4344a2cc0d45edcb0bcb",
                "deviceSecret" : "adsfweafdsf"
        }
    */
    strncpy(payload,request_string,strlen(request_string));
    _http_response(payload,
                   sizeof(payload),
                   request_string,
                   guider_addr,
                   443,
                   iotx_ca_get()
                  );
    CMP_INFO(cmp_log_info_auth_rsp, payload);

    pvalue = LITE_json_value_of((char*)string_code, payload);
    if (!pvalue) {
        goto do_exit;
    }

    ret_code = atoi(pvalue);
    LITE_free(pvalue);
    pvalue = NULL;

    if (200 != ret_code) {
        log_err(cmp_log_error_ret_code, ret_code);
        goto do_exit;
    }

    pvalue = LITE_json_value_of((char*)string_pk, payload);
    if (NULL == pvalue) {
        goto do_exit;
    }
    if (0 != strncmp(pvalue, product_key, strlen(product_key))) {
        LITE_free(pvalue);
        goto do_exit;
    }
    LITE_free(pvalue);
    pvalue = NULL;

    pvalue = LITE_json_value_of((char*)string_dn, payload);
    if (NULL == pvalue) {
        goto do_exit;
    }
    if (0 != strncmp(pvalue, device_name, strlen(device_name))) {
        LITE_free(pvalue);
        goto do_exit;
    }
    LITE_free(pvalue);
    pvalue = NULL;

    pvalue = LITE_json_value_of((char*)string_ds, payload);
    if (NULL == pvalue) {
        goto do_exit;
    }
    HAL_SetDeviceSecret((char*)pvalue);
    LITE_free(pvalue);
    pvalue = NULL;

    ret = 0;

do_exit:
    if (pvalue) {
        LITE_free(pvalue);
        pvalue = NULL;
    }

    return ret;
}

static int _calc_hmac_signature(
        const char *product_key,
        const char *device_name,
        char *hmac_sigbuf,
        const int hmac_buflen,
        const char *random)
{
    char signature[64];
    char hmac_source[512];
    int ret = FAIL_RETURN;
    char product_secret[PRODUCT_SECRET_LEN + 1] = {0};

    memset(signature, 0, sizeof(signature));
    memset(hmac_source, 0, sizeof(hmac_source));
    
    HAL_GetProductSecret(product_secret);

    ret = HAL_Snprintf(hmac_source,
                      sizeof(hmac_source),
                      string_hmac_format,
                      device_name,
                      product_key,
                      random);

#ifdef USING_SHA1_IN_HMAC
    utils_hmac_sha1(hmac_source, strlen(hmac_source),
                    signature,
                    product_secret,
                    strlen(product_secret));
#else
    utils_hmac_md5(hmac_source, strlen(hmac_source),
                   signature,
                   product_secret,
                   strlen(product_secret));
#endif

    memset(hmac_sigbuf, 0x0, hmac_buflen);
    memcpy(hmac_sigbuf, signature, hmac_buflen);
	
    return ret;
}
#endif


int iotx_cmp_auth(const char *product_key, const char *device_name, const char *client_id)
{
    char *req_str = NULL;
    char guider_sign[40+1] = {0};
    char *s_random = NULL;
    s_random = genRandomString(int_random_length);
    // todo   string_TIMESTAMP -> random
    _calc_hmac_signature(product_key, device_name, guider_sign, sizeof(guider_sign), s_random);
    req_str = _set_auth_req_str(product_key, device_name, client_id, guider_sign, s_random);
    CMP_INFO(cmp_log_info_auth_req, req_str);

    if (SUCCESS_RETURN != _get_device_secret(product_key, device_name, client_id, string_AUTH_URL, req_str)) {
        if (req_str) LITE_free(req_str);
        if (s_random) LITE_free(s_random);
        CMP_ERR(cmp_log_error_auth);
        return FAIL_RETURN;
    }
	if (req_str) LITE_free(req_str);
    if (s_random) LITE_free(s_random);
    return SUCCESS_RETURN;
}


#ifdef CMP_SUPPORT_TOPIC_DISPATCH
int iotx_cmp_parse_payload(void* payload,
                    int payload_length,
                    iotx_cmp_message_info_pt msg)
{
    char* payload_pt = (char*)payload;
    char* node = NULL;

    if (NULL == payload || NULL == msg || 0 == payload_length) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    switch (msg->message_type) {
        case IOTX_CMP_MESSAGE_RAW: {
            CMP_INFO(cmp_log_info_raw_data);
            msg->code = 0;
            msg->parameter = CMP_malloc(payload_length + 1);
            if (NULL == msg->parameter) {
                CMP_ERR(cmp_log_error_memory);
                LITE_free(node);
                return FAIL_RETURN;
            }
            memset(msg->parameter, 0x0, payload_length);
            memcpy(msg->parameter, payload, payload_length);
            msg->parameter_length = payload_length;
            msg->method = NULL;
            msg->id = 0;
            return SUCCESS_RETURN;
        }

        case IOTX_CMP_MESSAGE_RESPONSE: {
            node = LITE_json_value_of((char*)string_id, payload_pt);
            if (node == NULL) {
                CMP_ERR(cmp_log_error_parse_id);
                return FAIL_RETURN;
            }

            msg->id = atoi(node);
            LITE_free(node);
            node = NULL;

            /* parse code */
            node = LITE_json_value_of((char*)string_code, payload_pt);
            if (node == NULL) {
                CMP_ERR(cmp_log_error_parse_code);
                return FAIL_RETURN;
            }
            msg->code = atoi(node);
            LITE_free(node);
            node = NULL;

            /* parse data */
            node = LITE_json_value_of((char*)string_data, payload_pt);
            if (node == NULL) {
                CMP_ERR(cmp_log_error_parse_data);
                return FAIL_RETURN;
            }
            msg->parameter = CMP_malloc(strlen(node) + 1);
            if (NULL == msg->parameter) {
                CMP_ERR(cmp_log_error_memory);
                LITE_free(node);
                return FAIL_RETURN;
            }
            memset(msg->parameter, 0x0, strlen(node));
            strncpy(msg->parameter, node, strlen(node));
            msg->parameter_length = strlen(msg->parameter);
            LITE_free(node);
            node = NULL;

            msg->method = NULL;
            return SUCCESS_RETURN;
        }

        case IOTX_CMP_MESSAGE_REQUEST: {
            node = LITE_json_value_of((char*)string_id, payload_pt);
            if (node == NULL) {
                CMP_ERR(cmp_log_error_parse_id);
                return FAIL_RETURN;
            }

            msg->id = atoi(node);
            LITE_free(node);
            node = NULL;

            /* parse params */
            node = LITE_json_value_of((char*)string_params, payload_pt);
            if (node == NULL) {
                CMP_ERR(cmp_log_error_parse_params);
                return FAIL_RETURN;
            }

            msg->parameter = CMP_malloc(strlen(node) + 1);
            if (NULL == msg->parameter) {
                CMP_ERR(cmp_log_error_memory);
                LITE_free(node);
                return FAIL_RETURN;
            }
            memset(msg->parameter, 0x0, strlen(node));
            strncpy(msg->parameter, node, strlen(node));
            msg->parameter_length = strlen(msg->parameter);
            LITE_free(node);
            node = NULL;

            /* parse method */
            node = LITE_json_value_of((char*)string_method, payload_pt);
            if (node == NULL) {
                CMP_ERR(cmp_log_error_parse_method);
                return FAIL_RETURN;
            }
            msg->method  = CMP_malloc(strlen(node) + 1);
            if (NULL == msg->method ) {
                CMP_ERR(cmp_log_error_memory);
                LITE_free(node);
                return FAIL_RETURN;
            }
            memset(msg->method , 0x0, strlen(node));
            strncpy(msg->method , node, strlen(node));
            LITE_free(node);
            node = NULL;

            msg->code = 0;
            return SUCCESS_RETURN;
        }

        default:
            CMP_ERR(cmp_log_error_type);
            return FAIL_RETURN;
    }
    return SUCCESS_RETURN;
}
#else 
int iotx_cmp_parse_payload(void* payload,
                    int payload_length,
                    iotx_cmp_message_info_pt msg)
{
    char* payload_pt = (char*)payload;
    char* node = NULL;

    if (NULL == payload || NULL == msg || 0 == payload_length) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }
    
    node = LITE_json_value_of((char*)string_id, payload_pt);
    if (node == NULL) {
        msg->message_type = IOTX_CMP_MESSAGE_RAW;
        msg->code = 0;
        msg->parameter = CMP_malloc(payload_length + 1);
        if (NULL == msg->parameter) {
            CMP_ERR(cmp_log_error_memory);
            LITE_free(node);
            return FAIL_RETURN;
        }
        memset(msg->parameter, 0x0, payload_length);
        memcpy(msg->parameter, payload, payload_length);
        msg->parameter_length = payload_length;
        msg->method = NULL;
        msg->id = 0;
        return SUCCESS_RETURN;        
    }
    
    msg->id = atoi(node);
    LITE_free(node);
    node = NULL;

    /* parse code */
    node = LITE_json_value_of((char*)string_code, payload_pt);
    if (node == NULL) {
        msg->message_type = IOTX_CMP_MESSAGE_REQUEST;
        
        /* parse params */
        node = LITE_json_value_of((char*)string_params, payload_pt);
        if (node == NULL) {
            CMP_ERR(cmp_log_error_parse_params);
            return FAIL_RETURN;
        }

        msg->parameter = CMP_malloc(strlen(node) + 1);
        if (NULL == msg->parameter) {
            CMP_ERR(cmp_log_error_memory);
            LITE_free(node);
            return FAIL_RETURN;
        }
        memset(msg->parameter, 0x0, strlen(node));
        strncpy(msg->parameter, node, strlen(node));
        msg->parameter_length = strlen(msg->parameter);
        LITE_free(node);
        node = NULL;

        /* parse method */
        node = LITE_json_value_of((char*)string_method, payload_pt);
        if (node == NULL) {
            CMP_ERR(cmp_log_error_parse_method);
            return FAIL_RETURN;
        }
        msg->method  = CMP_malloc(strlen(node) + 1);
        if (NULL == msg->method ) {
            CMP_ERR(cmp_log_error_memory);
            LITE_free(node);
            return FAIL_RETURN;
        }
        memset(msg->method , 0x0, strlen(node));
        strncpy(msg->method , node, strlen(node));
        LITE_free(node);
        node = NULL;

        msg->code = 0;
        return SUCCESS_RETURN;
    }
    msg->code = atoi(node);
    LITE_free(node);
    node = NULL;
    
    /* parse code */
    node = LITE_json_value_of((char*)string_code, payload_pt);
    if (node == NULL) {
        CMP_ERR(cmp_log_error_parse_code);
        return FAIL_RETURN;
    }
    msg->code = atoi(node);
    LITE_free(node);
    node = NULL;
    
    /* parse data */
    node = LITE_json_value_of((char*)string_data, payload_pt);
    if (node == NULL) {
        CMP_ERR(cmp_log_error_parse_data);
        return FAIL_RETURN;
    }
    msg->parameter = CMP_malloc(strlen(node) + 1);
    if (NULL == msg->parameter) {
        CMP_ERR(cmp_log_error_memory);
        LITE_free(node);
        return FAIL_RETURN;
    }
    memset(msg->parameter, 0x0, strlen(node));
    strncpy(msg->parameter, node, strlen(node));
    msg->parameter_length = strlen(msg->parameter);
    LITE_free(node);
    node = NULL;
    
    msg->method = NULL;
    return SUCCESS_RETURN;    
}
#endif


int iotx_cmp_splice_payload(void* payload,
                    int* payload_length,
                    int id,
                    iotx_cmp_message_info_pt msg)
{
    if (NULL == payload || NULL == payload_length || NULL == msg) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    switch(msg->message_type) {
        case IOTX_CMP_MESSAGE_RESPONSE: {
            HAL_Snprintf(payload,
                   (*payload_length),
                   string_response_format,
                   id,
                   msg->code,
                   (char*)msg->parameter);
            (*payload_length) = strlen(payload);
        }
            break;

        case IOTX_CMP_MESSAGE_REQUEST: {
            HAL_Snprintf(payload,
                   (*payload_length),
                   string_request_format,
                   id,
                   (char*)msg->parameter,
                   msg->method);
            (*payload_length) = strlen(payload);
        }
            break;

        case IOTX_CMP_MESSAGE_RAW: {
            memcpy(payload, msg->parameter, msg->parameter_length);
            (*payload_length) = msg->parameter_length;
        }
            break;

        default:
            break;
    }

    return SUCCESS_RETURN;
}


void iotx_cmp_free_message_info(iotx_cmp_message_info_pt message_info)
{
    if (message_info->URI)
        LITE_free(message_info->URI);
    if (message_info->parameter)
        LITE_free(message_info->parameter);
    if (message_info->method)
        LITE_free(message_info->method);
}


void iotx_cmp_response_func(void* pcontext, iotx_cmp_message_info_pt message_info)
{
    iotx_cmp_send_peer_pt peer = NULL;
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)pcontext;
#ifdef CMP_SUPPORT_TOPIC_DISPATCH
    iotx_cmp_mapping_pt mapping = NULL;
#else
    iotx_cmp_event_msg_t msg_event = {0};
    iotx_cmp_new_data_t new_data = {0};
#endif
    iotx_device_info_pt pdevice_info = iotx_device_info_get();

    if (NULL == cmp_pt || NULL == message_info) {
        CMP_ERR(cmp_log_error_parameter);
        return;
    }

#ifdef CMP_SUPPORT_TOPIC_DISPATCH
    /* find mapping */
    mapping = iotx_cmp_find_mapping(cmp_pt, message_info->URI, strlen(message_info->URI));

    if (NULL == mapping) {
        CMP_WARNING(cmp_log_warning_not_mapping);
        iotx_cmp_free_message_info(message_info);
        return;
    }

    if (NULL == mapping->func && NULL== mapping->mail_box) {
        CMP_WARNING(cmp_log_warning_not_func);
        iotx_cmp_free_message_info(message_info);
        return;
    }

    peer = CMP_malloc(sizeof(iotx_cmp_send_peer_t));
    if (NULL == peer) {
        CMP_ERR(cmp_log_error_memory);
        iotx_cmp_free_message_info(message_info);
        return;
    }

    memset(peer, 0x0, sizeof(iotx_cmp_send_peer_t));
    strncpy(peer->product_key, pdevice_info->product_key, strlen(pdevice_info->product_key));
    strncpy(peer->device_name, pdevice_info->device_name, strlen(pdevice_info->device_name));

    if (mapping->func) mapping->func(peer, message_info, mapping->user_data);
    if (peer) LITE_free(peer);
    iotx_cmp_free_message_info(message_info);
#else
    peer = CMP_malloc(sizeof(iotx_cmp_send_peer_t));
    if (NULL == peer) {
        CMP_ERR(cmp_log_error_memory);
        iotx_cmp_free_message_info(message_info);
        return;
    }

    memset(peer, 0x0, sizeof(iotx_cmp_send_peer_t));
    strncpy(peer->product_key, pdevice_info->product_key, strlen(pdevice_info->product_key));
    strncpy(peer->device_name, pdevice_info->device_name, strlen(pdevice_info->device_name));

    new_data.message_info = message_info;
    new_data.peer = peer;
    msg_event.event_id = IOTX_CMP_EVENT_NEW_DATA_RECEIVED;
    msg_event.msg = &new_data;        

    iotx_cmp_trigger_event_callback(cmp_pt, &msg_event);
    
    LITE_free(peer);
    iotx_cmp_free_message_info(message_info);
#endif
}


int iotx_cmp_parse_URI(char* URI, int URI_length, char* URI_param, iotx_cmp_uri_types_t* uri_type)
{
    char pk_dv[CMP_PRODUCT_KEY_LEN + CMP_DEVICE_NAME_LEN + 10] = {0};
    char* temp = NULL;
    iotx_device_info_pt pdevice_info = iotx_device_info_get();

    if (NULL == URI || NULL == URI_param)  return FAIL_RETURN;

    if (0 == strlen(URI)) return FAIL_RETURN;

    HAL_Snprintf(pk_dv,
           CMP_PRODUCT_KEY_LEN + CMP_DEVICE_NAME_LEN + 10,
           string_SYS_URI,
           pdevice_info->product_key,
           pdevice_info->device_name);

    /* find /sys/product_key/device_name */
    temp = strstr(URI, pk_dv);

    if (temp) {
        temp = URI + strlen(pk_dv);
        strncpy(URI_param, temp, URI_length - strlen(pk_dv));
        (*uri_type) = IOTX_CMP_URI_SYS;
        CMP_INFO(cmp_log_info_URI_sys);
        CMP_INFO(cmp_log_info_URI_1, URI_length - strlen(pk_dv), URI_param);
        return SUCCESS_RETURN;
    }

    memset(pk_dv, 0x0, CMP_PRODUCT_KEY_LEN + CMP_DEVICE_NAME_LEN + 10);
    HAL_Snprintf(pk_dv,
           CMP_PRODUCT_KEY_LEN + CMP_DEVICE_NAME_LEN + 10,
           string_EXT_URI,
           pdevice_info->product_key,
           pdevice_info->device_name);

    /* find /ext/product_key/device_name */
    temp = strstr(URI, pk_dv);

    if (temp) {
        temp = URI + strlen(pk_dv);
        strncpy(URI_param, temp, URI_length - strlen(pk_dv));
        (*uri_type) = IOTX_CMP_URI_EXT;
        CMP_INFO(cmp_log_info_URI_ext);
        CMP_INFO(cmp_log_info_URI_1, URI_length - strlen(pk_dv), URI_param);
        return SUCCESS_RETURN;
    }

    strncpy(URI_param, URI, URI_length);
    (*uri_type) = IOTX_CMP_URI_UNDEFINE;
    CMP_INFO(cmp_log_info_URI_undefined);
    CMP_INFO(cmp_log_info_URI_1, URI_length, URI_param);
    return SUCCESS_RETURN;
}


int iotx_cmp_splice_URI(char* URI, int* URI_length, const char* URI_param, iotx_cmp_uri_types_t uri_type)
{
    iotx_device_info_pt pdevice_info = iotx_device_info_get();

    if (NULL == URI || NULL == URI_length) return FAIL_RETURN;

    if (0 == strlen(URI_param)) return FAIL_RETURN;

    memset(URI, 0x0, (*URI_length));

    switch(uri_type) {
        case IOTX_CMP_URI_SYS: {
            HAL_Snprintf(URI,
                   CMP_TOPIC_LEN_MAX,
                   string_SYS_URI_1,
                   pdevice_info->product_key,
                   pdevice_info->device_name,
                   URI_param);
        }
            break;

        case IOTX_CMP_URI_EXT: {
            HAL_Snprintf(URI,
                   CMP_TOPIC_LEN_MAX,
                   string_EXT_URI_1,
                   pdevice_info->product_key,
                   pdevice_info->device_name,
                   URI_param);
        }
            break;

        default: {
            strncpy(URI, URI_param, strlen(URI_param));
        }
            break;
    }

    (*URI_length) = strlen(URI);

    CMP_INFO(cmp_log_info_URI_length, *URI_length);
    CMP_INFO(cmp_log_info_URI, URI);

    return SUCCESS_RETURN;
}

iotx_cmp_connectivity_pt iotx_cmp_find_connectivity(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_send_peer_pt target)
{
    if (NULL == cmp_pt->connectivity_list)
        return NULL;

    if (NULL == target) {
        return cmp_pt->connectivity_list->node;
    }

    return NULL;
}

int iotx_cmp_add_connectivity(iotx_cmp_conntext_pt cmp_pt,
                    iotx_cmp_connectivity_pt connectivity)
{
    iotx_cmp_connectivity_list_pt conn = cmp_pt->connectivity_list;
    iotx_cmp_connectivity_list_pt new_conn = NULL;

    if (NULL == cmp_pt->connectivity_list) {

        cmp_pt->connectivity_list = CMP_malloc(sizeof(iotx_cmp_connectivity_list_t));

        if (NULL == cmp_pt->connectivity_list) {
            return FAIL_RETURN;
        }
        memset(cmp_pt->connectivity_list, 0x0, sizeof(iotx_cmp_connectivity_list_t));

        cmp_pt->connectivity_list->node = connectivity;
        cmp_pt->connectivity_list->next = NULL;
        return SUCCESS_RETURN;
    }

    while(conn->next) {
        conn = conn->next;
    }

    new_conn = CMP_malloc(sizeof(iotx_cmp_connectivity_list_t));
    if (NULL == cmp_pt->connectivity_list) {
        return FAIL_RETURN;
    }
    memset(new_conn->node, 0x0, sizeof(iotx_cmp_connectivity_list_t));
    new_conn->node = connectivity;
    new_conn->next = NULL;
    conn->next = new_conn;

    return SUCCESS_RETURN;
}

int iotx_cmp_add_connectivity_all(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_init_param_pt pparam)
{
    void* connectivity = NULL;

    /* cloud connection */
#ifdef CMP_VIA_MQTT_DIRECT
    connectivity = iotx_cmp_mqtt_direct_init(cmp_pt, pparam);
#else
    connectivity = iotx_cmp_cloud_conn_init(cmp_pt, pparam);
#endif
    if (NULL == connectivity) return FAIL_RETURN;

    if (FAIL_RETURN == iotx_cmp_add_connectivity(cmp_pt, connectivity)) return FAIL_RETURN;

    /* local connection */
#ifdef CMP_SUPPORT_LOCAL_CONN
    connectivity = iotx_cmp_local_conn_init(cmp_pt, pparam);
    if (NULL == connectivity) {
        return FAIL_RETURN;
    }

    if (NULL == iotx_cmp_add_connectivity(cmp_pt, connectivity)) {
        return FAIL_RETURN;
    }
#endif

    return SUCCESS_RETURN;
}

int iotx_cmp_remove_connectivity_all(iotx_cmp_conntext_pt cmp_pt)
{
    iotx_cmp_connectivity_list_pt conn_list = cmp_pt->connectivity_list;
    iotx_cmp_connectivity_list_pt curr_conn = NULL;

    if (NULL == cmp_pt->connectivity_list) return SUCCESS_RETURN;

    while(conn_list) {
        if (FAIL_RETURN == conn_list->node->deinit_func(conn_list->node)) {
            CMP_ERR(cmp_log_error_fail);
        }
        conn_list->node = NULL;

        curr_conn = conn_list;
        conn_list = conn_list->next;
        LITE_free(curr_conn);
    }
    cmp_pt->connectivity_list = NULL;

    return SUCCESS_RETURN;
}


iotx_cmp_mapping_pt iotx_cmp_get_mapping_node()
{
#ifndef CMP_SUPPORT_TOPIC_DISPATCH
    return NULL;

#else /* CMP_SUPPORT_TOPIC_DISPATCH */

#ifdef CMP_MAPPING_USE_POOL
    int i = 0;
    for (i = 0; i < CMP_SUPPORT_MAX_MAPPING_SIZE; i++) {
        if (g_cmp_mapping_list[i].is_used == 0) {
            g_cmp_mapping_list[i].is_used = 1;
            return &g_cmp_mapping_list[i];
        }
    }
    return NULL;
#else
    iotx_cmp_mapping_pt node = NULL;

    node = CMP_malloc(sizeof(iotx_cmp_mapping_t));
    if (NULL == node) return NULL;

    memset(node, 0x0, sizeof(iotx_cmp_mapping_t));
    return node;
#endif

#endif /* CMP_SUPPORT_TOPIC_DISPATCH */
}

int iotx_cmp_free_mapping_node(iotx_cmp_mapping_pt node)
{
#ifndef CMP_SUPPORT_TOPIC_DISPATCH
    return SUCCESS_RETURN;
#else /* CMP_SUPPORT_TOPIC_DISPATCH */
    if (NULL == node) return FAIL_RETURN;

#ifdef CMP_MAPPING_USE_POOL
    if (node->is_used == 1) {
        node->is_used = 0;
        memset(node, 0x0, sizeof(iotx_cmp_mapping_t));
        return SUCCESS_RETURN;
    }

    return FAIL_RETURN;
#else /* CMP_MAPPING_USE_POOL */
    LITE_free(node);
    return SUCCESS_RETURN;
#endif /* CMP_MAPPING_USE_POOL */

#endif /* CMP_SUPPORT_TOPIC_DISPATCH */
}


int iotx_cmp_add_mapping(iotx_cmp_conntext_pt cmp_pt,
                char* URI,
                iotx_cmp_message_types_t type,
                iotx_cmp_register_func_fpt func,
                void* user_data,
                void* mail_box)
{
#ifndef CMP_SUPPORT_TOPIC_DISPATCH
    return SUCCESS_RETURN;
#else /* CMP_SUPPORT_TOPIC_DISPATCH */
    iotx_cmp_mapping_pt mapping = NULL;

    if (NULL == cmp_pt || NULL == URI) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    mapping = iotx_cmp_get_mapping_node();
    if (NULL == mapping) return FAIL_RETURN;

    mapping->URI = CMP_malloc(strlen(URI) + 1);
    if (NULL == mapping->URI) {
        CMP_ERR(cmp_log_error_memory);
        iotx_cmp_free_mapping_node(mapping);
        return FAIL_RETURN;
    }
    memset(mapping->URI, 0x0, strlen(URI) + 1);
    strncpy(mapping->URI, URI, strlen(URI));

    mapping->type = type;
    mapping->func = func;
    mapping->user_data = user_data;
    mapping->mail_box = mail_box;

    mapping->next = cmp_pt->mapping_list;
    cmp_pt->mapping_list = mapping;

    return SUCCESS_RETURN;
#endif /* CMP_SUPPORT_TOPIC_DISPATCH */
}


/* remove */
int iotx_cmp_remove_mapping(iotx_cmp_conntext_pt cmp_pt, char* URI)
{
#ifndef CMP_SUPPORT_TOPIC_DISPATCH
    return SUCCESS_RETURN;
#else /* CMP_SUPPORT_TOPIC_DISPATCH */
    iotx_cmp_mapping_pt mapping = NULL;
    iotx_cmp_mapping_pt pre_mapping = NULL;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    pre_mapping = mapping = cmp_pt->mapping_list;

    /* first one is match */
    if (0 == strncmp(URI, mapping->URI, strlen(URI))) {
        if (NULL == mapping->next) {
            cmp_pt->mapping_list = NULL;
        } else {
            cmp_pt->mapping_list = mapping->next;
        }
    }

    while (mapping) {
        if (0 == strncmp(URI, mapping->URI, strlen(URI))) {
            CMP_INFO(cmp_log_info_remove_mapping);
            pre_mapping->next = mapping->next;

            LITE_free(mapping->URI);
            iotx_cmp_free_mapping_node(mapping);
            return SUCCESS_RETURN;
        }

        pre_mapping = mapping;
        mapping = mapping->next;
    }

    return FAIL_RETURN;
#endif /* CMP_SUPPORT_TOPIC_DISPATCH */
}


/* remove all */
int iotx_cmp_remove_mapping_all(iotx_cmp_conntext_pt cmp_pt)
{
#ifndef CMP_SUPPORT_TOPIC_DISPATCH
    return SUCCESS_RETURN;
#else /* CMP_SUPPORT_TOPIC_DISPATCH */
    iotx_cmp_mapping_pt mapping = NULL;
    iotx_cmp_mapping_pt next_mapping = NULL;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    next_mapping = mapping = cmp_pt->mapping_list;

    while (mapping) {
        next_mapping = mapping->next;

        LITE_free(mapping->URI);
        iotx_cmp_free_mapping_node(mapping);

        mapping = next_mapping;
    }

    cmp_pt->mapping_list = NULL;

    return FAIL_RETURN;
#endif /* CMP_SUPPORT_TOPIC_DISPATCH */    
}

iotx_cmp_mapping_pt iotx_cmp_find_mapping(iotx_cmp_conntext_pt cmp_pt, char* URI, int URI_length)
{
#ifndef CMP_SUPPORT_TOPIC_DISPATCH
    return NULL;
#else /* CMP_SUPPORT_TOPIC_DISPATCH */
    iotx_cmp_mapping_pt mapping = NULL;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return NULL;
    }
    mapping = cmp_pt->mapping_list;

    while (mapping) {
        if ((0 == strncmp(URI, mapping->URI, URI_length)) || (0 != strstr(mapping->URI, URI))) {
            return mapping;
        }

        mapping = mapping->next;
    }

    return NULL;
#endif /* CMP_SUPPORT_TOPIC_DISPATCH */
}


int iotx_cmp_register_service(iotx_cmp_conntext_pt cmp_pt,
                char* URI,
                iotx_cmp_message_types_t type,
                iotx_cmp_register_func_fpt register_func,
                void* user_data,
                void* mail_box)
{
    iotx_cmp_connectivity_list_pt conn_list = NULL;
#ifdef CMP_SUPPORT_TOPIC_DISPATCH
    iotx_cmp_mapping_pt mapping = NULL;
#endif

    if (NULL == cmp_pt || NULL == URI) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }
#ifdef CMP_SUPPORT_TOPIC_DISPATCH

    if (FAIL_RETURN == iotx_cmp_add_mapping(cmp_pt, URI, type, register_func, user_data, mail_box)) {
        iotx_cmp_event_msg_t event;
        iotx_cmp_event_result_t result_pt = {0};

        result_pt.result = -1;
        result_pt.URI = URI;
        event.event_id = IOTX_CMP_EVENT_REGISTER_RESULT;
        event.msg = (void*)&result_pt;

        CMP_ERR(cmp_log_error_fail_1);

        iotx_cmp_trigger_event_callback(cmp_pt, &event);

        return FAIL_RETURN;
    }

    mapping = iotx_cmp_find_mapping(cmp_pt, URI, strlen(URI));

    conn_list = cmp_pt->connectivity_list;

    while(conn_list && conn_list->node) {
        if (0 == conn_list->node->is_connected)
            return FAIL_RETURN;
        else if (conn_list->node->register_func)
            conn_list->node->register_func(cmp_pt, conn_list->node, mapping->URI);
        
        conn_list = conn_list->next;
    }
    
#else /* CMP_SUPPORT_TOPIC_DISPATCH */
    conn_list = cmp_pt->connectivity_list;

    while(conn_list && conn_list->node) {
        if (0 == conn_list->node->is_connected)
            return FAIL_RETURN;
        else if (conn_list->node->register_func)
            conn_list->node->register_func(cmp_pt, conn_list->node, URI);
        
        conn_list = conn_list->next;
    }
#endif

    return SUCCESS_RETURN;
}


int iotx_cmp_unregister_service(iotx_cmp_conntext_pt cmp_pt, char* URI)
{
    iotx_cmp_connectivity_list_pt conn_list = NULL;

    if (NULL == cmp_pt || NULL == URI) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    conn_list = cmp_pt->connectivity_list;

    while(conn_list) {
        if (0 == conn_list->node->is_connected)
            return FAIL_RETURN;
        else if (conn_list->node->unregister_func)
            conn_list->node->unregister_func(cmp_pt, conn_list->node, URI);
        
        conn_list = conn_list->next;
    }

    if (FAIL_RETURN == iotx_cmp_remove_mapping(cmp_pt, URI)) {
        CMP_ERR(cmp_log_error_fail);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

static int iotx_cmp_send_data_to_cloud(iotx_cmp_conntext_pt cmp_pt,
                void* connectivity_pt,
                char* URI,
                iotx_cmp_message_ack_types_t ack_type,
                void* payload,
                int payload_length)
{
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;
    
    if (NULL != connectivity && connectivity->is_connected == 1) {
#ifdef CMP_VIA_MQTT_DIRECT
    return iotx_cmp_mqtt_direct_send(cmp_pt, connectivity_pt, URI, ack_type, payload, payload_length);
#else
    return iotx_cmp_cloud_conn_send(cmp_pt, connectivity_pt, URI, ack_type, payload, payload_length);
#endif
    }
    return FAIL_RETURN;
}

#ifdef CMP_SUPPORT_LOCAL_CONN_CONN

static int iotx_cmp_send_dispatch(iotx_cmp_send_peer_pt target)
{
    iotx_device_info_pt pdevice_info = iotx_device_info_get();

    if (target == NULL) {
        return 1;
    }

    if (0 == strncmp(target->product_key, pdevice_info->product_key, strlen(target->product_key)) &&
        0 == strncmp(target->device_name, pdevice_info->device_name, strlen(target->device_name))) {
        return 2;
    }

    return 3;
}

static int iotx_cmp_send_data_to_local(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_send_peer_pt target,
                char* URI,
                iotx_cmp_message_ack_types_t ack_type,
                void* payload,
                int payload_length)
{
    return iotx_cmp_local_conn_send(cmp_pt, URI, ack_type, payload, payload_length);
}

static int iotx_cmp_send_data_to_all(iotx_cmp_conntext_pt cmp_pt,
                char* URI,
                iotx_cmp_message_ack_types_t ack_type,
                void* payload,
                int payload_length)
{
    if (FAIL_RETURN == iotx_cmp_send_data_to_cloud(cmp_pt,
                        iotx_cmp_find_connectivity(cmp_pt, NULL),
                        URI,
                        ack_type,
                        payload,
                        payload_length)) {
        CMP_ERR(cmp_log_error_fail);
    }

    /* iotx_cmp_send_data_to_local */
    return FAIL_RETURN;
}
#endif

int iotx_cmp_parse_message(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_message_info_pt message_info,
                char** URI,
                void** payload,
                int* payload_length)
{
    int URI_length = CMP_TOPIC_LEN_MAX;

    (*URI) = CMP_malloc(CMP_TOPIC_LEN_MAX);
    if (NULL == (*URI)) {
        CMP_ERR(cmp_log_error_memory);
        return FAIL_RETURN;
    }

    if (FAIL_RETURN == iotx_cmp_splice_URI((*URI), &URI_length, message_info->URI, message_info->URI_type)) {
        CMP_ERR(cmp_log_error_splice_payload);
        LITE_free((*URI));

        return FAIL_RETURN;
    }

    (*payload_length) = message_info->parameter_length + 10 +
        strlen(string_request_format);

    if (message_info->method)
        (*payload_length) += strlen(message_info->method);

    CMP_INFO(cmp_log_info_payload_length, (*payload_length));

    (*payload) = CMP_malloc((*payload_length));
    if (NULL == (*payload)) {
        CMP_ERR(cmp_log_error_memory);
        LITE_free((*URI));
        LITE_free((*payload));
        return FAIL_RETURN;
    }

    if (0 == message_info->id) {
        message_info->id = iotx_cmp_get_next_message_id(cmp_pt);
    }

    if (FAIL_RETURN == iotx_cmp_splice_payload((*payload), payload_length, message_info->id, message_info)) {
        CMP_ERR(cmp_log_error_parse_payload);
        LITE_free((*URI));
        LITE_free((*payload));
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}


int iotx_cmp_send_data(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_send_peer_pt target,
                char* URI,
                iotx_cmp_message_ack_types_t ack_type,
                void* payload,
                int payload_length)
{
    int rc = -1;

#ifdef CMP_SUPPORT_LOCAL_CONN_CONN
    int peer = -1;
    peer = iotx_cmp_send_dispatch(target);
    switch (peer) {
        /* send all */
        case 1: {
            rc = iotx_cmp_send_data_to_all(cmp_pt, URI, ack_type, payload, payload_length);
        }
            break;

        /* send cloud */
        case 2:{
            rc = iotx_cmp_send_data_to_cloud(cmp_pt,
                        iotx_cmp_find_connectivity(cmp_pt, NULL),
                        URI,
                        ack_type,
                        payload,
                        payload_length);
        }
            break;

        case 3:{
            rc = iotx_cmp_send_data_to_local(cmp_pt, target, URI, ack_type, payload, payload_length);
        }
            break;
        default:{
            return FAIL_RETURN;
        }
            break;
    }
#else
    rc = iotx_cmp_send_data_to_cloud(cmp_pt,
                    iotx_cmp_find_connectivity(cmp_pt, NULL),
                    URI,
                    ack_type,
                    payload,
                    payload_length);
#endif
    if (rc < 0) {
        iotx_cmp_event_msg_t event;
        iotx_cmp_event_result_t result_pt = {0};

        result_pt.result = -1;
        result_pt.URI = URI;
        event.event_id = IOTX_CMP_EVENT_SEND_RESULT;
        event.msg = (void*)&result_pt;

        CMP_ERR(cmp_log_error_fail_rc, rc);

        /* todo: send fail is not a event */
        iotx_cmp_trigger_event_callback(cmp_pt, &event);
    }

    LITE_free(URI);
    LITE_free(payload);
    return rc < 0 ? FAIL_RETURN : SUCCESS_RETURN;
}


int iotx_cmp_trigger_event_callback(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_event_msg_pt msg)
{
    if (cmp_pt->event_callback_list) {
        iotx_cmp_event_callback_list_pt event_callback = cmp_pt->event_callback_list;

        while (event_callback) {
			if(event_callback->node && event_callback->node->event_func)
                event_callback->node->event_func(cmp_pt, msg, event_callback->node->user_data);
            event_callback = event_callback->next;
        }
    } else {
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}


#ifdef CMP_SUPPORT_MULTI_THREAD

iotx_cmp_process_list_node_pt iotx_cmp_get_list_node(iotx_cmp_process_types_t type)
{
#ifdef CMP_PROCESS_NODE_USE_POOL
    int i = 0;
    iotx_cmp_process_list_node_pt list = NULL;

    if (IOTX_CMP_PROCESS_TYPE_CLOUD == type) {
        list = g_cmp_cloud_process_node_list;
    }
#ifdef CMP_SUPPORT_LOCAL_CONN
    else if (IOTX_CMP_PROCESS_TYPE_LOCAL == type) {
        list = g_cmp_local_process_node_list;
    }
#endif
    else {
        return NULL;
    }

    for (i = 0; i < CMP_SUPPORT_MAX_PROCESS_NODE_SIZE; i++) {
        if (list[i].is_used == 0) {
            list[i].is_used = 1;
            return &list[i];
        }
    }
    return NULL;
#else
    iotx_cmp_process_list_node_pt node = NULL;
    node = CMP_malloc(sizeof(iotx_cmp_process_list_node_t));
    if (NULL == node) {
        return NULL;
    }

    memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));
    return node;
#endif
}

int iotx_cmp_free_list_node(iotx_cmp_process_list_node_pt node)
{
    if (NULL == node) return FAIL_RETURN;

#ifdef CMP_PROCESS_NODE_USE_POOL
    if (node->is_used == 1) {
        node->is_used = 0;
        memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));
        return SUCCESS_RETURN;
    }

    return FAIL_RETURN;
#else
    LITE_free(node);
    return SUCCESS_RETURN;
#endif
}

int iotx_cmp_free_list_node_all(iotx_cmp_conntext_pt cmp_pt)
{
    iotx_cmp_process_list_node_pt node = NULL;
    iotx_cmp_process_list_node_pt next_node = NULL;

    if (NULL == cmp_pt || NULL == cmp_pt->process_cloud_list) return FAIL_RETURN;

    node = cmp_pt->process_cloud_list->header;

    while (node) {
        switch (node->type)
        {
            case IOTX_CMP_PROCESS_CLOUD_REGISTER: {
                iotx_cmp_process_register_pt register_pt = (iotx_cmp_process_register_pt)node->msg;
                LITE_free(register_pt->URI);
                LITE_free(register_pt);
            }
                break;

            case IOTX_CMP_PROCESS_CLOUD_UNREGISTER: {
                char* URI = (char*)node->msg;
                LITE_free(URI);
            }
                break;

            case IOTX_CMP_PROCESS_CLOUD_SEND: {
                iotx_cmp_process_send_pt send_pt = (iotx_cmp_process_send_pt)node->msg;
                if (send_pt->URI)
                    LITE_free(send_pt->URI);

                if (send_pt->payload)
                    LITE_free(send_pt->payload);

                if (send_pt->target)
                    LITE_free(send_pt->target);

                LITE_free(send_pt);
            }
                break;

            case IOTX_CMP_PROCESS_CLOUD_REGISTER_RESULT:
            case IOTX_CMP_PROCESS_CLOUD_UNREGISTER_RESULT: {
                iotx_cmp_process_register_result_pt register_pt = (iotx_cmp_process_register_result_pt)node->msg;
            #ifndef CMP_VIA_MQTT_DIRECT
                LITE_free(register_pt->URI);
            #endif
                LITE_free(register_pt);
            }
                break;

            case IOTX_CMP_PROCESS_CLOUD_NEW_DATA: {
                iotx_cmp_message_info_pt message_info = (iotx_cmp_message_info_pt)node->msg;
                iotx_cmp_free_message_info(message_info);
                LITE_free(message_info);
                break;
            }

            default:
                break;
        }
        next_node = node->next;

    #ifdef CMP_PROCESS_NODE_USE_POOL
        if (node->is_used == 1) {
            node->is_used = 0;
            memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));
        }
    #else
        LITE_free(node);
    #endif

        node = next_node;
    }

    LITE_free(cmp_pt->process_cloud_list);

#ifdef CMP_SUPPORT_LOCAL_CONN
    node = cmp_pt->process_local_list->header;

    while (node) {
        switch (node->type)
        {
            case IOTX_CMP_PROCESS_LOCAL_ADD_DEVICE: {
            }
                break;

            case IOTX_CMP_PROCESS_LOCAL_REMOVE_DEVICE: {
            }
                break;

            case IOTX_CMP_PROCESS_LOCAL_ADD_SERVICE: {
            }
                break;

            case IOTX_CMP_PROCESS_LOCAL_REMOVE_SERVICE: {
            }
                break;

            case IOTX_CMP_PROCESS_LOCAL_ADD_SERVICE_RESULT:
            case IOTX_CMP_PROCESS_LOCAL_REMOVE_SERVICE_RESULT: {
            }
                break;

            case IOTX_CMP_PROCESS_LOCAL_NEW_DATA: {
            }
                break;

            case IOTX_CMP_PROCESS_LOCAL_SEND: {
                break;
            }

            default:
                break;
        }

    #ifdef CMP_PROCESS_NODE_USE_POOL
        if (node->is_used == 1) {
            node->is_used = 0;
            memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));
        }
    #else
        LITE_free(node);
    #endif
        node = node->next;
    }

    LITE_free(cmp_pt->process_local_list);
#endif

    return SUCCESS_RETURN;
}

/* node is in */
int iotx_cmp_process_list_push(iotx_cmp_conntext_pt cmp_pt,
                    iotx_cmp_process_types_t type,
                    iotx_cmp_process_list_node_pt node)
{
    void* lock = NULL;
    iotx_cmp_process_list_pt list = NULL;

    if (NULL == cmp_pt || NULL == node) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    if (IOTX_CMP_PROCESS_TYPE_CLOUD == type) {
        lock = cmp_pt->process_cloud_lock;
        list = cmp_pt->process_cloud_list;
    }
#ifdef CMP_SUPPORT_LOCAL_CONN
    else if (IOTX_CMP_PROCESS_TYPE_LOCAL == type) {
        lock = cmp_pt->process_local_lock;
        list = cmp_pt->process_local_list;
    }
#endif
    else {
        return FAIL_RETURN;
    }

    HAL_MutexLock(lock);

    if (NULL == list) {
        list = CMP_malloc(sizeof(iotx_cmp_process_list_t));

        if (NULL == list) {
            HAL_MutexUnlock(lock);
            CMP_ERR(cmp_log_error_memory);
            return FAIL_RETURN;
        }

        list->header = list->tailer = NULL;
        list->size = 0;

        if (IOTX_CMP_PROCESS_TYPE_CLOUD == type) {
            cmp_pt->process_cloud_list = list;
        }
    #ifdef CMP_SUPPORT_LOCAL_CONN
        else if (IOTX_CMP_PROCESS_TYPE_LOCAL == type) {
            cmp_pt->process_local_list = list;
        }
    #endif
    }

    if (list->size == 0) {
        list->header = list->tailer = node;
        list->size = 1;
    } else if (list->size == CMP_SUPPORT_MAX_PROCESS_NODE_SIZE) {
        HAL_MutexUnlock(lock);
        CMP_ERR(cmp_log_error_process);
        return FAIL_RETURN;
    } else {
        node->pre = list->tailer;
        list->tailer->next = node;
        list->tailer = node;
        list->size++;
    }

    HAL_MutexUnlock(lock);

    return SUCCESS_RETURN;
}


iotx_cmp_process_list_node_pt iotx_cmp_process_list_pop(iotx_cmp_conntext_pt cmp_pt,
                    iotx_cmp_process_types_t type)
{
    void* lock = NULL;
    iotx_cmp_process_list_pt list = NULL;
    iotx_cmp_process_list_node_pt node = NULL;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return NULL;
    }

    if (IOTX_CMP_PROCESS_TYPE_CLOUD == type) {
        lock = cmp_pt->process_cloud_lock;
        list = cmp_pt->process_cloud_list;
    }
#ifdef CMP_SUPPORT_LOCAL_CONN
    else if (IOTX_CMP_PROCESS_TYPE_LOCAL == type) {
        lock = cmp_pt->process_local_lock;
        list = cmp_pt->process_local_list;
    }
#endif

    if (NULL == list) {
        CMP_ERR(cmp_log_error_status);
        return NULL;
    }

    HAL_MutexLock(lock);

    if (list->size == 1) {
        node = list->header;
        list->header = list->tailer = NULL;
        list->size = 0;
        LITE_free(list);
        list = NULL;
        if (IOTX_CMP_PROCESS_TYPE_CLOUD == type) {
            cmp_pt->process_cloud_list = NULL;
        }
    #ifdef CMP_SUPPORT_LOCAL_CONN
        else if (IOTX_CMP_PROCESS_TYPE_LOCAL == type) {
            cmp_pt->process_local_list = NULL;
        }
    #endif
    } else {
        node = list->header;
        list->header = list->header->next;
        list->header->pre = NULL;
        list->size--;
    }

    HAL_MutexUnlock(lock);
    return node;
}


int iotx_cmp_process_list_get_size(iotx_cmp_conntext_pt cmp_pt,
                    iotx_cmp_process_types_t type)
{
    iotx_cmp_process_list_pt list = NULL;
    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    if (IOTX_CMP_PROCESS_TYPE_CLOUD == type) {
        list = cmp_pt->process_cloud_list;
    }
#ifdef CMP_SUPPORT_LOCAL_CONN
    else if (IOTX_CMP_PROCESS_TYPE_LOCAL == type) {
        list = cmp_pt->process_local_list;
    }
#endif

    if (NULL == list) return 0;

    return list->size;
}


void* iotx_cmp_cloud_process(void *pclient)
{
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)pclient;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return NULL;
    }

    CMP_INFO(cmp_log_info_enter_process_1);

    while (!cmp_pt->thread_stop) {
        iotx_cmp_connectivity_pt connectivity = iotx_cmp_find_connectivity(cmp_pt, NULL);

        if (NULL == connectivity) {
            HAL_SleepMs(50);
            continue;
        }

        if (0 == connectivity->is_connected) {
            connectivity->connect_func(cmp_pt, connectivity);
            if (1 == connectivity->is_connected) {
                iotx_cmp_event_msg_t event_msg = {0};
                event_msg.event_id = IOTX_CMP_EVENT_CLOUD_CONNECTED;
                event_msg.msg = NULL;
                iotx_cmp_trigger_event_callback(cmp_pt, &event_msg);
            }
            continue;
        }
        
        if (iotx_cmp_process_list_get_size(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD) > 0) {
            iotx_cmp_process_list_node_pt node = NULL;
            if (NULL != (node = iotx_cmp_process_list_pop(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD))) {
                CMP_INFO(cmp_log_info_process_type, node->type);

                switch(node->type) {
                    case IOTX_CMP_PROCESS_CLOUD_REGISTER: {
                        iotx_cmp_process_register_pt register_pt = (iotx_cmp_process_register_pt)node->msg;
                        if (FAIL_RETURN == iotx_cmp_register_service(cmp_pt, register_pt->URI, register_pt->type, register_pt->register_func,
                                register_pt->user_data, register_pt->mail_box)) {
                            CMP_ERR(cmp_log_error_fail);
                        }

                        LITE_free(register_pt->URI);
                        LITE_free(register_pt);
                    }
                        break;

                    case IOTX_CMP_PROCESS_CLOUD_UNREGISTER: {
                        char* URI = (char*)node->msg;
                        if (FAIL_RETURN == iotx_cmp_unregister_service(cmp_pt, URI)) {
                            CMP_ERR(cmp_log_error_fail);
                        }

                        LITE_free(URI);
                    }
                        break;

                    case IOTX_CMP_PROCESS_CLOUD_SEND: {
                        iotx_cmp_process_send_pt send_pt = (iotx_cmp_process_send_pt)node->msg;
                        if (FAIL_RETURN == iotx_cmp_send_data(cmp_pt, send_pt->target, send_pt->URI, send_pt->ack_type, send_pt->payload, send_pt->payload_length)) {
                            CMP_ERR(cmp_log_error_fail);
                        }

                        if (send_pt->target)
                            LITE_free(send_pt->target);
                        LITE_free(send_pt);
                    }
                        break;

                    case IOTX_CMP_PROCESS_CLOUD_DISCONNECT: {
                    #ifdef CMP_VIA_MQTT_DIRECT
                        iotx_cmp_mqtt_direct_disconnect_handler(cmp_pt);
                    #else
                        iotx_cmp_cloud_conn_disconnect_handler(cmp_pt);
                    #endif
                    }
                        break;

                    case IOTX_CMP_PROCESS_CLOUD_RECONNECT: {
                    #ifdef CMP_VIA_MQTT_DIRECT
                        iotx_cmp_mqtt_direct_reconnect_handler(cmp_pt);
                    #else
                        iotx_cmp_cloud_conn_reconnect_handler(cmp_pt);
                    #endif
                    }
                        break;

                    case IOTX_CMP_PROCESS_CLOUD_REGISTER_RESULT:
                    case IOTX_CMP_PROCESS_CLOUD_UNREGISTER_RESULT: {
                        iotx_cmp_process_register_result_pt register_pt = (iotx_cmp_process_register_result_pt)node->msg;
                    #ifdef CMP_VIA_MQTT_DIRECT
                        iotx_cmp_mqtt_direct_register_handler(cmp_pt, register_pt->URI, register_pt->result, register_pt->is_register);
                    #else
                        iotx_cmp_cloud_conn_register_handler(cmp_pt, register_pt->URI, register_pt->result, register_pt->is_register);
                        LITE_free(register_pt->URI);
                    #endif
                        LITE_free(register_pt);
                    }
                        break;

                    case IOTX_CMP_PROCESS_CLOUD_NEW_DATA: {
                        iotx_cmp_message_info_pt message_info = (iotx_cmp_message_info_pt)node->msg;
                    #ifdef CMP_VIA_MQTT_DIRECT
                        iotx_cmp_mqtt_direct_response_handler(cmp_pt, message_info);
                    #else
                        iotx_cmp_cloud_conn_response_handler(cmp_pt, message_info);
                    #endif
                        LITE_free(node->msg);
                        break;
                    }

                    default:
                        break;
                }

                iotx_cmp_free_list_node(node);
            }
        }

            /* cloud yield */
        #ifdef CMP_VIA_MQTT_DIRECT
            iotx_cmp_mqtt_direct_yield(iotx_cmp_find_connectivity(cmp_pt, NULL), 50);
        #else
            iotx_cmp_cloud_conn_yield(iotx_cmp_find_connectivity(cmp_pt, NULL), 50);
        #endif
    }

    cmp_pt->thread_is_stoped = 1;

    return NULL;
}

#ifdef CMP_SUPPORT_LOCAL_CONN
void* iotx_cmp_local_process(void *pclient)
{
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)pclient;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return NULL;
    }

    CMP_INFO(cmp_log_info_enter_process_2);

    while (!cmp_pt->thread_stop) {
        if (iotx_cmp_process_list_get_size(cmp_pt, IOTX_CMP_PROCESS_TYPE_LOCAL) > 0) {
            iotx_cmp_process_list_node_pt node = NULL;
            if (NULL != (node = iotx_cmp_process_list_pop(cmp_pt, IOTX_CMP_PROCESS_TYPE_LOCAL))) {
                CMP_INFO(cmp_log_info_process_type, node->type);

                switch(node->type) {
                    case IOTX_CMP_PROCESS_LOCAL_ADD_DEVICE: {
                    }
                        break;

                    case IOTX_CMP_PROCESS_LOCAL_REMOVE_DEVICE: {
                    }
                        break;

                    case IOTX_CMP_PROCESS_LOCAL_SEND: {
                    }
                        break;

                    case IOTX_CMP_PROCESS_LOCAL_DISCONNECT: {
                    }
                        break;

                    case IOTX_CMP_PROCESS_LOCAL_RECONNECT: {
                    }
                        break;

                    case IOTX_CMP_PROCESS_LOCAL_ADD_SERVICE: {
                    }
                        break;

                    case IOTX_CMP_PROCESS_LOCAL_REMOVE_SERVICE: {
                    }
                        break;

                    case IOTX_CMP_PROCESS_LOCAL_ADD_SERVICE_RESULT:
                    case IOTX_CMP_PROCESS_LOCAL_REMOVE_SERVICE_RESULT: {
                    }
                        break;

                    case IOTX_CMP_PROCESS_LOCAL_NEW_DATA: {
                        break;
                    }

                    default:
                        break;
                }

                iotx_cmp_free_list_node(node);
            }
            /* local yield */

        } else {

        /* cloud yield */
        }
    }

    return NULL;
}

#endif

#endif



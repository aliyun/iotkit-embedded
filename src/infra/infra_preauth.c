/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include "infra_config.h"

#ifdef INFRA_PREAUTH

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "infra_state.h"
#include "infra_compat.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_httpc.h"
#include "infra_preauth.h"
#include "infra_string.h"

#define PREAUTH_HTTP_REQ_LEN            300
#define PREAUTH_HTTP_RSP_LEN            300

#define PREAUTH_IOT_ID_MAXLEN           (64)
#define PREAUTH_IOT_TOKEN_MAXLEN        (65)
#define PREAUTH_IOT_HOST_MAXLEN         (64)

#ifndef CONFIG_GUIDER_AUTH_TIMEOUT
    #define CONFIG_GUIDER_AUTH_TIMEOUT      (10 * 1000)
#endif

#ifdef SUPPORT_TLS
    extern const char *iotx_ca_crt;
#endif

typedef struct {
    char *payload;
    int payload_len;
} preauth_http_response_t;

int _preauth_assemble_auth_req_string(const iotx_dev_meta_info_t *dev_meta,
                                      const char *device_id, char *request_buff, uint32_t buff_len)
{
    uint8_t i = 0;
    const char *kv[][2] = {
        { "productKey", NULL },
        { "deviceName", NULL },
        { "clientId", NULL },
        { "resources", "mqtt" }
    };

    if (dev_meta == NULL || device_id == NULL || request_buff == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    kv[0][1] = dev_meta->product_key;
    kv[1][1] = dev_meta->device_name;
    kv[2][1] = device_id;

    for (i = 0; i < (sizeof(kv) / (sizeof(kv[0]))); i++) {
        if ((strlen(request_buff) + strlen(kv[i][0]) + strlen(kv[i][1]) + 2) >=
            buff_len) {
            return STATE_HTTP_PREAUTH_REQ_BUF_SHORT;
        }

        memcpy(request_buff + strlen(request_buff), kv[i][0], strlen(kv[i][0]));
        memcpy(request_buff + strlen(request_buff), "=", 1);
        memcpy(request_buff + strlen(request_buff), kv[i][1], strlen(kv[i][1]));
        memcpy(request_buff + strlen(request_buff), "&", 1);
    }

    memset(request_buff + strlen(request_buff) - 1, '\0', 1);
    return STATE_SUCCESS;
}

static int _preauth_get_string_value(char *p_string, char *value_buff, uint32_t buff_len)
{
    char *p = p_string;
    char *p_start = NULL;
    char *p_end = NULL;
    uint32_t len = 0;

    while (*(++p) != ',' || *p != '}') {
        if (*p == '\"') {
            if (p_start) {
                p_end = p;
                break;
            } else {
                p_start = p + 1;
            }
        }
    }

    if (p_start == NULL || p_end == NULL) {
        return -1;
    }

    len = p_end - p_start;
    if (len > buff_len) {
        return -1;
    }

    memcpy(value_buff, p_start, len);
    return STATE_SUCCESS;
}

static int _preauth_parse_auth_rsp_string(char *json_string, uint32_t string_len, iotx_sign_mqtt_t *output)
{
    int32_t res = STATE_SUCCESS;
    char *code = NULL, *host = NULL, *port = NULL;
    uint32_t code_len = 0, host_len = 0, port_len = 0;
    uint32_t port_num = 0;

    printf("json_string: %s\n",json_string);

    res = infra_json_value(json_string, string_len, "code", strlen("code"), &code, &code_len);
    if (res < SUCCESS_RETURN || code_len != strlen("200") || memcmp(code, "200", code_len) != 0)  {
        return STATE_HTTP_PREAUTH_INVALID_RESP;
    }

    res = infra_json_value(json_string, string_len, "host", strlen("host"), &host, &host_len);
    if (res < SUCCESS_RETURN) {
        return STATE_HTTP_PREAUTH_INVALID_RESP;
    }

    res = infra_json_value(json_string, string_len, "port", strlen("port"), &port, &port_len);
    if (res < SUCCESS_RETURN) {
        return STATE_HTTP_PREAUTH_INVALID_RESP;
    }

    utils_str2uint(port, port_len, &port_num);

    memcpy(output->hostname, host + 1, host_len -2);
    output->port = (uint16_t)port_num;

    return STATE_SUCCESS;
}

static int _preauth_recv_callback(char *ptr, int length, int total_length, void *userdata)
{
    preauth_http_response_t *response = (preauth_http_response_t *)userdata;
    if (strlen(response->payload) + length > response->payload_len) {
        return -1;
    }
    memcpy(response->payload + strlen(response->payload), ptr, length);

    return length;
}

int preauth_get_connection_info(iotx_mqtt_region_types_t region, iotx_dev_meta_info_t *dev_meta,
                                const char *device_id, iotx_sign_mqtt_t *preauth_output)
{
    char http_url[128] = "http://";
    char http_url_frag[] = "/auth/bootstrap";
    uint64_t timestamp;
#ifdef SUPPORT_TLS
    int http_port = 443;
    char *pub_key = (char *)iotx_ca_crt;
#else
    int http_port = 80;
    char *pub_key = NULL;
#endif
    int res = -1;
    void *http_handle = NULL;
    iotx_http_method_t http_method = IOTX_HTTP_POST;
    int http_timeout_ms = CONFIG_GUIDER_AUTH_TIMEOUT;
    preauth_http_response_t response;
    char *http_header = "Accept: text/xml,text/javascript,text/html,application/json\r\n" \
                        "Content-Type: application/x-www-form-urlencoded;charset=utf-8\r\n";
    int http_recv_maxlen = PREAUTH_HTTP_RSP_LEN;
    char request_buff[PREAUTH_HTTP_REQ_LEN] = {0};
    char response_buff[PREAUTH_HTTP_RSP_LEN] = {0};

    if (g_infra_http_domain[region] == NULL) {
        return STATE_USER_INPUT_HTTP_DOMAIN;
    }

    memset(&response, 0, sizeof(preauth_http_response_t));
    memcpy(http_url + strlen(http_url), g_infra_http_domain[region], strlen(g_infra_http_domain[region]));
    memcpy(http_url + strlen(http_url), http_url_frag, sizeof(http_url_frag));

    _preauth_assemble_auth_req_string(dev_meta, device_id, request_buff, sizeof(request_buff));

    response.payload = response_buff;
    response.payload_len = PREAUTH_HTTP_RSP_LEN;

    http_handle = wrapper_http_init();
    wrapper_http_setopt(http_handle, IOTX_HTTPOPT_URL, (void *)http_url);
    wrapper_http_setopt(http_handle, IOTX_HTTPOPT_PORT, (void *)&http_port);
    wrapper_http_setopt(http_handle, IOTX_HTTPOPT_METHOD, (void *)&http_method);
    wrapper_http_setopt(http_handle, IOTX_HTTPOPT_HEADER, (void *)http_header);
    wrapper_http_setopt(http_handle, IOTX_HTTPOPT_CERT, (void *)pub_key);
    wrapper_http_setopt(http_handle, IOTX_HTTPOPT_TIMEOUT, (void *)&http_timeout_ms);
    wrapper_http_setopt(http_handle, IOTX_HTTPOPT_RECVCALLBACK, (void *)_preauth_recv_callback);
    wrapper_http_setopt(http_handle, IOTX_HTTPOPT_RECVMAXLEN, (void *)&http_recv_maxlen);
    wrapper_http_setopt(http_handle, IOTX_HTTPOPT_RECVCONTEXT, (void *)&response);

    iotx_state_event(ITE_STATE_MQTT_COMM, STATE_HTTP_PREAUTH_REQ, "%s", http_url);
    iotx_state_event(ITE_STATE_MQTT_COMM, STATE_HTTP_PREAUTH_REQ, "%d", http_port);
    iotx_state_event(ITE_STATE_MQTT_COMM, STATE_HTTP_PREAUTH_REQ, "%d", http_timeout_ms);

    timestamp = HAL_UptimeMs();
    res = wrapper_http_perform(http_handle, request_buff, strlen(request_buff));

    iotx_state_event(ITE_STATE_MQTT_COMM, STATE_HTTP_PREAUTH_REQ, "preauth done in %d ms - ret: %d",
                     (int)(HAL_UptimeMs() - timestamp), res);

    wrapper_http_deinit(&http_handle);

    if (res < SUCCESS_RETURN) {
        return res;
    }

#ifdef INFRA_LOG_NETWORK_PAYLOAD
    preauth_info("Downstream Payload:");
    iotx_facility_json_print(response_buff, LOG_INFO_LEVEL, '<');
#endif
    res = _preauth_parse_auth_rsp_string(response_buff, strlen(response_buff), preauth_output);

    return res;
}

#endif /* #ifdef MQTT_PRE_AUTH */

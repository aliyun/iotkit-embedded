
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "aliot_platform.h"

#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_md5.h"
#include "aliyun_iot_common_hmac.h"
#include "aliyun_iot_common_httpclient.h"
#include "aliyun_iot_common_jsonparser.h"

#include "aliyun_iot_ca.h"
#include "aliyun_iot_auth.h"


const static char *iot_atuh_host = "http://iot-auth.cn-shanghai.aliyuncs.com/auth/devicename";

static int aliyun_iot_get_id_token(
            const char *auth_host,
            const char *product_key,
            const char *device_name,
            const char *device_secret,
            const char *client_id,
            const char *version,
            const char *timestamp,
            const char *resources,
            char *iot_id,
            char *iot_token,
            char *host,
            uint16_t *pport)
{
#define SIGN_SOURCE_LEN     (256)
#define HTTP_POST_MAX_LEN   (1024)
#define HTTP_RESP_MAX_LEN   (1024)

    int ret = -1, length;
    char sign[33];
    char *buf = NULL, *post_buf = NULL, *response_buf = NULL;

    httpclient_t httpclient;
    httpclient_data_t httpclient_data;


    length = strlen(client_id);
    length += strlen(product_key);
    length += strlen(device_name);
    length += strlen(timestamp);
    length += 40; //40 chars space for key strings(clientId,deviceName,productKey,timestamp)

    if (length > SIGN_SOURCE_LEN) {
        ALIOT_LOG_WARN("The total length may be is too long. client_id=%s, product_key=%s, device_name=%s, timestamp= %s",
                       client_id, product_key, device_name, timestamp);
    }

    if (NULL == (buf = aliot_platform_malloc(length))) {
        goto do_exit;
    }

    //Calculate sign
    memset(sign, 0, sizeof(sign));

    ret = snprintf(buf,
                   SIGN_SOURCE_LEN,
                   "clientId%sdeviceName%sproductKey%stimestamp%s",
                   client_id,
                   device_name,
                   product_key,
                   timestamp);
    if ((ret < 0) || (ret > SIGN_SOURCE_LEN)) {
        goto do_exit;
    }
    ALIOT_LOG_DEBUG("sign source=%s", buf);
    aliyun_iot_common_hmac_md5(buf, strlen(buf), sign, device_secret, strlen(device_secret));


    memset(&httpclient, 0, sizeof(httpclient_t));
    httpclient.header = "Accept: text/xml,text/javascript,text/html,application/json\r\n";

    memset(&httpclient_data, 0, sizeof(httpclient_data_t));

    post_buf = (char *) aliot_platform_malloc(HTTP_POST_MAX_LEN);
    if (NULL == post_buf) {
        ALIOT_LOG_ERROR("malloc http post buf failed!");
        return ERROR_MALLOC;
    }
    memset(post_buf, 0, HTTP_POST_MAX_LEN);

    ret = snprintf(post_buf,
                   HTTP_POST_MAX_LEN,
                   "productKey=%s&deviceName=%s&sign=%s&version=%s&clientId=%s&timestamp=%s&resources=%s",
                   product_key,
                   device_name,
                   sign,
                   version,
                   client_id,
                   timestamp,
                   resources);

    if ((ret < 0) || (ret >= HTTP_POST_MAX_LEN)) {
        ALIOT_LOG_ERROR("http message body is too long");
        ret = -1;
        goto do_exit;
    }

    ALIOT_LOG_DEBUG("http content:%s\n\r", post_buf);

    ret = strlen(post_buf);

    response_buf = (char *) aliot_platform_malloc(HTTP_RESP_MAX_LEN);
    if (NULL == response_buf) {
        ALIOT_LOG_ERROR("malloc http response buf failed!");
        return ERROR_MALLOC;
    }
    memset(response_buf, 0, HTTP_RESP_MAX_LEN);

    httpclient_data.post_content_type = "application/x-www-form-urlencoded;charset=utf-8";
    httpclient_data.post_buf = post_buf;
    httpclient_data.post_buf_len = ret;
    httpclient_data.response_buf = response_buf;
    httpclient_data.response_buf_len = HTTP_RESP_MAX_LEN;

    aliyun_iot_common_post(
                &httpclient,
                auth_host,
                443,
                aliyun_iot_ca_get(),
                &httpclient_data);

    ALIOT_LOG_DEBUG("http response:%s\n\r", httpclient_data.response_buf);


    //get iot-id and iot-token from response
    int type;
    const char *pvalue, *presrc;
    char port_str[6];

    //get iot-id
    if (NULL == (pvalue = json_get_value_by_fullname(
                                      httpclient_data.response_buf,
                                      strlen(httpclient_data.response_buf),
                                      "data.iotId",
                                      &length,
                                      &type))) {
        goto do_exit;
    }
    memcpy(iot_id, pvalue, length);
    iot_id[length] = '\0';


    //get iot-token
    if (NULL == (pvalue = json_get_value_by_fullname(
                                      httpclient_data.response_buf,
                                      strlen(httpclient_data.response_buf),
                                      "data.iotToken",
                                      &length,
                                      &type))) {
        goto do_exit;
    }
    memcpy(iot_token, pvalue, length);
    iot_token[length] = '\0';


    /*get host and port.*/

    if (NULL == (presrc = json_get_value_by_fullname(
                                      httpclient_data.response_buf,
                                      strlen(httpclient_data.response_buf),
                                      "data.resources.mqtt",
                                      &length,
                                      &type))) {
        goto do_exit;
    }

    //get host
    if (NULL == (pvalue = json_get_value_by_fullname(
                                      presrc,
                                      strlen(presrc),
                                      "host",
                                      &length,
                                      &type))) {
        goto do_exit;
    }
    memcpy(host, pvalue, length);
    host[length] = '\0';

    //get port
    if (NULL == (pvalue = json_get_value_by_fullname(
                                      presrc,
                                      strlen(presrc),
                                      "port",
                                      &length,
                                      &type))) {
        goto do_exit;
    }
    memcpy(port_str, pvalue, length);
    port_str[length] = '\0';
    *pport = atoi(port_str);

    ALIOT_LOG_DEBUG("\niot-id=%s\niot-token=%s\nhost=%s\nport=%d\r\n",
                    iot_id, iot_token, host, *pport);

    ret = 0;

do_exit:
    if (NULL != buf) {
        aliot_platform_free(buf);
    }

    if (NULL != post_buf) {
        aliot_platform_free(post_buf);
    }

    if (NULL != response_buf) {
        aliot_platform_free(response_buf);
    }

    return ret;
}


int32_t aliyun_iot_auth(aliot_device_info_pt pdevice_info, aliot_user_info_pt puser_info)
{
    int ret = 0;
    char iot_id[ALIOT_AUTH_IOT_ID + 1], iot_token[ALIOT_AUTH_IOT_TOKEN + 1], host[HOST_ADDRESS_LEN + 1];
    uint16_t port;

    if (0 != aliyun_iot_get_id_token(
                    iot_atuh_host,
                    pdevice_info->product_key,
                    pdevice_info->device_name,
                    pdevice_info->device_secret,
                    pdevice_info->device_id,
                    "default",
                    "2524608000000", //01 Jan 2050
                    "mqtt",
                    iot_id,
                    iot_token,
                    host,
                    &port)) {
        return -1;
    }


    strncpy(puser_info->user_name, iot_id, USER_NAME_LEN);
    strncpy(puser_info->password, iot_token, PASSWORD_LEN);
    strncpy(puser_info->host_name, host, HOST_ADDRESS_LEN);
    puser_info->port = port;
#ifdef ALIOT_MQTT_CHANNEL_ENCRYPT_SSL
    puser_info->pubKey = aliyun_iot_ca_get();
#else
    puser_info->pubKey = NULL;
#endif
    if (NULL == puser_info->pubKey) {
        //Append string "::nonesecure::" to client_id if TCP connection be used.
        ret = snprintf(puser_info->client_id,
                       CLIENT_ID_LEN,
                       "%s%s",
                       pdevice_info->device_id,
                       "::nonesecure::");

    } else {
        ret = snprintf(puser_info->client_id,
                       CLIENT_ID_LEN,
                       "%s",
                       pdevice_info->device_id);
    }

    if (ret >= CLIENT_ID_LEN) {
        ALIOT_LOG_ERROR("client_id is too long");
    } else if (ret < 0) {
        return -1;
    }

    return 0;
}

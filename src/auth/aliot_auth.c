
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "aliot_platform.h"

#include "aliot_error.h"
#include "lite/lite-log.h"
#include "lite/lite-utils.h"
#include "aliot_md5.h"
#include "aliot_hmac.h"
#include "aliot_httpclient.h"

#include "aliot_ca.h"
#include "aliot_auth.h"

#define _ONLINE

#ifdef _ONLINE
const static char *iot_atuh_host = "http://iot-auth.cn-shanghai.aliyuncs.com/auth/devicename";
#else
const static char *iot_atuh_host = "http://iot-auth-pre.cn-shanghai.aliyuncs.com/auth/devicename";
#endif

static int aliot_get_id_token(
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
        log_warning("The total length may be is too long. client_id=%s, product_key=%s, device_name=%s, timestamp= %s",
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
    log_debug("sign source = %.64s ...", buf);
    aliot_hmac_md5(buf, strlen(buf), sign, device_secret, strlen(device_secret));


    memset(&httpclient, 0, sizeof(httpclient_t));
    httpclient.header = "Accept: text/xml,text/javascript,text/html,application/json\r\n";

    memset(&httpclient_data, 0, sizeof(httpclient_data_t));

    post_buf = (char *) aliot_platform_malloc(HTTP_POST_MAX_LEN);
    if (NULL == post_buf) {
        log_err("malloc http post buf failed!");
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
        log_err("http message body is too long");
        ret = -1;
        goto do_exit;
    }

    log_debug("http request: \r\n\r\n%s\r\n", post_buf);

    ret = strlen(post_buf);

    response_buf = (char *)aliot_platform_malloc(HTTP_RESP_MAX_LEN);
    if (NULL == response_buf) {
        log_err("malloc http response buf failed!");
        return ERROR_MALLOC;
    }
    memset(response_buf, 0, HTTP_RESP_MAX_LEN);

    httpclient_data.post_content_type = "application/x-www-form-urlencoded;charset=utf-8";
    httpclient_data.post_buf = post_buf;
    httpclient_data.post_buf_len = ret;
    httpclient_data.response_buf = response_buf;
    httpclient_data.response_buf_len = HTTP_RESP_MAX_LEN;

#ifdef _ONLINE

    aliot_post( &httpclient,
                auth_host,
                443,
                aliot_ca_get(),
                10000,
                &httpclient_data);
#else

    aliot_post( &httpclient,
                auth_host,
                80,
                NULL,
                10000,
                &httpclient_data);
#endif

    /*
        {
            "code": 200,
            "data": {
                "iotId":"030VCbn30334364bb36997f44cMYTBAR",
                "iotToken":"e96d15a4d4734a73b13040b1878009bc",
                "resources": {
                    "mqtt": {
                            "host":"iot-as-mqtt.cn-shanghai.aliyuncs.com",
                            "port":1883
                        }
                    }
            },
            "message":"success"
        }
    */
    log_debug("http response: \r\n\r\n%s\r\n", httpclient_data.response_buf);

    //get iot-id and iot-token from response
    int type;
    const char *pvalue, *presrc;
    char port_str[6];

    //get iot-id
    pvalue = LITE_json_value_of("data.iotId", httpclient_data.response_buf);
    if (NULL == pvalue) {
        goto do_exit;
    }
    strcpy(iot_id, pvalue);


    //get iot-token
    pvalue = LITE_json_value_of("data.iotToken", httpclient_data.response_buf);
    if (NULL == pvalue) {
        goto do_exit;
    }
    strcpy(iot_token, pvalue);


    //get host
    pvalue = LITE_json_value_of("data.resources.mqtt.host", httpclient_data.response_buf);
    if (NULL == pvalue) {
        goto do_exit;
    }
    strcpy(host, pvalue);

    //get port
    pvalue = LITE_json_value_of("data.resources.mqtt.port", httpclient_data.response_buf);
    if (NULL == pvalue) {
        goto do_exit;
    }
    strcpy(port_str, pvalue);
    *pport = atoi(port_str);

    log_debug("%10s: %s", "iotId", iot_id);
    log_debug("%10s: %s", "iotToken", iot_token);
    log_debug("%10s: %s", "Host", host);
    log_debug("%10s: %d", "Port", *pport);

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

typedef enum _SECURE_MODE {
    MODE_TLS_TOKEN              = -1,
    MODE_TCP_TOKEN_PLAIN        = 0,
    MODE_TCP_TOKEN_ID2_ENCRPT   = 1,
    MODE_TLS_DIRECT             = 2,
    MODE_TCP_DIRECT_PLAIN       = 3,
    MODE_TCP_DIRECT_ID2_ENCRYPT = 4,
    MODE_TLS_ID2                = 5,
} SECURE_MODE;

/*
    mqtt直接连接，域名＝ ${productkey}.iot-as-mqtt.cn-shanghai.aliyuncs.com
    sign签名和AUTH一致
    mqttClientId = clientId|securemode=0,gw=0,signmethod=hmacmd5,pid=xxx|
    mqttuserName = deviceName&productkey
    mqttPassword = sign

    其中gw＝1代表网关设备，0为普通设备； pid代表合作伙伴id，可选；
    clientId为客户端自标示id不可空，建议使用MAC、SN；
    ||内为扩展参数，域名直连模式下 securemode必须传递，不传递则默认是auth方式。
 */
int32_t aliot_auth(aliot_device_info_pt pdevice_info, aliot_user_info_pt puser_info)
{
    int ret = 0;
    char pid[16];

    memset(pid, 0, sizeof(pid));
    aliot_platform_module_get_pid(pid);

#ifdef DIRECT_MQTT
#define DIRECT_MQTT_DOMAIN  "iot-as-mqtt.cn-shanghai.aliyuncs.com"

    char sign[33];
    char buf[512];
    SECURE_MODE mode;

    puser_info->port = 1883;
    ret = snprintf(puser_info->host_name,
                   sizeof(puser_info->host_name),
#if 0
                   "%s.%s",
                   pdevice_info->product_key,
                   DIRECT_MQTT_DOMAIN
#else
                   "%s",
                   "10.125.63.74"
#endif
                   );
    assert(ret < sizeof(puser_info->host_name));

#ifdef ALIOT_MQTT_TCP
    mode = MODE_TCP_DIRECT_PLAIN;
    puser_info->pubKey = NULL;
#else
    mode = MODE_TLS_DIRECT;
    puser_info->pubKey = aliot_ca_get();
#endif

    ret = snprintf(puser_info->client_id,
                   sizeof(puser_info->client_id),
                   (strlen(pid) ?
                        "%s|securemode=%d,gw=0,signmethod=hmacmd5,pid=%s,timestamp=2524608000000|" :
                        "%s|securemode=%d,gw=0,signmethod=hmacmd5%s,timestamp=2524608000000|"),
                   pdevice_info->device_id,
                   mode,
                   (strlen(pid) ? pid : "")
                   );
    assert(ret < sizeof(puser_info->client_id));

    ret = snprintf(puser_info->user_name,
                   sizeof(puser_info->user_name),
                   "%s&%s",
                   pdevice_info->device_name,
                   pdevice_info->product_key);
    assert(ret < sizeof(puser_info->user_name));

    memset(sign, 0, sizeof(sign));
    memset(buf, 0, sizeof(buf));
    ret = snprintf(buf,
                   sizeof(buf),
                   "clientId%sdeviceName%sproductKey%stimestamp2524608000000",
                   pdevice_info->device_id,
                   pdevice_info->device_name,
                   pdevice_info->product_key);
    assert(ret < sizeof(buf));
    log_debug("sign source=%s (%d)", buf, strlen(buf));

    aliot_hmac_md5(buf, strlen(buf),
                   sign,
                   pdevice_info->device_secret,
                   strlen(pdevice_info->device_secret));

    ret = snprintf(puser_info->password,
                   sizeof(puser_info->password),
                   "%s",
                   sign);
    assert(ret < sizeof(sign));

#else   /* #ifdef DIRECT_MQTT */

    char iot_id[ALIOT_AUTH_IOT_ID + 1], iot_token[ALIOT_AUTH_IOT_TOKEN + 1], host[HOST_ADDRESS_LEN + 1];
    uint16_t port;

    if (0 != aliot_get_id_token(
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
#ifdef ALIOT_MQTT_TCP
    puser_info->pubKey = NULL;
#else
    puser_info->pubKey = aliot_ca_get();
#endif
    if (NULL == puser_info->pubKey) {
        //Append string "nonesecure" if TCP connection be used.
        if (NULL == aliot_platform_module_get_pid(pid)) {
            ret = snprintf(puser_info->client_id,
                       CLIENT_ID_LEN,
                       "%s|securemode=0|",
                       pdevice_info->device_id);
        } else {
            //Append "pid" if we have pid
            ret = snprintf(puser_info->client_id,
                       CLIENT_ID_LEN,
                       "%s|securemode=0,pid=%s|",
                       pdevice_info->device_id,
                       pid);
        }
    } else {
        if (NULL == aliot_platform_module_get_pid(pid)) {
            ret = snprintf(puser_info->client_id,
                                   CLIENT_ID_LEN,
                                   "%s",
                                   pdevice_info->device_id);
        } else {
            //Append "pid" if we have pid
            ret = snprintf(puser_info->client_id,
                       CLIENT_ID_LEN,
                       "%s|pid=%s|",
                       pdevice_info->device_id,
                       pid);
        }
    }

    if (ret >= CLIENT_ID_LEN) {
        log_err("client_id is too long");
    } else if (ret < 0) {
        return -1;
    }
#endif

    return 0;
}

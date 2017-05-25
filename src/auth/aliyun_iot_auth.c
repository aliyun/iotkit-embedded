/*********************************************************************************
 * 文件名称: aliyun_iot_auth.c
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_md5.h"
#include "aliyun_iot_common_hmac.h"
#include "aliyun_iot_common_httpclient.h"
#include "aliyun_iot_common_jsonparser.h"
#include "aliyun_iot_auth.h"
#include "aliyun_iot_platform_memory.h"



const static char *iot_atuh_host = "http://iot-auth-pre.cn-shanghai.aliyuncs.com/auth/devicename";
//const static char *iot_atuh_host = "https://iot-auth.alibaba.net/auth/devicename";

const static char *iot_mqtt_server_ca_crt =  { \
"-----BEGIN CERTIFICATE-----\r\n"                                      \
"MIIEszCCA5ugAwIBAgIGAVrwPKWZMA0GCSqGSIb3DQEBCwUAMIGXMSYwJAYJKoZI\r\n" \
"hvcNAQkBFhdhbGlsYW5nQGFsaWJhYmEtaW5jLmNvbTEaMBgGA1UEAwwRQWxpbGFu\r\n" \
"ZyBTZXJ2ZXIgQ0ExDDAKBgNVBAsMA0luYzEQMA4GA1UECgwHQWxpYmFiYTERMA8G\r\n" \
"A1UEBwwISGFuZ1pob3UxETAPBgNVBAgMCFpoZUppYW5nMQswCQYDVQQGEwJDTjAe\r\n" \
"Fw0xNzAzMjAwOTM5NTBaFw0xOTAzMjAwOTM5NTBaMIGaMSYwJAYJKoZIhvcNAQkB\r\n" \
"FhdhbGlsYW5nQGFsaWJhYmEtaW5jLmNvbTEdMBsGA1UEAwwUaW90LWF1dGguYWxp\r\n" \
"YmFiYS5uZXQxDDAKBgNVBAsMA0luYzEQMA4GA1UECgwHQWxpYmFiYTERMA8GA1UE\r\n" \
"BwwISGFuZ1pob3UxETAPBgNVBAgMCFpoZUppYW5nMQswCQYDVQQGEwJDTjCCASIw\r\n" \
"DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAIpCZxJE3z40aNnBjAl4MIE3+MGx\r\n" \
"qwekC3+PG4JUZP+aWg5UlN7Re0Fw80B0DNuvqvMWsx3qa8obxr09huPhTqZiBRro\r\n" \
"NrSDfbDnL5C5QFu/8LRK6rBiSnMfbu6KK+OUSdHachDAfeozIGK/Pru/g5SV1SLH\r\n" \
"NK2V9bFx3Ma5Hhn/EqikqlIMhIZceyHRkjRKln60Ip95ei6KxoPWSXuHeDXnPAV6\r\n" \
"t+nzbzMzj14JazBXyf+3l/FUXVIR6Es989NJ5LK4LDcX3FYjzroxqau9F0CYyE9W\r\n" \
"D0+XCShgFixZUfT5ImReQmVtU/Kym8gQA/wv+txRqXgnkLWA2yB7j0C0wScCAwEA\r\n" \
"AaOB/zCB/DARBgYpAQECCx4EBwQFNzEyMTMwGQYGKQEBAhUeBA8EDTE0OTAwODkx\r\n" \
"OTA3NzMwEwYDVR0lBAwwCgYIKwYBBQUHAwEwHwYDVR0RBBgwFoIUaW90LWF1dGgu\r\n" \
"YWxpYmFiYS5uZXQwHQYDVR0OBBYEFLUxOu3kvFzb7g3j0CrsWU0It8T6MB8GA1Ud\r\n" \
"IwQYMBaAFBUiAjS7MOQsazZMAwFAIW93KkyWMAwGA1UdDwQFAwMH/4AwSAYIKwYB\r\n" \
"BQUHAQEEPDA6MDgGCCsGAQUFBzABhixodHRwOi8vYWxpbGFuZy5hbGliYWJhLWlu\r\n" \
"Yy5jb20vcnBjL2NlcnQvb2NzcDANBgkqhkiG9w0BAQsFAAOCAQEAUmsDUSz0a/u/\r\n" \
"U21r5aWFZqwjD2Xs/fT+K+83BCQ533uJ4HRz+bHTUnv7ohF+HY8I6loFhreK4Oig\r\n" \
"EUWonJx+DdUEI8I9W0KkbhUfZPvWpDriWPAcEamxKln420LW8mee0XCRU8RUMXVy\r\n" \
"odCibEQCjXld4MxCQeVTgPOKeirtPg83SdsaFdm4KxYvxNbuC6bulZ4H/ZIShthL\r\n" \
"Z9doOJR5GxljzwDNSZ8PcUTbbI8u+QYPLTeSgLM0Vc+/AzIAhxHpw8hYvXZGtTZ8\r\n" \
"OHs8TlwPlsHEh2eZN2RzRM+0CmMT2s3cXrd4E+nFPO2jbBCFQ7zkG1fDXEulKvT9\r\n" \
"UOhKUYndwg==\r\n"                                                     \
"-----END CERTIFICATE-----"
};



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
     
    if (NULL == (buf = aliyun_iot_memory_malloc( length ))) {
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

    post_buf = (char *) aliyun_iot_memory_malloc(HTTP_POST_MAX_LEN);
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
            resources );

    if ((ret < 0) || (ret >= HTTP_POST_MAX_LEN)) {
        ALIOT_LOG_ERROR("http message body is too long");
        ret = -1;
        goto do_exit;
    }

    ALIOT_LOG_DEBUG("http content:%s\n\r", post_buf);

    ret = strlen(post_buf);

    response_buf = (char *) aliyun_iot_memory_malloc(HTTP_RESP_MAX_LEN);
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
            80,
            NULL,
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
                            &type)))
    {
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
                            &type)))
    {
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
                            &type)))
    {
        goto do_exit;
    }

    //get host
    if (NULL == (pvalue = json_get_value_by_fullname(
                            presrc,
                            strlen(presrc),
                            "host",
                            &length,
                            &type)))
    {
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
                            &type)))
    {
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
        aliyun_iot_memory_free(buf);
    }

    if (NULL != post_buf) {
        aliyun_iot_memory_free(post_buf);
    }

    if (NULL != response_buf) {
        aliyun_iot_memory_free(response_buf);
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
                &port))
    {
       return -1;
    }


    strncpy(puser_info->user_name, iot_id, USER_NAME_LEN);
    strncpy(puser_info->password, iot_token, PASSWORD_LEN);
    strncpy(puser_info->host_name, host, HOST_ADDRESS_LEN);
    puser_info->port = port;
    puser_info->pubKey = iot_mqtt_server_ca_crt;
    //puser_info->pubKey = NULL;

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
    } else if (ret < 0){
        return -1;
    }

    return 0;
}

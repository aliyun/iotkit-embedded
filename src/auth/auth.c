#include "guider_internal.h"

#define _ONLINE

#ifdef _ONLINE
    const static char *guider_host = "http://iot-auth.cn-shanghai.aliyuncs.com/auth/devicename";
#else
    const static char *guider_host = "http://iot-auth-pre.cn-shanghai.aliyuncs.com/auth/devicename";
#endif

static int _hmac_md5_signature(
            char *md5_sigbuf,
            const int md5_buflen,
            const char *client_id,
            const char *device_name,
            const char *product_key,
            const char *timestamp_str,
            const char *device_secret)
{
    char                signature[40];
    char                hmac_source[512];
    int                 rc = -1;

    memset(signature, 0, sizeof(signature));
    memset(hmac_source, 0, sizeof(hmac_source));
    rc = snprintf(hmac_source,
                  sizeof(hmac_source),
                  "clientId%s" "deviceName%s" "productKey%s" "timestamp%s",
                  client_id,
                  device_name,
                  product_key,
                  timestamp_str ? timestamp_str : GUIDER_DEFAULT_TS_STR);
    assert(rc < sizeof(hmac_source));
    log_debug("| source: %s (%d)", hmac_source, strlen(hmac_source));

    utils_hmac_md5(hmac_source, strlen(hmac_source),
                   signature,
                   device_secret,
                   strlen(device_secret));
    log_debug("| signature: %s (%d)", signature, strlen(signature));

    memcpy(md5_sigbuf, signature, md5_buflen);
    return 0;
}

static int iotx_get_id_token(
            const char *auth_host,
            const char *request_string,
            char *iot_id,
            char *iot_token,
            char *host,
            uint16_t *pport)
{
#define SIGN_SOURCE_LEN     (256)
#define HTTP_POST_MAX_LEN   (1024)
#define HTTP_RESP_MAX_LEN   (1024)

    int ret = -1;
    char *post_buf = NULL, *response_buf = NULL;

    httpclient_t httpclient;
    httpclient_data_t httpclient_data;

    memset(&httpclient, 0, sizeof(httpclient_t));
    httpclient.header = "Accept: text/xml,text/javascript,text/html,application/json\r\n";

    memset(&httpclient_data, 0, sizeof(httpclient_data_t));

    post_buf = (char *) LITE_malloc(HTTP_POST_MAX_LEN);
    if (NULL == post_buf) {
        log_err("malloc http post buf failed!");
        return ERROR_MALLOC;
    }
    memset(post_buf, 0, HTTP_POST_MAX_LEN);

    ret = snprintf(post_buf,
                   HTTP_POST_MAX_LEN,
                   "%s",
                   request_string);

    if ((ret < 0) || (ret >= HTTP_POST_MAX_LEN)) {
        log_err("http message body is too long");
        ret = -1;
        goto do_exit;
    }

    log_debug("http request: \r\n\r\n%s\r\n", post_buf);

    ret = strlen(post_buf);

    response_buf = (char *)LITE_malloc(HTTP_RESP_MAX_LEN);
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

    iotx_post(&httpclient,
              auth_host,
#ifdef IOTX_MQTT_TCP
              80,
#else
              443,
#endif
              iotx_ca_get(),
              10000,
              &httpclient_data);
#else

    iotx_post(&httpclient,
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
    LITE_free(pvalue);

    //get iot-token
    pvalue = LITE_json_value_of("data.iotToken", httpclient_data.response_buf);
    if (NULL == pvalue) {
        goto do_exit;
    }
    strcpy(iot_token, pvalue);
    LITE_free(pvalue);

    //get host
    pvalue = LITE_json_value_of("data.resources.mqtt.host", httpclient_data.response_buf);
    if (NULL == pvalue) {
        goto do_exit;
    }
    strcpy(host, pvalue);
    LITE_free(pvalue);

    //get port
    pvalue = LITE_json_value_of("data.resources.mqtt.port", httpclient_data.response_buf);
    if (NULL == pvalue) {
        goto do_exit;
    }
    strcpy(port_str, pvalue);
    LITE_free(pvalue);
    *pport = atoi(port_str);

    log_debug("%10s: %s", "iotId", iot_id);
    log_debug("%10s: %s", "iotToken", iot_token);
    log_debug("%10s: %s", "Host", host);
    log_debug("%10s: %d", "Port", *pport);

    ret = 0;

do_exit:
    if (NULL != post_buf) {
        LITE_free(post_buf);
    }

    if (NULL != response_buf) {
        LITE_free(response_buf);
    }

    return ret;
}

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
typedef enum _SECURE_MODE {
    MODE_TLS_TOKEN              = -1,
    MODE_TCP_TOKEN_PLAIN        = 0,
    MODE_TCP_TOKEN_ID2_ENCRPT   = 1,
    MODE_TLS_DIRECT             = 2,
    MODE_TCP_DIRECT_PLAIN       = 3,
    MODE_TCP_DIRECT_ID2_ENCRYPT = 4,
    MODE_TLS_ID2                = 5,
} SECURE_MODE;

static SECURE_MODE _guider_secure_mode(void)
{
    int             rc = -1;

#ifdef DIRECT_MQTT

#ifdef IOTX_MQTT_TCP
    rc = MODE_TCP_DIRECT_PLAIN;
#else
    rc = MODE_TLS_DIRECT;
#endif

#else // DIRECT_MQTT
    rc = MODE_TCP_TOKEN_PLAIN;
#endif
    return  rc;
}

static void _authenticate_url(char *buf, int len)
{
#ifdef DIRECT_MQTT
    snprintf(buf, len, "%s", "");
#else

    snprintf(buf, len, "%s", "http://");
#ifdef _ONLINE
    strcat(buf, "iot-auth.cn-shanghai.aliyuncs.com");
#else
    strcat(buf, "iot-auth-pre.cn-shanghai.aliyuncs.com");
#endif
    strcat(buf, "/auth/devicename");

#endif  // DIRECT_MQTT

    return;
}

// TODO: -1 ?
const char **secmode_str[] = {
    "TCP + Guider + Plain",
    "TCP + Guider + ID2-Crypto",
    "TLS + Direct",
    "TCP + Direct + Plain",
    "TCP + Direct + ID2-Crypto",
    "TLS + Guider + ID2-Authenticate",
    ""
};

/*
    struct {
        char            host_name[HOST_ADDRESS_LEN + 1];
        uint16_t        port;
        char            user_name[USER_NAME_LEN + 1];
        char            password[PASSWORD_LEN + 1];
        char            client_id[CLIENT_ID_LEN + 1];
        const char *    pubKey;
    }
*/
int32_t iotx_auth(iotx_device_info_pt pdevice_info, iotx_user_info_pt puser_info)
{
    char            conn_host[CONN_HOST_LEN] = {0};
    char            conn_port[CONN_PORT_LEN] = {0};
    char            conn_usr[CONN_USR_LEN] = {0};
    char            conn_pwd[CONN_PWD_LEN] = {0};
    char            conn_cid[CONN_CID_LEN] = {0};

    char            guider_pid_buf[GUIDER_PID_LEN + 16] = {0};
    const char     *guider_ca_ptr = NULL;
    char            guider_url[GUIDER_URL_LEN] = {0};
    int             guider_portnum = -1;
    SECURE_MODE     guider_secmode = 0;
    char            guider_sign[GUIDER_SIGN_LEN] = {0};
    char            guider_timestamp_str[GUIDER_TS_LEN] = {0};

    int             ret = -1;
    char            partner_id[16];

    memset(partner_id, 0, sizeof(partner_id));
    HAL_GetPartnerID(partner_id);
    if (strlen(partner_id)) {
        snprintf(guider_pid_buf, sizeof(guider_pid_buf), "partner_id=%s", partner_id);
    } else {
        strcpy(guider_pid_buf, "");
    }

    _authenticate_url(guider_url, sizeof(guider_url));
#ifdef IOTX_MQTT_TCP
    guider_ca_ptr = NULL;
    guider_portnum = 80;
#else
    guider_ca_ptr = iotx_ca_get();
    guider_portnum = 443;
#endif
    guider_secmode = _guider_secure_mode();

#ifdef EQUIP_ID2
    // fetch from network
#else
    snprintf(guider_timestamp_str, sizeof(guider_timestamp_str), "%s", GUIDER_DEFAULT_TS_STR);
#endif

#ifdef EQUIP_ID2
    // get ID2 + DeviceCode + Signature
#else
    _hmac_md5_signature(guider_sign, sizeof(guider_sign),
                        pdevice_info->device_id,
                        pdevice_info->device_name,
                        pdevice_info->product_key,
                        guider_timestamp_str,
                        pdevice_info->device_secret);
#endif

    log_debug("%s", "....................................................");
    log_debug("%20s : %-s", "ProductKey", pdevice_info->product_key);
    log_debug("%20s : %-s", "DeviceName", pdevice_info->device_name);
    log_debug("%20s : %-s", "DeviceID", pdevice_info->device_id);
    log_debug("%20s : %-s", "DeviceSecret", pdevice_info->device_secret);
    log_debug("%s", "....................................................");
    log_debug("%20s : %-s", "PartnerID Buf", guider_pid_buf);
    log_debug("%20s : %p ('%.16s' ...)", "TLS CA", guider_ca_ptr, guider_ca_ptr);
    log_debug("%20s : %s", "Guider URL", guider_url);
    log_debug("%20s : %d", "Guider Port", guider_portnum);
    log_debug("%20s : %d (%s)", "Guider SecMode", guider_secmode, secmode_str[guider_secmode]);
    log_debug("%20s : %s", "Guider Timestamp", guider_timestamp_str);
    log_debug("%s", "....................................................");
    log_debug("%20s : %s", "Guider Sign", guider_sign);
    log_debug("%s", "....................................................");

#ifdef DIRECT_MQTT
#define DIRECT_MQTT_DOMAIN  "iot-as-mqtt.cn-shanghai.aliyuncs.com"

    char            iotx_signature[33];
    char            iotx_hmac_source[512];
    SECURE_MODE     iotx_secmode;

#ifdef IOTX_MQTT_TCP
    iotx_secmode = MODE_TCP_DIRECT_PLAIN;
    puser_info->pubKey = NULL;
#else
    iotx_secmode = MODE_TLS_DIRECT;
    puser_info->pubKey = iotx_ca_get();
#endif

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
    puser_info->port = 1883;

    ret = snprintf(puser_info->client_id,
                   sizeof(puser_info->client_id),
                   (strlen(partner_id) ?
                    "%s|securemode=%d,gw=0,signmethod=hmacmd5,partner_id=%s,timestamp=2524608000000|" :
                    "%s|securemode=%d,gw=0,signmethod=hmacmd5%s,timestamp=2524608000000|"),
                   pdevice_info->device_id,
                   iotx_secmode,
                   (strlen(partner_id) ? partner_id : "")
                  );
    assert(ret < sizeof(puser_info->client_id));

    ret = snprintf(puser_info->user_name,
                   sizeof(puser_info->user_name),
                   "%s&%s",
                   pdevice_info->device_name,
                   pdevice_info->product_key);
    assert(ret < sizeof(puser_info->user_name));

    ret = _hmac_md5_signature(iotx_signature, sizeof(iotx_signature),
                              pdevice_info->device_id,
                              pdevice_info->device_name,
                              pdevice_info->product_key,
                              0,
                              pdevice_info->device_secret);
    log_debug("iotx_signature : %s", iotx_signature);

    ret = snprintf(puser_info->password,
                   sizeof(puser_info->password),
                   "%s",
                   iotx_signature);
    assert(ret <= strlen(iotx_signature));

#else   /* #ifdef DIRECT_MQTT */

    char iot_id[GUIDER_IOT_ID_LEN + 1], iot_token[GUIDER_IOT_TOKEN_LEN + 1], host[HOST_ADDRESS_LEN + 1];
    uint16_t port;

    char        sign[40] = {0};

    _hmac_md5_signature(sign, sizeof(sign),
            pdevice_info->device_id,
            pdevice_info->device_name,
            pdevice_info->product_key,
            0,
            pdevice_info->device_secret);
    log_debug("sign = %.64s", sign);

    char       *reqstr = NULL;

    ret = asprintf(&reqstr,
            "productKey=%s&" "deviceName=%s&" "sign=%s&"
            "version=default&" "clientId=%s&" "timestamp=%s&" "resources=mqtt",
            pdevice_info->product_key,
            pdevice_info->device_name,
            sign,
            pdevice_info->device_id,
            GUIDER_DEFAULT_TS_STR);

    if (0 != iotx_get_id_token(
                    guider_host,
                    reqstr,
                    iot_id,
                    iot_token,
                    host,
                    &port)) {
        free(reqstr);
        return -1;
    }


    strncpy(puser_info->user_name, iot_id, USER_NAME_LEN);
    strncpy(puser_info->password, iot_token, PASSWORD_LEN);
    strncpy(puser_info->host_name, host, HOST_ADDRESS_LEN);
    puser_info->port = port;
#ifdef IOTX_MQTT_TCP
    puser_info->pubKey = NULL;
#else
    puser_info->pubKey = iotx_ca_get();
#endif
    if (NULL == puser_info->pubKey) {
        //Append string "nonesecure" if TCP connection be used.
        if (NULL == HAL_GetPartnerID(partner_id)) {
            ret = snprintf(puser_info->client_id,
                           CLIENT_ID_LEN,
                           "%s|securemode=0|",
                           pdevice_info->device_id);
        } else {
            //Append "partner_id" if we have partner_id
            ret = snprintf(puser_info->client_id,
                           CLIENT_ID_LEN,
                           "%s|securemode=0,partner_id=%s|",
                           pdevice_info->device_id,
                           partner_id);
        }
    } else {
        if (NULL == HAL_GetPartnerID(partner_id)) {
            ret = snprintf(puser_info->client_id,
                           CLIENT_ID_LEN,
                           "%s",
                           pdevice_info->device_id);
        } else {
            //Append "partner_id" if we have partner_id
            ret = snprintf(puser_info->client_id,
                           CLIENT_ID_LEN,
                           "%s|partner_id=%s|",
                           pdevice_info->device_id,
                           partner_id);
        }
    }

    if (ret >= CLIENT_ID_LEN) {
        log_err("client_id is too long");
    } else if (ret < 0) {
        free(reqstr);
        return -1;
    }
#endif

    log_debug("%s", "-----------------------------------------");
    log_debug("%16s : %-s", "Host", conn_host);
    log_debug("%16s : %-s", "Port", conn_port);
    log_debug("%16s : %-s", "UserName", conn_usr);
    log_debug("%16s : %-s", "PassWord", conn_pwd);
    log_debug("%16s : %-s", "ClientID", conn_cid);
    log_debug("%s", "-----------------------------------------");

    free(reqstr);
    return 0;
}

#include "guider_internal.h"

#define _ONLINE

typedef enum _SECURE_MODE {
    MODE_TLS_GUIDER             = -1,
    MODE_TCP_GUIDER_PLAIN       = 0,
    MODE_TCP_GUIDER_ID2_ENCRPT  = 1,
    MODE_TLS_DIRECT             = 2,
    MODE_TCP_DIRECT_PLAIN       = 3,
    MODE_TCP_DIRECT_ID2_ENCRYPT = 4,
    MODE_TLS_ID2                = 5,
} SECURE_MODE;

const char **secmode_str[] = {
    "TCP + Guider + Plain",
    "TCP + Guider + ID2-Crypto",
    "TLS + Direct",
    "TCP + Direct + Plain",
    "TCP + Direct + ID2-Crypto",
    "TLS + Guider + ID2-Authenticate",
    ""
};

static int _hmac_md5_signature(
            char *md5_sigbuf,
            const int md5_buflen,
            const char *timestamp_str)
{
    char                    signature[40];
    char                    hmac_source[512];
    int                     rc = -1;
    iotx_device_info_pt     dev;

    dev = iotx_get_device_info();
    assert(dev);

    memset(signature, 0, sizeof(signature));
    memset(hmac_source, 0, sizeof(hmac_source));
    rc = snprintf(hmac_source,
                  sizeof(hmac_source),
                  "clientId%s" "deviceName%s" "productKey%s" "timestamp%s",
                  dev->device_id,
                  dev->device_name,
                  dev->product_key,
                  timestamp_str);
    assert(rc < sizeof(hmac_source));
    log_debug("| source: %s (%d)", hmac_source, strlen(hmac_source));

    utils_hmac_md5(hmac_source, strlen(hmac_source),
                   signature,
                   dev->device_secret,
                   strlen(dev->device_secret));
    log_debug("| signature: %s (%d)", signature, strlen(signature));

    memcpy(md5_sigbuf, signature, md5_buflen);
    return 0;
}

static int _http_response(char *payload,
                          const int payload_len,
                          const char *request_string,
                          const char *url,
                          const int port_num,
                          const char *pkey
                         )
{
#define HTTP_POST_MAX_LEN   (1024)
#define HTTP_RESP_MAX_LEN   (1024)

    int                     ret = -1;
    char                   *requ_payload = NULL;
    char                   *resp_payload = NULL;

    httpclient_t            httpc;
    httpclient_data_t       httpc_data;

    memset(&httpc, 0, sizeof(httpclient_t));
    memset(&httpc_data, 0, sizeof(httpclient_data_t));

    httpc.header = "Accept: text/xml,text/javascript,text/html,application/json\r\n";

    requ_payload = (char *)LITE_malloc(HTTP_POST_MAX_LEN);
    if (NULL == requ_payload) {
        log_err("Allocate HTTP request buf failed!");
        return ERROR_MALLOC;
    }
    memset(requ_payload, 0, HTTP_POST_MAX_LEN);

    ret = snprintf(requ_payload,
                   HTTP_POST_MAX_LEN,
                   "%s",
                   request_string);
    assert(ret < HTTP_POST_MAX_LEN);
    log_debug("requ_payload: \r\n\r\n%s\r\n", requ_payload);

    resp_payload = (char *)LITE_malloc(HTTP_RESP_MAX_LEN);
    assert(resp_payload);
    memset(resp_payload, 0, HTTP_RESP_MAX_LEN);

    httpc_data.post_content_type = "application/x-www-form-urlencoded;charset=utf-8";
    httpc_data.post_buf = requ_payload;
    httpc_data.post_buf_len = strlen(requ_payload);
    httpc_data.response_buf = resp_payload;
    httpc_data.response_buf_len = HTTP_RESP_MAX_LEN;

    httpclient_common(&httpc,
                      url,
                      port_num,
                      pkey,
                      HTTPCLIENT_POST,
                      10000,
                      &httpc_data);

    memcpy(payload, httpc_data.response_buf, payload_len);
    log_debug("PAYLOAD: %s", payload);

    LITE_free(requ_payload);
    LITE_free(resp_payload);

    return 0;
}

static int _iotId_iotToken_http(
            const char *guider_addr,
            const char *request_string,
            char *iot_id,
            char *iot_token,
            char *host,
            uint16_t *pport)
{
    char            iotx_payload[512] = {0};
    int             iotx_port = 443;
    int             ret = -1;

#ifdef IOTX_MQTT_TCP
    iotx_port = 80;
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
    _http_response(iotx_payload,
                   sizeof(iotx_payload),
                   request_string,
                   guider_addr,
                   iotx_port,
                   iotx_ca_get());
    log_debug("http response: \r\n\r\n%s\r\n", iotx_payload);

    const char         *pvalue;
    char                port_str[6];

    pvalue = LITE_json_value_of("data.iotId", iotx_payload);
    if (NULL == pvalue) {
        goto do_exit;
    }
    strcpy(iot_id, pvalue);
    LITE_free(pvalue);

    pvalue = LITE_json_value_of("data.iotToken", iotx_payload);
    if (NULL == pvalue) {
        goto do_exit;
    }
    strcpy(iot_token, pvalue);
    LITE_free(pvalue);

    pvalue = LITE_json_value_of("data.resources.mqtt.host", iotx_payload);
    if (NULL == pvalue) {
        goto do_exit;
    }
    strcpy(host, pvalue);
    LITE_free(pvalue);

    pvalue = LITE_json_value_of("data.resources.mqtt.port", iotx_payload);
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
    return ret;
}

void _timestamp_string(char *buf, int len)
{
#ifdef EQUIP_ID2
    // fetch from network
#else
    snprintf(buf, len, "%s", GUIDER_DEFAULT_TS_STR);
#endif
    return;
}

static SECURE_MODE _secure_mode_num(void)
{
    int             rc = -1;

#ifdef DIRECT_MQTT

#ifdef IOTX_MQTT_TCP
    rc = MODE_TCP_DIRECT_PLAIN;
#else
    rc = MODE_TLS_DIRECT;
#endif

#else // DIRECT_MQTT
    rc = MODE_TCP_GUIDER_PLAIN;
#endif
    return  rc;
}

void _secure_mode_str(char *buf, int len)
{
    memset(buf, 0, len);
#ifndef MQTT_TCP
    snprintf(buf, len, "");
#else
    snprintf(buf, len, "securemode=0");
#endif
    return;
}

void _ident_partner(char *buf, int len)
{
    char                tmp[GUIDER_PID_LEN] = {0};

    memset(tmp, 0, sizeof(tmp));
    HAL_GetPartnerID(tmp);
    if (strlen(tmp)) {
        snprintf(buf, len, ",partner_id=%s", tmp);
    } else {
        strcpy(buf, "");
    }

    return;
}

static void _authenticate_http_url(char *buf, int len)
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

char *_authenticate_string(char sign[], char ts[])
{
    char                   *ret = NULL;
    iotx_device_info_pt     dev = NULL;

    dev = iotx_get_device_info();
    assert(dev);

    asprintf(&ret,
             "productKey=%s&" "deviceName=%s&" "sign=%s&"
             "version=default&" "clientId=%s&" "timestamp=%s&" "resources=mqtt",
             dev->product_key,
             dev->device_name,
             sign,
             dev->device_id,
             ts);

    return ret;
}

int _fill_conn_string(char *dst, int len, const char *fmt, ...)
{
    int                     rc = -1;
    va_list                 ap;
    char                   *ptr = NULL;

    va_start(ap, fmt);
    rc = vsnprintf(dst, len, fmt, ap);
    va_end(ap);
    assert(rc <= len);

    ptr = strstr(dst, "||");
    if (ptr) {
        *ptr = '\0';
    }

    // log_debug("dst(%d) = %s", rc, dst);
    return 0;
}

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
int32_t iotx_guider(iotx_device_info_pt dev, iotx_user_info_pt usr)
{
    char            guider_pid_buf[GUIDER_PID_LEN + 16] = {0};
    char            guider_url[GUIDER_URL_LEN] = {0};
    int             guider_portnum = 443;
    SECURE_MODE     guider_secmode_num = 0;
    char            guider_secmode_str[CONN_SECMODE_LEN] = {0};
    char            guider_sign[GUIDER_SIGN_LEN] = {0};
    char            guider_timestamp_str[GUIDER_TS_LEN] = {0};

    char           *req_str = NULL;
    int             ret = -1;

    _secure_mode_str(guider_secmode_str, sizeof(guider_secmode_str));

    _timestamp_string(guider_timestamp_str, sizeof(guider_timestamp_str));
    _ident_partner(guider_pid_buf, sizeof(guider_pid_buf));
    _authenticate_http_url(guider_url, sizeof(guider_url));
#ifdef IOTX_MQTT_TCP
    guider_portnum = 80;
#endif
    guider_secmode_num = _secure_mode_num();

#ifdef EQUIP_ID2
    // get ID2 Signature, deviceCode fetched meanwhile
#else
    _hmac_md5_signature(guider_sign, sizeof(guider_sign),
                        guider_timestamp_str);
#endif

    log_debug("%s", "....................................................");
    log_debug("%20s : %-s", "ProductKey", dev->product_key);
    log_debug("%20s : %-s", "DeviceName", dev->device_name);
    log_debug("%20s : %-s", "DeviceID", dev->device_id);
    log_debug("%20s : %-s", "DeviceSecret", dev->device_secret);
    log_debug("%s", "....................................................");
    log_debug("%20s : %-s", "PartnerID Buf", guider_pid_buf);
    log_debug("%20s : %s", "Guider URL", guider_url);
    log_debug("%20s : %d", "Guider Port", guider_portnum);
    log_debug("%20s : %d (%s)", "Guider SecMode", guider_secmode_num, secmode_str[guider_secmode_num]);
    log_debug("%20s : %s", "Guider Timestamp", guider_timestamp_str);
    log_debug("%s", "....................................................");
    log_debug("%20s : %s", "Guider Sign", guider_sign);
    log_debug("%s", "....................................................");

#ifndef DIRECT_MQTT
    char            iotx_conn_host[HOST_ADDRESS_LEN + 1] = {0};
    uint16_t        iotx_conn_port = 1883;
    char            iotx_id[GUIDER_IOT_ID_LEN + 1] = {0};
    char            iotx_token[GUIDER_IOT_TOKEN_LEN + 1] = {0};

    req_str = _authenticate_string(guider_sign, guider_timestamp_str);
    assert(req_str);
    log_debug("req_str = '%s'", req_str);

    if (0 != _iotId_iotToken_http(guider_url,
                                  req_str,
                                  iotx_id,
                                  iotx_token,
                                  iotx_conn_host,
                                  &iotx_conn_port)) {
        if (req_str) {
            free(req_str);
        }
        return -1;
    }
#endif

    /* Start Filling Connection Information */
    usr->pubKey = iotx_ca_get();
#ifdef DIRECT_MQTT

    usr->port = 1883;
#if 0
    _fill_conn_string(usr->host_name, "%s.%s",
                      dev->product_key,
                      GUIDER_DIRECT_DOMAIN);
#else
    _fill_conn_string(usr->host_name, sizeof(usr->host_name), "%s", "10.125.63.74");
#endif
    _fill_conn_string(usr->user_name, sizeof(usr->user_name), "%s&%s",
                      dev->device_name,
                      dev->product_key);
    _fill_conn_string(usr->password, sizeof(usr->password), "%s", guider_sign);
    _fill_conn_string(usr->client_id, sizeof(usr->client_id),
                      "%s"
                      "|securemode=%d,gw=0,signmethod=hmacmd5"
                      "%s,timestamp=%s|",
                      dev->device_id,
                      guider_secmode_num,
                      guider_pid_buf, guider_timestamp_str);

#else   /* DIRECT_MQTT */

    usr->port = iotx_conn_port;
    _fill_conn_string(usr->host_name, sizeof(usr->host_name), "%s", iotx_conn_host);
    _fill_conn_string(usr->user_name, sizeof(usr->user_name), "%s", iotx_id);
    _fill_conn_string(usr->password, sizeof(usr->password), "%s", iotx_token);
    _fill_conn_string(usr->client_id, sizeof(usr->client_id),
                      "%s" "|%s" "%s|",
                      dev->device_id,
                      guider_secmode_str, guider_pid_buf);

#endif  /* DIRECT_MQTT */

    log_debug("%s", "-----------------------------------------");
    log_debug("%16s : %-s", "Host", usr->host_name);
    log_debug("%16s : %d",  "Port", usr->port);
    log_debug("%16s : %-s", "UserName", usr->user_name);
    log_debug("%16s : %-s", "PassWord", usr->password);
    log_debug("%16s : %-s", "ClientID", usr->client_id);
    log_debug("%16s : %p ('%.16s ...')", "TLS PubKey",
              usr->pubKey ? usr->pubKey : 0xdead,
              usr->pubKey ? usr->pubKey : "N/A");
    log_debug("%s", "-----------------------------------------");

    if (req_str) {
        free(req_str);
    }

    return 0;
}

#include <stdio.h>
#include <string.h>
#include "infra_defs.h"
#include "infra_config.h"
#include "infra_string.h"
#include "infra_sha256.h"
#include "dev_sign_api.h"
#include "dev_sign_wrapper.h"

#define SIGN_MQTT_PORT (1883)

typedef enum {
    MODE_TLS_DIRECT             = 2,
    MODE_TCP_DIRECT_PLAIN       = 3,
} secure_mode_t;

static secure_mode_t _get_secure_mode(void)
{
    secure_mode_t res = MODE_TCP_DIRECT_PLAIN;

#ifdef SUPPORT_TLS
    res = MODE_TLS_DIRECT;
#else
    res = MODE_TCP_DIRECT_PLAIN;
#endif

    return  res;
}

#ifndef DEVICE_MODEL_ENABLE
int32_t IOT_Sign_MQTT(iotx_mqtt_region_types_t region, iotx_dev_meta_info_t *meta, iotx_sign_mqtt_t *signout)
{
    uint16_t length = 0;
    char device_id[IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 1] = {0};
    char timestamp[20] = {0};
    char timp_intstr[20] = {0};

    char signsource[DEV_SIGN_SOURCE_MAXLEN] = {0};
    uint16_t signsource_len = 0;
    uint8_t sign[32] = {0};
    const char *sign_fmt = "clientId%sdeviceName%sproductKey%stimestamp%s";
    const char *clientid_fmt = "%s|securemode=%d,timestamp=%s,signmethod=hmacsha256,gw=%d,ext=%d,ver=c-sdk-%s|";

    if (region >= IOTX_MQTT_DOMAIN_NUMBER || meta == NULL) {
        return -1;
    }

    memset(signout,0,sizeof(iotx_sign_mqtt_t));

    memcpy(device_id,meta->product_key,strlen(meta->product_key));
    memcpy(device_id+strlen(device_id),".",strlen("."));
    memcpy(device_id+strlen(device_id),meta->device_name,strlen(meta->device_name));

    memcpy(timestamp,"2524608000000",strlen("2524608000000"));


    signsource_len = strlen(sign_fmt) + strlen(device_id) + strlen(meta->device_name) + strlen(meta->product_key) + strlen(timestamp) + 1;
    if (signsource_len >= DEV_SIGN_SOURCE_MAXLEN) {
        return ERROR_DEV_SIGN_SOURCE_TOO_SHORT;
    }
    memset(signsource,0,DEV_SIGN_SOURCE_MAXLEN);
    memcpy(signsource,"clientId",strlen("clientId"));
    memcpy(signsource+strlen(signsource),device_id,strlen(device_id));
    memcpy(signsource+strlen(signsource),"deviceName",strlen("deviceName"));
    memcpy(signsource+strlen(signsource),meta->device_name,strlen(meta->device_name));
    memcpy(signsource+strlen(signsource),"productKey",strlen("productKey"));
    memcpy(signsource+strlen(signsource),meta->product_key,strlen(meta->product_key));
    memcpy(signsource+strlen(signsource),"timestamp",strlen("timestamp"));
    memcpy(signsource+strlen(signsource),timestamp,strlen(timestamp));

    utils_hmac_sha256((uint8_t *)signsource,strlen(signsource),(uint8_t *)meta->device_secret,strlen(meta->device_secret),sign);

    /* Get Sign Information For MQTT */
    length = strlen(meta->product_key) + strlen(g_infra_mqtt_domain[region].region) + 2;
    if (length >= DEV_SIGN_HOSTNAME_MAXLEN) {
        return ERROR_DEV_SIGN_HOST_NAME_TOO_SHORT;
    }

    memset(signout->hostname,0,DEV_SIGN_HOSTNAME_MAXLEN);
    memcpy(signout->hostname,meta->product_key,strlen(meta->product_key));
    memcpy(signout->hostname+strlen(signout->hostname),".",strlen("."));
    memcpy(signout->hostname+strlen(signout->hostname),g_infra_mqtt_domain[region].region,strlen(g_infra_mqtt_domain[region].region));

    length = strlen(meta->device_name) + strlen(meta->product_key) + 2;
    if (length >= DEV_SIGN_USERNAME_MAXLEN) {
        return ERROR_DEV_SIGN_USERNAME_TOO_SHORT;
    }

    memset(signout->username,0,DEV_SIGN_USERNAME_MAXLEN);
    memcpy(signout->username,meta->device_name,strlen(meta->device_name));
    memcpy(signout->username+strlen(signout->username),"&",strlen("&"));
    memcpy(signout->username+strlen(signout->username),meta->product_key,strlen(meta->product_key));

    memset(signout->password,0,DEV_SIGN_PASSWORD_MAXLEN);
    infra_hex2str(sign,32,signout->password);

    length = strlen(clientid_fmt) + strlen(device_id) + strlen(timestamp) + strlen(IOTX_SDK_VERSION) + 30 + 1;
    if (length >= DEV_SIGN_CLIENT_ID_MAXLEN) {
        return ERROR_DEV_SIGN_CLIENT_ID_TOO_SHORT;
    }

    memset(signout->clientid,0,DEV_SIGN_CLIENT_ID_MAXLEN);
    memcpy(signout->clientid,device_id,strlen(device_id));
    memcpy(signout->clientid+strlen(signout->clientid),"|securemode=",strlen("|securemode="));
    memset(timp_intstr,0,20);
    infra_int2str(_get_secure_mode(),timp_intstr);
    memcpy(signout->clientid+strlen(signout->clientid),timp_intstr,strlen(timp_intstr));
    memcpy(signout->clientid+strlen(signout->clientid),",timestamp=",strlen(",timestamp="));
    memcpy(signout->clientid+strlen(signout->clientid),timestamp,strlen(timestamp));
    memcpy(signout->clientid+strlen(signout->clientid),",signmethod=hmacsha256,gw=0,ext=0,ver=c-sdk-",strlen(",signmethod=hmacsha256,gw=0,ext=0,ver=c-sdk-"));
    memcpy(signout->clientid+strlen(signout->clientid),IOTX_SDK_VERSION,strlen(IOTX_SDK_VERSION));
    memcpy(signout->clientid+strlen(signout->clientid),"|",strlen("|"));

    signout->port = g_infra_mqtt_domain[region].port;

    return SUCCESS_RETURN;
}

#else
int32_t IOT_Sign_MQTT(iotx_mqtt_region_types_t region, iotx_dev_meta_info_t *meta, iotx_sign_mqtt_t *signout)
{
    uint16_t length = 0;
    char device_id[IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 1] = {0};
    char timestamp[20] = {0};
    char timp_intstr[20] = {0};

    char signsource[DEV_SIGN_SOURCE_MAXLEN] = {0};
    uint16_t signsource_len = 0;
    uint8_t sign[32] = {0};
    const char *sign_fmt = "clientId%sdeviceName%sproductKey%stimestamp%s";
    const char *clientid_fmt = "%s|securemode=%d,timestamp=%s,signmethod=hmacsha256,ver=c-sdk-%s|";
    const char *alink_version = ",v="IOTX_ALINK_VERSION"|";

    if (region >= IOTX_MQTT_DOMAIN_NUMBER || meta == NULL) {
        return -1;
    }

    memset(signout,0,sizeof(iotx_sign_mqtt_t));

    memcpy(device_id,meta->product_key,strlen(meta->product_key));
    memcpy(device_id+strlen(device_id),".",strlen("."));
    memcpy(device_id+strlen(device_id),meta->device_name,strlen(meta->device_name));

    memcpy(timestamp,"2524608000000",strlen("2524608000000"));


    signsource_len = strlen(sign_fmt) + strlen(device_id) + strlen(meta->device_name) + strlen(meta->product_key) + strlen(timestamp) + 1;
    if (signsource_len >= DEV_SIGN_SOURCE_MAXLEN) {
        return ERROR_DEV_SIGN_SOURCE_TOO_SHORT;
    }
    memset(signsource,0,DEV_SIGN_SOURCE_MAXLEN);
    memcpy(signsource,"clientId",strlen("clientId"));
    memcpy(signsource+strlen(signsource),device_id,strlen(device_id));
    memcpy(signsource+strlen(signsource),"deviceName",strlen("deviceName"));
    memcpy(signsource+strlen(signsource),meta->device_name,strlen(meta->device_name));
    memcpy(signsource+strlen(signsource),"productKey",strlen("productKey"));
    memcpy(signsource+strlen(signsource),meta->product_key,strlen(meta->product_key));
    memcpy(signsource+strlen(signsource),"timestamp",strlen("timestamp"));
    memcpy(signsource+strlen(signsource),timestamp,strlen(timestamp));

    utils_hmac_sha256((uint8_t *)signsource,strlen(signsource),(uint8_t *)meta->device_secret,strlen(meta->device_secret),sign);

    /* Get Sign Information For MQTT */
    length = strlen(meta->product_key) + strlen(g_infra_mqtt_domain[region].region) + 2;
    if (length >= DEV_SIGN_HOSTNAME_MAXLEN) {
        return ERROR_DEV_SIGN_HOST_NAME_TOO_SHORT;
    }

    memset(signout->hostname,0,DEV_SIGN_HOSTNAME_MAXLEN);
    memcpy(signout->hostname,meta->product_key,strlen(meta->product_key));
    memcpy(signout->hostname+strlen(signout->hostname),".",strlen("."));
    memcpy(signout->hostname+strlen(signout->hostname),g_infra_mqtt_domain[region].region,strlen(g_infra_mqtt_domain[region].region));

    length = strlen(meta->device_name) + strlen(meta->product_key) + 2;
    if (length >= DEV_SIGN_USERNAME_MAXLEN) {
        return ERROR_DEV_SIGN_USERNAME_TOO_SHORT;
    }

    memset(signout->username,0,DEV_SIGN_USERNAME_MAXLEN);
    memcpy(signout->username,meta->device_name,strlen(meta->device_name));
    memcpy(signout->username+strlen(signout->username),"&",strlen("&"));
    memcpy(signout->username+strlen(signout->username),meta->product_key,strlen(meta->product_key));

    memset(signout->password,0,DEV_SIGN_PASSWORD_MAXLEN);
    infra_hex2str(sign,32,signout->password);

    length = strlen(clientid_fmt) + strlen(device_id) + strlen(timestamp) + strlen(IOTX_SDK_VERSION) + 30 + 1;
    length += strlen(alink_version);
    if (length >= DEV_SIGN_CLIENT_ID_MAXLEN) {
        return ERROR_DEV_SIGN_CLIENT_ID_TOO_SHORT;
    }

    memset(signout->clientid,0,DEV_SIGN_CLIENT_ID_MAXLEN);
    memcpy(signout->clientid,device_id,strlen(device_id));
    memcpy(signout->clientid+strlen(signout->clientid),"|securemode=",strlen("|securemode="));
    memset(timp_intstr,0,20);
    infra_int2str(_get_secure_mode(),timp_intstr);
    memcpy(signout->clientid+strlen(signout->clientid),timp_intstr,strlen(timp_intstr));
    memcpy(signout->clientid+strlen(signout->clientid),",timestamp=",strlen(",timestamp="));
    memcpy(signout->clientid+strlen(signout->clientid),timestamp,strlen(timestamp));
    memcpy(signout->clientid+strlen(signout->clientid),",signmethod=hmacsha256,ver=c-sdk-",strlen(",signmethod=hmacsha256,ver=c-sdk-"));
    memcpy(signout->clientid+strlen(signout->clientid),IOTX_SDK_VERSION,strlen(IOTX_SDK_VERSION));
    memcpy(signout->clientid+strlen(signout->clientid),alink_version,strlen(alink_version));

    signout->port = g_infra_mqtt_domain[region].port;

    return SUCCESS_RETURN;
}

#endif /* #ifndef DEVICE_MODEL_ENABLE */

#include <stdio.h>
#include <string.h>
#include "infra_defs.h"
#include "infra_string.h"
#include "dev_sign_api.h"
#include "dev_sign_wrapper.h"

#define DEV_SIGN_MQTT_MALLOC(size) HAL_Malloc(size)
#define DEV_SIGN_MQTT_FREE(ptr)    HAL_Free(ptr)

#define SIGN_MQTT_PORT (1883)

typedef enum {
    MODE_TLS_DIRECT             = 2,
    MODE_TCP_DIRECT_PLAIN       = 3,
} secure_mode_t;

iotx_mqtt_region_types_t g_sign_mqtt_region = IOTX_CLOUD_REGION_SHANGHAI;

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

int32_t IOT_Sign_MQTT(iotx_mqtt_region_types_t region, iotx_dev_meta_info_t *meta, iotx_sign_mqtt_t *signout)
{
    uint16_t res = 0, length = 0;
    char device_id[IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 1] = {0};
    char timestamp[20] = {0};

    char *signsource = NULL;
    uint16_t signsource_len = 0;
    uint8_t sign[32] = {0};
    const char *sign_fmt = "clientId%sdeviceName%sproductKey%stimestamp%s";
    const char *clientid_fmt = "%s|securemode=%d,timestamp=%s,signmethod=hmacsha256,gw=%d,ext=%d,ver=c-sdk-%s|";

    if (region < 0 || region >= IOTX_MQTT_DOMAIN_NUMBER || meta == NULL) {
        return -1;
    }

    memset(signout,0,sizeof(iotx_sign_mqtt_t));

    if (region >= 0) {
        g_sign_mqtt_region = region;
    }

    HAL_Snprintf(device_id,IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 1,"%s.%s",meta->product_key,meta->device_name);
    HAL_Snprintf(timestamp,20,"%s","2524608000000");

    do {
        signsource_len = strlen(sign_fmt) + strlen(device_id) + strlen(meta->device_name) + strlen(meta->product_key) + strlen(timestamp) + 1;
        signsource = DEV_SIGN_MQTT_MALLOC(signsource_len);
        if (signsource == NULL) {
            break;
        }
        memset(signsource,0,signsource_len);
        HAL_Snprintf(signsource,signsource_len,sign_fmt,device_id,meta->device_name,meta->product_key,timestamp);

        printf("signsource: %s\n",signsource);

        res = algo_hmac_sha256_wrapper((uint8_t *)signsource,strlen(signsource),(uint8_t *)meta->device_secret,strlen(meta->device_secret),sign);
        if (res < 0) {
            break;
        }

        /* Get Sign Information For MQTT */
        length = strlen(meta->product_key) + strlen(g_infra_mqtt_domain[g_sign_mqtt_region]) + 2;
        signout->hostname = DEV_SIGN_MQTT_MALLOC(length);
        if (signout->hostname == NULL) {
            break;
        }
        memset(signout->hostname,0,length);
        HAL_Snprintf(signout->hostname,length,"%s.%s",meta->product_key,g_infra_mqtt_domain[g_sign_mqtt_region]);

        length = strlen(meta->device_name) + strlen(meta->product_key) + 2;
        signout->username = DEV_SIGN_MQTT_MALLOC(length);
        if (signout->username == NULL) {
            break;
        }
        memset(signout->username,0,length);
        HAL_Snprintf(signout->username,length,"%s&%s",meta->device_name,meta->product_key);

        length = 32 * 2 + 1;
        signout->password = DEV_SIGN_MQTT_MALLOC(length);
        if (signout->password == NULL) {
            break;
        }
        memset(signout->password,0,length);
        infra_hex2str(sign,32,signout->password);

        length = strlen(clientid_fmt) + strlen(device_id) + strlen(timestamp) + strlen(IOTX_SDK_VERSION) + 30 + 1;
        signout->clientid = DEV_SIGN_MQTT_MALLOC(length);
        if (signout->clientid == NULL) {
            break;
        }
        memset(signout->clientid,0,length);
        HAL_Snprintf(signout->clientid,length,clientid_fmt,device_id,_get_secure_mode(),timestamp,0,0,IOTX_SDK_VERSION);

        signout->port = SIGN_MQTT_PORT;

        return 0;
    }while(0);
    
    if (signsource) {DEV_SIGN_MQTT_FREE(signsource);}
    if (signout->hostname) {DEV_SIGN_MQTT_FREE(signout->hostname);}
    if (signout->username) {DEV_SIGN_MQTT_FREE(signout->username);}
    if (signout->password) {DEV_SIGN_MQTT_FREE(signout->password);}
    if (signout->clientid) {DEV_SIGN_MQTT_FREE(signout->clientid);}

    return -1;
}
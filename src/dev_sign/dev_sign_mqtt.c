#include <string.h>
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

static char *g_sign_mqtt_direct[] = {
    "iot-as-mqtt.cn-shanghai.aliyuncs.com",         /* Shanghai */
    "iot-as-mqtt.ap-southeast-1.aliyuncs.com",      /* Singapore */
    "iot-as-mqtt.ap-northeast-1.aliyuncs.com",      /* Japan */
    "iot-as-mqtt.us-west-1.aliyuncs.com",           /* America */
    "iot-as-mqtt.eu-central-1.aliyuncs.com"         /* Germany */
};

iotx_cloud_region_types_t g_sign_mqtt_region = IOTX_CLOUD_REGION_SHANGHAI;

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

uint8_t IOT_Sign_MQTT(iotx_cloud_region_types_t region, iotx_sign_mqtt_t *signout)
{
    uint16_t res = 0, length = 0;
    char partner_id[IOTX_PARTNER_ID_LEN + 1] = {0};
    char module_id[IOTX_MODULE_ID_LEN + 1] = {0};
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN + 1] = {0};
    char device_secret[IOTX_DEVICE_SECRET_LEN + 1] = {0};
    char device_id[IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 1] = {0};
    char timestamp[20] = {0};

    char *signsource = NULL;
    uint16_t signsource_len = 0;
    uint8_t sign[32] = {0};
    const char *sign_fmt = "clientId%sdeviceName%sproductKey%stimestamp%s";
    const char *clientid_fmt = "%s|securemode=%d,timestamp=%s,signmethod=hmacsha256,gw=%d,ext=%d,partner_id=%s,module_id=%s,ver=c-sdk-%s|";

    if (signout == NULL || region >= sizeof(g_sign_mqtt_direct)/sizeof(char *)) {
        return -1;
    }

    memset(signout,0,sizeof(iotx_sign_mqtt_t));

    if (region >= 0) {
        g_sign_mqtt_region = region;
    }

    HAL_GetPartnerID(partner_id);
    HAL_GetModuleID(module_id);
    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);
    HAL_GetDeviceSecret(device_secret);
    HAL_Snprintf(device_id,IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 1,"%s.%s",product_key,device_name);
    HAL_Snprintf(timestamp,20,"%lld",HAL_UptimeMs());

    do {
        signsource_len = strlen(sign_fmt) + strlen(device_id) + strlen(device_name) + strlen(product_key) + strlen(timestamp) + 1;
        signsource = DEV_SIGN_MQTT_MALLOC(signsource_len);
        if (signsource == NULL) {
            break;
        }
        memset(signsource,0,signsource_len);
        HAL_Snprintf(signsource,signsource_len,sign_fmt,device_id,device_name,product_key,timestamp);

        res = algo_hmac_sha256_wrapper((uint8_t *)signsource,strlen(signsource),(uint8_t *)device_secret,strlen(device_secret),sign);
        if (res < 0) {
            break;
        }

        /* Get Sign Information For MQTT */
        length = strlen(product_key) + strlen(g_sign_mqtt_direct[g_sign_mqtt_region]) + 2;
        signout->hostname = DEV_SIGN_MQTT_MALLOC(length);
        if (signout->hostname == NULL) {
            break;
        }
        memset(signout->hostname,0,length);
        HAL_Snprintf(signout->hostname,length,"%s.%s",product_key,g_sign_mqtt_direct[g_sign_mqtt_region]);

        length = strlen(device_name) + strlen(product_key) + 2;
        signout->username = DEV_SIGN_MQTT_MALLOC(length);
        if (signout->username == NULL) {
            break;
        }
        memset(signout->username,0,length);
        HAL_Snprintf(signout->username,length,"%s.%s",device_name,product_key);

        length = 32 * 2 + 1;
        signout->password = DEV_SIGN_MQTT_MALLOC(length);
        if (signout->password == NULL) {
            break;
        }
        memset(signout->password,0,length);
        infra_hex2str(sign,32,signout->password);

        length = strlen(clientid_fmt) + strlen(device_id) + strlen(timestamp) + strlen(partner_id) + strlen(module_id) + strlen(IOTX_SDK_VERSION) + 30 + 1;
        signout->clientid = DEV_SIGN_MQTT_MALLOC(length);
        if (signout->clientid == NULL) {
            break;
        }
        memset(signout->clientid,0,length);
        HAL_Snprintf(signout->clientid,length,clientid_fmt,device_id,_get_secure_mode(),timestamp,0,0,partner_id,module_id,IOTX_SDK_VERSION);

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
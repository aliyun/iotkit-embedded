#include <string.h>
#include "infra_string.h"
#include "dev_sign_api.h"
#include "dev_sign_wrapper.h"

#ifdef INFRA_MEM_STATS
#define DEV_SIGN_MQTT_MALLOC(size) LITE_malloc(size, MEM_MAGIC, "devsign.mqtt")
#define DEV_SIGN_MQTT_FREE(ptr)    LITE_free(ptr)
#else
#define DEV_SIGN_MQTT_MALLOC(size) HAL_Malloc(size)
#define DEV_SIGN_MQTT_FREE(ptr)    HAL_Free(ptr)
#endif

#define SIGN_MQTT_HOSTNAME_LEN (128)
#define SIGN_MQTT_CLIENTID_LEN (256)
#define SIGN_MQTT_USERNAME_LEN (IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 2)
#define SIGN_MQTT_PASSWORD_LEN (128)

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

char g_sign_mqtt_hostname[SIGN_MQTT_HOSTNAME_LEN] = {0};
char g_sign_mqtt_username[SIGN_MQTT_USERNAME_LEN] = {0};
char g_sign_mqtt_password[SIGN_MQTT_PASSWORD_LEN] = {0};
char g_sign_mqtt_clientid[SIGN_MQTT_CLIENTID_LEN] = {0};
extern const char *iotx_ca_crt;

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
    int res = 0;
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
    const char *clientid_fmt = "%s|securemode=%d,timestamp=%s,signmethod=hmacsha256,gw=%d,ext=%d%s%s,ver=c-sdk-%s|";

    if (signout == NULL || region >= sizeof(g_sign_mqtt_direct)/sizeof(char *)) {
        return -1;
    }

    memset(g_sign_mqtt_hostname,0,SIGN_MQTT_HOSTNAME_LEN);
    memset(g_sign_mqtt_clientid,0,SIGN_MQTT_CLIENTID_LEN);
    memset(g_sign_mqtt_username,0,SIGN_MQTT_USERNAME_LEN);
    memset(g_sign_mqtt_password,0,SIGN_MQTT_PASSWORD_LEN);

    HAL_GetPartnerID(partner_id);
    HAL_GetModuleID(module_id);
    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);
    HAL_GetDeviceSecret(device_secret);
    HAL_Snprintf(device_id,IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 1,"%s.%s",product_key,device_name);
    HAL_Snprintf(timestamp,20,"%lld",HAL_UptimeMs());

    signsource_len = strlen(sign_fmt) + strlen(device_id) + strlen(device_name) + strlen(product_key) + strlen(timestamp) + 1;
    signsource = DEV_SIGN_MQTT_MALLOC(signsource_len);
    if (signsource == NULL) {
        return -1;
    }
    memset(signsource,0,signsource_len);
    HAL_Snprintf(signsource,signsource_len,sign_fmt,device_id,device_name,product_key,timestamp);

    res = algo_hmac_sha256_wrapper((uint8_t *)signsource,strlen(signsource),(uint8_t *)device_secret,strlen(device_secret),sign);
    if (res < 0) {
        return -1;
    }

    HAL_Snprintf(g_sign_mqtt_hostname,SIGN_MQTT_HOSTNAME_LEN,"%s.%s",product_key,g_sign_mqtt_direct[region]);
    HAL_Snprintf(g_sign_mqtt_username,SIGN_MQTT_USERNAME_LEN,"%s.%s",device_name,product_key);
    infra_hex2str(sign,32,g_sign_mqtt_password);
    HAL_Snprintf(g_sign_mqtt_clientid,SIGN_MQTT_CLIENTID_LEN,clientid_fmt,device_id,_get_secure_mode(),timestamp,0,0,partner_id,module_id,IOTX_SDK_VERSION);

    signout->hostname = g_sign_mqtt_hostname;
    signout->port = 1883;
    signout->username = g_sign_mqtt_username;
    signout->password = g_sign_mqtt_password;
    signout->clientid = g_sign_mqtt_clientid;
#ifdef SUPPORT_TLS
    signout->pub_key = iotx_ca_crt;
#endif

    return 0;
}
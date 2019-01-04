#include <stdio.h>
#include <string.h>
#include "infra_defs.h"
#include "infra_config.h"
#include "infra_string.h"
#include "infra_sha256.h"
#include "dev_sign_api.h"
#include "dev_sign_wrapper.h"

#ifdef INFRA_MEM_STATS
#define DEV_SIGN_MQTT_MALLOC(size)            LITE_malloc(size, MEM_MAGIC, "dev_sign")
#define DEV_SIGN_MQTT_FREE(ptr)               LITE_free(ptr)
#else
#define DEV_SIGN_MQTT_MALLOC(size)            HAL_Malloc(size)
#define DEV_SIGN_MQTT_FREE(ptr)               {HAL_Free((void *)ptr);ptr = NULL;}
#endif

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

int32_t IOT_Sign_MQTT(iotx_mqtt_region_types_t region, iotx_dev_meta_info_t *meta, iotx_sign_mqtt_t *signout)
{
    uint16_t length = 0;
    char device_id[IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 1] = {0};
    char timestamp[20] = {0};
    char timp_intstr[20] = {0};

    char *signsource = NULL;
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

    do {
        signsource_len = strlen(sign_fmt) + strlen(device_id) + strlen(meta->device_name) + strlen(meta->product_key) + strlen(timestamp) + 1;
        signsource = DEV_SIGN_MQTT_MALLOC(signsource_len);
        if (signsource == NULL) {
            break;
        }
        memset(signsource,0,signsource_len);
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
#if !defined(ON_DAILY) && !defined(ON_PRE)
        length = strlen(meta->product_key) + strlen(g_infra_mqtt_domain[region].region) + 2;
#else
        length = strlen(g_infra_mqtt_domain[region].region) + 2;
#endif
        signout->hostname = DEV_SIGN_MQTT_MALLOC(length);
        if (signout->hostname == NULL) {
            break;
        }
        memset(signout->hostname,0,length);
#if !defined(ON_DAILY) && !defined(ON_PRE)
        memcpy(signout->hostname,meta->product_key,strlen(meta->product_key));
        memcpy(signout->hostname+strlen(signout->hostname),".",strlen("."));
#endif
        memcpy(signout->hostname+strlen(signout->hostname),g_infra_mqtt_domain[region].region,strlen(g_infra_mqtt_domain[region].region));

        length = strlen(meta->device_name) + strlen(meta->product_key) + 2;
        signout->username = DEV_SIGN_MQTT_MALLOC(length);
        if (signout->username == NULL) {
            break;
        }
        memset(signout->username,0,length);
        memcpy(signout->username,meta->device_name,strlen(meta->device_name));
        memcpy(signout->username+strlen(signout->username),"&",strlen("&"));
        memcpy(signout->username+strlen(signout->username),meta->product_key,strlen(meta->product_key));

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

        if (signsource) {DEV_SIGN_MQTT_FREE(signsource);}
        return 0;
    }while(0);
    
    if (signsource) {DEV_SIGN_MQTT_FREE(signsource);}
    if (signout->hostname) {DEV_SIGN_MQTT_FREE(signout->hostname);}
    if (signout->username) {DEV_SIGN_MQTT_FREE(signout->username);}
    if (signout->password) {DEV_SIGN_MQTT_FREE(signout->password);}
    if (signout->clientid) {DEV_SIGN_MQTT_FREE(signout->clientid);}

    return -1;
}


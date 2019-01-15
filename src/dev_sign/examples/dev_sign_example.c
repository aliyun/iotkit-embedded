#include "dev_sign_internal.h"

#define EXAMPLE_PRODUCT_KEY     "a1X2bEnP82z"
#define EXAMPLE_PRODUCT_SECRET  "7jluWm1zql7bt8qK"
#define EXAMPLE_DEVICE_NAME     "example1"
#define EXAMPLE_DEVICE_SECRET   "ga7XA6KdlEeiPXQPpRbAjOZXwG8ydgSe"

void HAL_Printf(const char *fmt, ...);

int main(int argc, char *argv[])
{
    iotx_mqtt_region_types_t region = IOTX_CLOUD_REGION_SHANGHAI;
    iotx_dev_meta_info_t meta;
    iotx_sign_mqtt_t sign_mqtt;

    HAL_Printf("mqtt example\n");

    memset(&meta,0,sizeof(iotx_dev_meta_info_t));
    memcpy(meta.product_key,EXAMPLE_PRODUCT_KEY,strlen(EXAMPLE_PRODUCT_KEY));
    memcpy(meta.product_secret,EXAMPLE_PRODUCT_SECRET,strlen(EXAMPLE_PRODUCT_SECRET));
    memcpy(meta.device_name,EXAMPLE_DEVICE_NAME,strlen(EXAMPLE_DEVICE_NAME));
    memcpy(meta.device_secret,EXAMPLE_DEVICE_SECRET,strlen(EXAMPLE_DEVICE_SECRET));

    if (IOT_Sign_MQTT(region,&meta,&sign_mqtt) < 0) {
        return -1;
    }

#if 0   /* Uncomment this if you want to show more information */
    HAL_Printf("sign_mqtt.hostname: %s\n",sign_mqtt.hostname);
    HAL_Printf("sign_mqtt.port    : %d\n",sign_mqtt.port);
    HAL_Printf("sign_mqtt.username: %s\n",sign_mqtt.username);
    HAL_Printf("sign_mqtt.password: %s\n",sign_mqtt.password);
    HAL_Printf("sign_mqtt.clientid: %s\n",sign_mqtt.clientid);
#endif

    return 0;
}


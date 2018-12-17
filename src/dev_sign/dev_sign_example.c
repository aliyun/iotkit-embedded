#include <stdio.h>
#include "infra_defs.h"
#include "dev_sign_api.h"
#include "dev_sign_wrapper.h"

int main(int argc, char *argv[])
{
    iotx_mqtt_region_types_t region = IOTX_CLOUD_REGION_SHANGHAI;
    iotx_sign_mqtt_t sign_mqtt;

    printf("mqtt example\n");

    if (IOT_Sign_MQTT(region,&sign_mqtt) < 0) {
        return -1;
    }

    printf("sign_mqtt.hostname: %s\n",sign_mqtt.hostname);
    printf("sign_mqtt.port    : %d\n",sign_mqtt.port);
    printf("sign_mqtt.username: %s\n",sign_mqtt.username);
    printf("sign_mqtt.password: %s\n",sign_mqtt.password);
    printf("sign_mqtt.clientid: %s\n",sign_mqtt.clientid);

    HAL_Free(sign_mqtt.hostname);
    HAL_Free(sign_mqtt.username);
    HAL_Free(sign_mqtt.password);
    HAL_Free(sign_mqtt.clientid);

    return 0;
}
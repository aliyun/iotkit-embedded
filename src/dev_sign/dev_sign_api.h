#ifndef _DEV_SIGN_H_
#define _DEV_SIGN_H_

#include "infra_types.h"
#include "infra_defs.h"

typedef struct {
    char *hostname;
    uint16_t port;
    char *clientid;
    char *username;
    char *password;
} iotx_sign_mqtt_t;

int32_t IOT_Sign_MQTT(iotx_mqtt_region_types_t region, iotx_sign_mqtt_t *signout);

#endif

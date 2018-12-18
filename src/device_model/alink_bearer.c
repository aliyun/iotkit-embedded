/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"


static alink_bearer_ctx_t alink_bearer_mqtt;



int alink_bearer_open(void)
{
    memset(&alink_bearer_mqtt, 0, sizeof(alink_bearer_ctx_t));
    return alink_bearer_mqtt_open(&alink_bearer_mqtt);
}


int alink_bearer_conect(void)
{
    return alink_bearer_mqtt.p_api.bearer_connect(&alink_bearer_mqtt, 20000);
}


int alink_bearer_send(alink_bearer_type_t bearer, char *uri, uint8_t *payload, uint32_t len)
{
    (void)bearer;

    return alink_bearer_mqtt.p_api.bearer_pub(&alink_bearer_mqtt, uri, payload, len, 0);
}
/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "mdal_mal_import.h"


int HAL_MDAL_MAL_Init(iotx_mqtt_param_t *pInitParams)
{
    return -1;
}

int HAL_MDAL_MAL_Deinit()
{
    return -1;
}

int HAL_MDAL_MAL_Connect(char *proKey, char *devName, char *devSecret)
{
    return -1;
}

int HAL_MDAL_MAL_Disconnect(void)
{
    return -1;
}

int HAL_MDAL_MAL_Subscribe(const char *topic, int qos, unsigned int *mqtt_packet_id, int *mqtt_status, int timeout_ms)
{
    return -1;
}

int HAL_MDAL_MAL_Unsubscribe(const char *topic, unsigned int *mqtt_packet_id, int *mqtt_status)
{
    return -1;
}

int HAL_MDAL_MAL_Publish(const char *topic, int qos, const char *message, unsigned int msg_len)
{
    return -1;
}

int HAL_MDAL_MAL_State(void)
{
    return -1;
}

void HAL_MDAL_MAL_RegRecvCb(recv_cb cb)
{
    return;
}

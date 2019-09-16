/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __WIFI_MQTT_H__
#define __WIFI_MQTT_H__

#include "stdint.h"

enum {
    WIFI_CMP_PKT_TYPE_REQ = 1,
    WIFI_CMP_PKT_TYPE_RSP,
};

int wifi_mqtt_init(void *handle);
int wifi_mqtt_deinit(void);
int wifi_mqtt_report(char *topic, char *data, int len, int qos);
int wifi_build_packet(int type, void *id, void *ver, void *method, void *data, int code, void *packet, int *packet_len);
int wifi_mqtt_get_payload(void *mesg, char **payload, uint32_t *payload_len);
#endif

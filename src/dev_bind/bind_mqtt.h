/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __BIND_MQTT_H__
#define __BIND_MQTT_H__

#include "bind_utils.h"

int bind_mqtt_init(void *bind_handle, void *mqtt_handle);
int bind_mqtt_deinit(void *bind_handle);
int bind_report_token(void *bind_handle);
int bind_report_unbind(void *bind_handle);
#endif

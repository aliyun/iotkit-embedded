/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __BIND_API_H__
#define __BIND_API_H__
#include <stdint.h>

int IOT_Bind_Start(void *mqtt_handle, void *coap_handle);

int IOT_Bind_Yield(void);

int IOT_Bind_SetToken(uint8_t *token);

int IOT_Bind_SetToken_Ext(uint8_t *token, uint8_t token_len, char *passwd, uint8_t *bssid, uint8_t bssid_len);

int IOT_Bind_Reset(void);

int IOT_Bind_Stop(void);
#endif

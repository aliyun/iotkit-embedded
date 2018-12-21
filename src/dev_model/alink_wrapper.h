/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_WRAPPER_H__
#define __ALINK_WRAPPER_H__

#include "infra_types.h"
#include "infra_defs.h"
#include "infra_log.h"
#include "infra_list.h"
#include "infra_compat.h" 

int8_t HAL_GetPartnerID(char *pid_str);
int8_t HAL_GetModuleID(char *mid_str);
int8_t HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int8_t HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);
int8_t HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN]);

int HAL_SetProductKey(char *product_key);
int HAL_SetProductSecret(char *product_secret);
int HAL_SetDeviceName(char *device_name);
int HAL_SetDeviceSecret(char *device_secret);


void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);
uint64_t HAL_UptimeMs(void);

void HAL_SleepMs(uint32_t ms);

void *HAL_MutexCreate(void);
void HAL_MutexDestroy(void *mutex);
void HAL_MutexLock(void *mutex);
void HAL_MutexUnlock(void *mutex);


#endif /* #ifndef __ALINK_WRAPPER_H__ */
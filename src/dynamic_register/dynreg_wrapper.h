#ifndef _DYNREG_WRAPPER_H_
#define _DYNREG_WRAPPER_H_

#include "infra_types.h"

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);

int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int HAL_GetProductSecret(char *product_secret);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);
int HAL_SetDeviceSecret(char *device_secret);

#endif


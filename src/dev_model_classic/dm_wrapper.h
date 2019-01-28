#ifndef _DM_WRAPPER_H_
#define _DM_WRAPPER_H_

int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN]);

void *HAL_MutexCreate(void);
void HAL_MutexDestroy(void *mutex);
void HAL_MutexLock(void *mutex);
void HAL_MutexUnlock(void *mutex);

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
uint64_t HAL_UptimeMs(void);
void HAL_SleepMs(uint32_t ms);
void HAL_Srandom(uint32_t seed);
uint32_t HAL_Random(uint32_t region);

#endif
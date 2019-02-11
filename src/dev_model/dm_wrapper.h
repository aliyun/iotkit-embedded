#ifndef _DM_WRAPPER_H_
#define _DM_WRAPPER_H_

#include "wrappers_defs.h"

int HAL_SetProductKey(char *product_key);
int HAL_SetDeviceName(char *device_name);
int HAL_SetProductSecret(char *product_secret);
int HAL_SetDeviceSecret(char *device_secret);

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

int HAL_ThreadCreate(
            void **thread_handle,
            void *(*work_routine)(void *),
            void *arg,
            hal_os_thread_param_t *hal_os_thread_param,
            int *stack_used);
void HAL_ThreadDelete(void *thread_handle);

void *HAL_SemaphoreCreate(void);
void HAL_SemaphoreDestroy(void *sem);
void HAL_SemaphorePost(void *sem);
int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms);

#if defined(OTA_ENABLED) && !defined(BUILD_AOS)
void HAL_Firmware_Persistence_Start(void);
int HAL_Firmware_Persistence_Write(char *buffer, uint32_t length);
int HAL_Firmware_Persistence_Stop(void);
#endif

#endif
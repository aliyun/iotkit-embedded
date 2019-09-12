#ifndef _WRAPPERS_OS_H_
#define _WRAPPERS_OS_H_
#include <stdarg.h>
#include "infra_types.h"
#include "infra_defs.h"


#define HAL_Printf printf

void *HAL_Malloc(uint32_t size);

void HAL_Free(void *ptr);

uint64_t HAL_UptimeMs(void);

void HAL_SleepMs(uint32_t ms);

void HAL_Srandom(uint32_t seed);

uint32_t HAL_Random(uint32_t region);

int HAL_Snprintf(char *str, const int len, const char *fmt, ...);

int HAL_Vsnprintf(char *str, const int len, const char *format, va_list ap);

void *HAL_SemaphoreCreate(void);

void HAL_SemaphoreDestroy(void *sem);

void HAL_SemaphorePost(void *sem);

int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms);

int HAL_ThreadCreate(
            void **thread_handle,
            void *(*work_routine)(void *),
            void *arg,
            hal_os_thread_param_t *hal_os_thread_param,
            int *stack_used);

void *HAL_MutexCreate(void);

void HAL_MutexDestroy(void *mutex);

void HAL_MutexLock(void *mutex);

void HAL_MutexUnlock(void *mutex);

void HAL_Reboot(void);

int HAL_GetFirmwareVersion(char *version);

void HAL_Firmware_Persistence_Start(void);

int HAL_Firmware_Persistence_Write(char *buffer, uint32_t length);

int HAL_Firmware_Persistence_Stop(void);

#endif


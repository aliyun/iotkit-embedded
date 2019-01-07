/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef _AT_WRAPPER_H_
#define _AT_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#include "wrappers_defs.h"

#ifndef NULL
#define NULL (void *)0
#endif

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);
uint64_t HAL_UptimeMs(void);
void *HAL_MutexCreate(void);
void HAL_MutexDestroy(void *mutex);
void HAL_MutexLock(void *mutex);
void HAL_MutexUnlock(void *mutex);
void HAL_SleepMs(uint32_t ms);
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

#ifdef __cplusplus
}
#endif

#endif



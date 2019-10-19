/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>

#include <process.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>

#include "infra_config.h"
#include "infra_compat.h"
#include "infra_defs.h"
#include "wrappers_defs.h"
#include "wrappers_os.h"

#define hal_err(...)        HAL_Printf("[prt] "), HAL_Printf(__VA_ARGS__), HAL_Printf("\r\n")
#define hal_debug(...)      HAL_Printf("[prt] "), HAL_Printf(__VA_ARGS__), HAL_Printf("\r\n")

#define __DEMO__

#ifdef __DEMO__
    #ifdef DYNAMIC_REGISTER
        char _product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1ZETBPbycq";
        char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "L68wCVXYUaNg1Ey9";
        char _device_name[IOTX_DEVICE_NAME_LEN + 1]       = "example1";
        char _device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "";
    #else
        #ifdef DEVICE_MODEL_ENABLED
            char _product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1RIsMLz2BJ";
            char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "fSAF0hle6xL0oRWd";
            char _device_name[IOTX_DEVICE_NAME_LEN + 1]       = "example1";
            char _device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "RDXf67itLqZCwdMCRrw0N5FHbv5D7jrE";
        #else
            char _product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1MZxOdcBnO";
            char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "h4I4dneEFp7EImTv";
            char _device_name[IOTX_DEVICE_NAME_LEN + 1]       = "test_01";
            char _device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "t9GmMf2jb3LgWfXBaZD2r3aJrfVWBv56";
        #endif
    #endif
#endif

void *HAL_MutexCreate(void)
{
    HANDLE mutex;

    if (NULL == (mutex = CreateMutex(NULL, FALSE, NULL))) {
        hal_err("create mutex error");
    }

    return mutex;
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
    if (0 == CloseHandle(mutex)) {
        hal_err("destroy mutex error");
    }
}

void HAL_MutexLock(_IN_ void *mutex)
{
    if (WAIT_FAILED == WaitForSingleObject(mutex, INFINITE)) {
        hal_err("lock mutex error");
    }
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
    ReleaseMutex(mutex);
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return malloc(size);
}

void HAL_Free(_IN_ void *ptr)
{
    free(ptr);
}

uint64_t HAL_UptimeMs(void)
{
    return (uint64_t)(GetTickCount());
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
    Sleep(ms);
}

uint32_t orig_seed = 2;
void HAL_Srandom(uint32_t seed)
{
    orig_seed = seed;
}

uint32_t HAL_Random(uint32_t region)
{
    orig_seed = 1664525 * orig_seed + 1013904223;
    return (region > 0) ? (orig_seed % region) : orig_seed;
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = _vsnprintf(str, len - 1, fmt, args);
    va_end(args);

    return ret;
}


int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *fmt, va_list ap)
{
    int ret;

    ret = _vsnprintf(str, len - 1, fmt, ap);

    return ret;
}

int HAL_GetFirmwareVersion(char *version)
{
    char *ver = "app-1.0.0-20180101.1000";
    int len = strlen(ver);
    memset(version, 0x0, IOTX_FIRMWARE_VER_LEN);
    strncpy(version, ver, IOTX_FIRMWARE_VER_LEN);
    version[len] = '\0';
    return strlen(version);
}

#if 1

void *HAL_SemaphoreCreate(void)
{
    return CreateSemaphore(NULL, 0, 1, NULL);
}

void HAL_SemaphoreDestroy(_IN_ void *sem)
{
    CloseHandle(sem);
}

void HAL_SemaphorePost(_IN_ void *sem)
{
    ReleaseSemaphore(sem, 1, NULL);

}

int HAL_SemaphoreWait(_IN_ void *sem, _IN_ uint32_t timeout_ms)
{
    uint32_t timeout = timeout_ms;
    if (timeout == (uint32_t) - 1) {
        timeout = INFINITE;
    }
    return WaitForSingleObject(sem, timeout);
}
#define DEFAULT_THREAD_SIZE 4096
int HAL_ThreadCreate(
            _OU_ void **thread_handle,
            _IN_ void *(*work_routine)(void *),
            _IN_ void *arg,
            _IN_ hal_os_thread_param_t *hal_os_thread_param,
            _OU_ int *stack_used)
{
    SIZE_T stack_size;
    (void)stack_used;

    if (!hal_os_thread_param || hal_os_thread_param->stack_size == 0) {
        stack_size = DEFAULT_THREAD_SIZE;
    } else {
        stack_size = hal_os_thread_param->stack_size;
    }
    thread_handle = CreateThread(NULL, stack_size,
                                 (LPTHREAD_START_ROUTINE)work_routine,
                                 arg, 0, NULL);
    if (thread_handle == NULL) {
        return -1;
    }
    return 0;
}

#endif  /* #if 0 */

#ifdef __DEMO__
    static FILE *fp;

    #define otafilename "/tmp/alinkota.bin"
#endif

void HAL_Firmware_Persistence_Start(void)
{
#ifdef __DEMO__
    fp = fopen(otafilename, "w");
    //    assert(fp);
#endif
    return;
}

int HAL_Firmware_Persistence_Write(_IN_ char *buffer, _IN_ uint32_t length)
{
#ifdef __DEMO__
    unsigned int written_len = 0;
    written_len = fwrite(buffer, 1, length, fp);

    if (written_len != length) {
        return -1;
    }
#endif
    return 0;
}

int HAL_Firmware_Persistence_Stop(void)
{
#ifdef __DEMO__
    if (fp != NULL) {
        fclose(fp);
    }
#endif

    return 0;
}


typedef struct {
    char name[32];
    void (*func)(void *);
    void *user_data;
    HANDLE tmr;
    int index;
} timer_t;

#define HAL_TIMER_SIZE  10

static int timerInited = 0;
static HANDLE hTimerQueue = NULL;
static timer_t *gTimerTable[HAL_TIMER_SIZE] = {0};

static void timer_init()
{
    if (timerInited) {
        return;
    }
    hTimerQueue = CreateTimerQueue();
    if (NULL == hTimerQueue) {
        printf("Failed to create timer queue\n\r");
        return;
    }
    timerInited = 1;
}

static void CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    timer_t *pTimer = (timer_t *)lpParam;

    hal_debug("Timer %s(%d) expired\n\r", pTimer->name, pTimer->index);
    pTimer->func(pTimer->user_data);
}

void HAL_Reboot(void)
{
    hal_err("not support HAL_Reboot! \n\r");
}


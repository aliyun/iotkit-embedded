#include <process.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include "aliot_platform_os.h"


void *aliot_platform_mutex_create(void)
{
    HANDLE mutex;

    if (NULL == (mutex = CreateMutex(NULL, FALSE, NULL))) {
        printf("create mutex error: %u\n", GetLastError());
    }

    return mutex;
}

void aliot_platform_mutex_destroy(_IN_ void *mutex)
{
    if (0 == CloseHandle(mutex)) {
        printf("destroy mutex error: %u\n", GetLastError());
    }
}

void aliot_platform_mutex_lock(_IN_ void *mutex)
{
    if (WAIT_FAILED == WaitForSingleObject(mutex, INFINITE)) {
        printf("lock mutex error: %u\n", GetLastError());
    }
}

void aliot_platform_mutex_unlock(_IN_ void *mutex)
{
    if (0 == ReleaseMutex(mutex)) {
        printf("unlock mutex error: %u\n", GetLastError());
    }
}

void *aliot_platform_malloc(_IN_ uint32_t size)
{
    return malloc(size);
}

void aliot_platform_free(_IN_ void *ptr)
{
    return free(ptr);
}

int aliot_platform_ota_start(void)
{
    printf("this interface is NOT support yet.");
    return -1;
}

int aliot_platform_ota_write(_IN_ char *buffer, _IN_ uint32_t length)
{
    printf("this interface is NOT support yet.");
    return -1;
}

int aliot_platform_ota_finalize(_IN_ int stat)
{
    printf("this interface is NOT support yet.");
    return -1;
}

uint32_t aliot_platform_time_get_ms(void)
{
    return (uint32_t)(GetTickCount());
}

void aliot_platform_msleep(_IN_ uint32_t ms)
{
    Sleep(ms);
}

void aliot_platform_printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}

char *aliot_platform_module_get_pid(char pid[])
{
    printf("this interface is NOT support yet.");
    return NULL;
}

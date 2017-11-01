/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>

#include "iot_import.h"

#include <process.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>


#define PLATFORM_WINOS_PERROR printf


void *HAL_MutexCreate(void)
{
    HANDLE mutex;

    if (NULL == (mutex = CreateMutex(NULL, FALSE, NULL))) {
        PLATFORM_WINOS_PERROR("create mutex error");
    }

    return mutex;
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
    if (0 == CloseHandle(mutex)) {
        PLATFORM_WINOS_PERROR("destroy mutex error");
    }
}

void HAL_MutexLock(_IN_ void *mutex)
{
    if (WAIT_FAILED == WaitForSingleObject(mutex, INFINITE)) {
        PLATFORM_WINOS_PERROR("lock mutex error");
    }
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
    if (WAIT_FAILED == WaitForSingleObject(mutex, INFINITE)) {
        PLATFORM_WINOS_PERROR("lock mutex error");
    }
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
    return (region > 0) ? (orig_seed % region) : 0;
}

void HAL_Printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}


int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = _vsnprintf(str, len-1, fmt, args);
    va_end(args);

    return ret;
}


int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *fmt, va_list ap)
{
    int ret;

    ret = _vsnprintf(str, len-1, fmt, ap);

    return ret;
}


int HAL_GetPartnerID(char pid_str[PID_STRLEN_MAX])
{
    memset(pid_str, 0x0, PID_STRLEN_MAX);
#ifdef __UBUNTU_SDK_DEMO__
    strcpy(pid_str, "example.demo.partner-id");
#endif
    return strlen(pid_str);
}

int HAL_GetModuleID(char mid_str[MID_STRLEN_MAX])
{
    memset(mid_str, 0x0, MID_STRLEN_MAX);
#ifdef __UBUNTU_SDK_DEMO__
    strcpy(mid_str, "example.demo.module-id");
#endif
    return strlen(mid_str);
}


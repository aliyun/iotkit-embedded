 /*
  * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
  * License-Identifier: Apache-2.0
  *
  * Licensed under the Apache License, Version 2.0 (the "License"); you may
  * not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */


#include <process.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include "platform_os.h"


#define PLATFORM_WINOS_PERROR(log) \
    do { \
        char *s = NULL; \
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
                       NULL, \
                       WSAGetLastError(), \
                       MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), \
                       (LPSTR)&s, \
                       0, \
                       NULL); \
        printf("WINOS#LINE=%d#FUNC=%s()#%s: %s", __LINE__, __FUNCTION__, log, s); \
        fflush(stdout);\
        LocalFree(s); \
    }while(0);

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
    if (0 == ReleaseMutex(mutex)) {
        PLATFORM_WINOS_PERROR("unlock mutex error");
    }
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return malloc(size);
}

void HAL_Free(_IN_ void *ptr)
{
    return free(ptr);
}

int HAL_OTA_Start(const char *md5, uint32_t file_size)
{
    printf("this interface is NOT support yet.");
    return -1;
}

int HAL_OTA_Write(_IN_ char *buffer, _IN_ uint32_t length)
{
    printf("this interface is NOT support yet.");
    return -1;
}

int HAL_OTA_Finalize(_IN_ int stat)
{
    printf("this interface is NOT support yet.");
    return -1;
}

uint32_t HAL_UptimeMs(void)
{
    return (uint32_t)(GetTickCount());
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
    Sleep(ms);
}

void HAL_Printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}

char *HAL_GetPartnerID(char pid_str[])
{
    return NULL;
}

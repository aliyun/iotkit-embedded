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

#include <process.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>

#include "iot_import.h"
#include "iot_export.h"
#include "iot_import_product.h"

#define __DEMO__

#ifdef __DEMO__
char _product_key[PRODUCT_KEY_LEN + 1];
char _product_secret[PRODUCT_SECRET_LEN + 1];
char _device_name[DEVICE_NAME_LEN + 1];
char _device_secret[DEVICE_SECRET_LEN + 1];
#endif


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


int HAL_GetPartnerID(char* pid_str)
{
    memset(pid_str, 0x0, PID_STRLEN_MAX);
#ifdef __DEMO__
    strcpy(pid_str, "example.demo.partner-id");
#endif
    return strlen(pid_str);
}

int HAL_GetModuleID(char* mid_str)
{
    memset(mid_str, 0x0, MID_STRLEN_MAX);
#ifdef __DEMO__
    strcpy(mid_str, "example.demo.module-id");
#endif
    return strlen(mid_str);
}

char *HAL_GetChipID(_OU_ char* cid_str)
{
    memset(cid_str, 0x0, HAL_CID_LEN);
#ifdef __DEMO__
    strncpy(cid_str, "rtl8188eu 12345678", HAL_CID_LEN);
    cid_str[HAL_CID_LEN - 1] = '\0';
#endif
    return cid_str;
}

int HAL_GetDeviceID(_OU_ char* device_id)
{
    memset(device_id, 0x0, DEVICE_ID_LEN);
#ifdef __DEMO__
    HAL_Snprintf(device_id, DEVICE_ID_LEN, "%s.%s", _product_key, _device_name);
    device_id[DEVICE_ID_LEN - 1] = '\0';
#endif

    return strlen(device_id);
}

int HAL_SetProductKey(_IN_ char* product_key)
{
	int len = strlen(product_key);
#ifdef __DEMO__
	if (len > PRODUCT_KEY_LEN) return -1;
	memset(_product_key, 0x0, PRODUCT_KEY_LEN + 1);
	strncpy(_product_key, product_key, len);
	FILE *fp = fopen("pk", "w");
	if (fp == NULL)
		return -1;
	unsigned int written_len = 0;
	written_len = fwrite(product_key, 1, strlen(product_key), fp);

	if (written_len != strlen(product_key)) {
		return -1;
	}
#endif
	return written_len;
}


int HAL_SetDeviceName(_IN_ char* device_name)
{
    int len = strlen(device_name);
#ifdef __DEMO__
    if (len > DEVICE_NAME_LEN) return -1;
    memset(_device_name, 0x0, DEVICE_NAME_LEN + 1);
    strncpy(_device_name, device_name, len);
	FILE *fp = fopen("dn", "w");
    if(fp == NULL)
    	return -1;
    unsigned int written_len = 0;
	written_len = fwrite(device_name, 1, strlen(device_name), fp);

	if (written_len != strlen(device_name)) {
		return -1;
	}
#endif
    return written_len;
}


int HAL_SetDeviceSecret(_IN_ char* device_secret)
{
    int len = strlen(device_secret);
#ifdef __DEMO__
    if (len > DEVICE_SECRET_LEN) return -1;
    memset(_device_secret, 0x0, DEVICE_SECRET_LEN + 1);
    strncpy(_device_secret, device_secret, len);
    FILE *fp = fopen("ds", "w");
    if(fp == NULL)
    	return -1;
    unsigned int written_len = 0;
	written_len = fwrite(device_secret, 1, strlen(device_secret), fp);

	if (written_len != strlen(device_secret)) {
		return -1;
	}
#endif
    return written_len;
}


int HAL_SetProductSecret(_IN_ char* product_secret)
{
    int len = strlen(product_secret);
#ifdef __DEMO__
    if (len > PRODUCT_SECRET_LEN) return -1;
    memset(_product_secret, 0x0, PRODUCT_SECRET_LEN + 1);
    strncpy(_product_secret, product_secret, len);
	FILE *fp = fopen("ps", "w");
    if(fp == NULL)
    	return -1;
    unsigned int written_len = 0;
	written_len = fwrite(product_secret, 1, strlen(product_secret), fp);

	if (written_len != strlen(product_secret)) {
		return -1;
	}
#endif
    return written_len;
}

int HAL_GetProductKey(_OU_ char* product_key)
{
    int len = strlen(_product_key);
    memset(product_key, 0x0, PRODUCT_KEY_LEN);

#ifdef __DEMO__
    strncpy(product_key, _product_key, len);
	FILE * fp = fopen("pk","r");
    if (NULL == fp) return -1;
    len = fread(product_key,PRODUCT_KEY_LEN,1,fp);
#endif

    return strlen(product_key);
}

int HAL_GetProductSecret(_OU_ char* product_secret)
{
    int len = strlen(_product_secret);
    memset(product_secret, 0x0, PRODUCT_SECRET_LEN);

#ifdef __DEMO__
    strncpy(product_secret, _product_secret, len);
	FILE * fp = fopen("ps","r");
    if (NULL == fp) return -1;
    len = fread(product_secret,PRODUCT_SECRET_LEN,1,fp);
#endif

    return strlen(product_secret);
}

int HAL_GetDeviceName(_OU_ char* device_name)
{
    int len = strlen(_device_name);
    memset(device_name, 0x0, DEVICE_NAME_LEN);

#ifdef __DEMO__
    strncpy(device_name, _device_name, len);
	FILE * fp = fopen("dn","r");
    if (NULL == fp) return -1;
    len = fread(device_name,DEVICE_NAME_LEN,1,fp);
#endif

    return strlen(device_name);
}

int HAL_GetDeviceSecret(_OU_ char* device_secret)
{
    int len = strlen(_device_secret);
    memset(device_secret, 0x0, DEVICE_SECRET_LEN);

#ifdef __DEMO__
    strncpy(device_secret, _device_secret, len);
    FILE * fp = fopen("ds","r");
    if (NULL == fp) return -1;
    len = fread(device_secret,DEVICE_SECRET_LEN,1,fp);
#endif

    return strlen(device_secret);
}

int HAL_GetFirmwareVesion(_OU_ char* version)
{
    memset(version, 0x0, FIRMWARE_VERSION_MAXLEN);
#ifdef __DEMO__
    strncpy(version, "1.0", FIRMWARE_VERSION_MAXLEN);
    version[FIRMWARE_VERSION_MAXLEN - 1] = '\0';
#endif

    return strlen(version);
}

#ifdef __DEMO__
static FILE *fp_temp;

#define otafilename "/tmp/alinkota.bin"
#endif

void HAL_Firmware_Persistence_Start(void)
{
#ifdef __DEMO__
    fp_temp = fopen(otafilename, "w");
//    assert(fp);
#endif
    return;
}

int HAL_Firmware_Persistence_Write(_IN_ char *buffer, _IN_ uint32_t length)
{
#ifdef __DEMO__
    unsigned int written_len = 0;
    written_len = fwrite(buffer, 1, length, fp_temp);

    if (written_len != length) {
        return -1;
    }
#endif
    return 0;
}

int HAL_Firmware_Persistence_Stop(void)
{
#ifdef __DEMO__
    if (fp_temp != NULL) {
        fclose(fp_temp);
    }
#endif

    /* check file md5, and burning it to flash ... finally reboot system */
    return 0;
}



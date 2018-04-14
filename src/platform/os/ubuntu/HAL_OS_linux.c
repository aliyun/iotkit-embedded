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

#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "iot_import.h"
#ifdef MQTT_ID2_AUTH
#include "tfs.h"
#endif /**< MQTT_ID2_AUTH*/

#define __DEMO__

#ifdef __DEMO__
char _product_key[PRODUCT_KEY_LEN + 1];
char _product_secret[PRODUCT_SECRET_LEN + 1];
char _device_name[DEVICE_NAME_LEN + 1];
char _device_secret[DEVICE_SECRET_LEN + 1];
#define UNUSED(expr) do { (void)(expr); } while (0)
#endif

void *HAL_MutexCreate(void)
{
    int err_num;
    pthread_mutex_t *mutex = (pthread_mutex_t *)HAL_Malloc(sizeof(pthread_mutex_t));
    if (NULL == mutex) {
        return NULL;
    }

    if (0 != (err_num = pthread_mutex_init(mutex, NULL))) {
        perror("create mutex failed");
        HAL_Free(mutex);
        return NULL;
    }

    return mutex;
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_destroy((pthread_mutex_t *)mutex))) {
        perror("destroy mutex failed");
    }

    HAL_Free(mutex);
}

void HAL_MutexLock(_IN_ void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_lock((pthread_mutex_t *)mutex))) {
        perror("lock mutex failed");
    }
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_unlock((pthread_mutex_t *)mutex))) {
        perror("unlock mutex failed");
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

#ifdef __APPLE__
uint64_t HAL_UptimeMs(void)
{
    struct timeval tv = { 0 };
    uint64_t time_ms;

    gettimeofday(&tv, NULL);

    time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return time_ms;
}
#else
uint64_t HAL_UptimeMs(void)
{
    uint64_t            time_ms;
    struct timespec     ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    time_ms = ((uint64_t)ts.tv_sec * (uint64_t)1000) + (ts.tv_nsec / 1000 / 1000);

    return time_ms;
}
#endif

void HAL_SleepMs(_IN_ uint32_t ms)
{
    usleep(1000 * ms);
}

void HAL_Srandom(uint32_t seed)
{
    srandom(seed);
}

uint32_t HAL_Random(uint32_t region)
{
    return (region > 0) ? (random() % region) : 0;
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}

void HAL_Printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
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

#ifdef MQTT_ID2_AUTH
int HAL_GetID2(_OU_ char* id2_str)
{
	int rc;
    uint8_t                 id2[TFS_ID2_LEN + 1] = {0};
	uint32_t                id2_len = TFS_ID2_LEN + 1;
    memset(id2_str, 0x0, TFS_ID2_LEN + 1);
#ifdef __DEMO__
    rc = tfs_get_ID2(id2, &id2_len);
    if (rc < 0) return rc;
    strncpy(id2_str, (const char*)id2, TFS_ID2_LEN);
#endif
    return strlen(id2_str);
}
#endif /**< MQTT_ID2_AUTH*/

int HAL_SetProductKey(_IN_ char* product_key)
{
    int len = strlen(product_key);
#ifdef __DEMO__
    if (len > PRODUCT_KEY_LEN) return -1;
    memset(_product_key, 0x0, PRODUCT_KEY_LEN + 1);
    strncpy(_product_key, product_key, len);
	FILE *fp = fopen("pk", "w");
    if(fp == NULL)
    	return -1;
    unsigned int written_len = 0;
	written_len = fwrite(product_key, 1, strlen(product_key), fp);
    fclose(fp);

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
    fclose(fp);

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
    fclose(fp);

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
    fclose(fp);

	if (written_len != strlen(product_secret)) {
		return -1;
	}
#endif
    return written_len;
}

int HAL_GetProductKey(_OU_ char* product_key)
{
    int len;
    memset(product_key, 0x0, PRODUCT_KEY_LEN);

#ifdef __DEMO__
	FILE * fp = fopen("pk","r");
    if (NULL == fp) return -1;
    len = fread(product_key,PRODUCT_KEY_LEN,1,fp);
    UNUSED(len);
    product_key[PRODUCT_KEY_LEN] = '\0';
    fclose(fp);
#endif

    return strlen(product_key);
}

int HAL_GetProductSecret(_OU_ char* product_secret)
{
    int len;
    memset(product_secret, 0x0, PRODUCT_SECRET_LEN);

#ifdef __DEMO__
	FILE * fp = fopen("ps","r");
    if (NULL == fp) return -1;
    len = fread(product_secret,PRODUCT_SECRET_LEN,1,fp);
    UNUSED(len);
    product_secret[PRODUCT_SECRET_LEN] = '\0';
    fclose(fp);
#endif

    return strlen(product_secret);
}

int HAL_GetDeviceName(_OU_ char* device_name)
{
    int len;
    memset(device_name, 0x0, DEVICE_NAME_LEN);

#ifdef __DEMO__
	FILE * fp = fopen("dn","r");
    if (NULL == fp) return -1;
    len = fread(device_name,DEVICE_NAME_LEN,1,fp);
    UNUSED(len);
    device_name[DEVICE_NAME_LEN] = '\0';
    fclose(fp);
#endif

    return strlen(device_name);
}

int HAL_GetDeviceSecret(_OU_ char* device_secret)
{
    int len;
    memset(device_secret, 0x0, DEVICE_SECRET_LEN);

#ifdef __DEMO__
    FILE * fp = fopen("ds","r");
    if (NULL == fp) return -1;
    len = fread(device_secret,DEVICE_SECRET_LEN,1,fp);
    UNUSED(len);
    device_secret[DEVICE_SECRET_LEN] = '\0';
    fclose(fp);
#endif

    return strlen(device_secret);
}


int HAL_GetFirmwareVesion(_OU_ char* version)
{
    char *ver = "1.0";
    int len = strlen(ver);
    memset(version, 0x0, FIRMWARE_VERSION_MAXLEN);
#ifdef __DEMO__
    strncpy(version, ver, len);
    version[len] = '\0';
#endif
    return strlen(version);
}

static FILE *fp;

#define otafilename "/tmp/alinkota.bin"

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

    /* check file md5, and burning it to flash ... finally reboot system */

    return 0;
}



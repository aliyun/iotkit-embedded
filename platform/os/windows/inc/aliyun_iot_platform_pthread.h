#ifndef ALIYUN_IOT_PLATFORM_PTHREAD_H
#define ALIYUN_IOT_PLATFORM_PTHREAD_H


#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include <windows.h>

typedef struct ALIYUN_IOT_MUTEX
{
    HANDLE lock;
}ALIYUN_IOT_MUTEX_S;

typedef struct ALIYUN_IOT_PTHREAD
{
    HANDLE threadID;
}ALIYUN_IOT_PTHREAD_S;

#define THREAD_NAME_LEN 64
typedef struct ALIYUN_IOT_PTHREAD_PARAM
{
    uint32_t stackDepth;
    uint32_t priority;
    int8_t threadName[THREAD_NAME_LEN];
}ALIYUN_IOT_PTHREAD_PARAM_S;

int32_t aliyun_iot_mutex_init(ALIYUN_IOT_MUTEX_S*mutex);

int32_t aliyun_iot_mutex_lock(ALIYUN_IOT_MUTEX_S*mutex);

int32_t aliyun_iot_mutex_unlock(ALIYUN_IOT_MUTEX_S *mutex);

int32_t aliyun_iot_mutex_destory(ALIYUN_IOT_MUTEX_S *mutex);

int32_t aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S* handle,void*(*func)(void*),void *arg,ALIYUN_IOT_PTHREAD_PARAM_S* param);

int32_t aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S*handle);

int32_t aliyun_iot_pthread_taskdelay( int32_t MsToDelay);

int32_t aliyun_iot_pthread_setname(char* name);

int32_t aliyun_iot_pthread_param_set(ALIYUN_IOT_PTHREAD_PARAM_S *param, int8_t *threadName, uint32_t stackSize, uint32_t threadPriority);

#endif




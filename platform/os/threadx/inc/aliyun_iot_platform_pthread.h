#ifndef ALIYUN_IOT_PLATFORM_PTHREAD_H
#define ALIYUN_IOT_PLATFORM_PTHREAD_H

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_platform_memory.h"
#include "threadx/tx_api.h"


typedef struct ALIYUN_IOT_MUTEX
{
	TX_MUTEX *pMutex;
}ALIYUN_IOT_MUTEX_S;

typedef struct ALIYUN_IOT_PTHREAD
{
    TX_THREAD *pThreadID;
	void *pThreadStack;
}ALIYUN_IOT_PTHREAD_S;

#define THREAD_NAME_LEN 64
typedef struct ALIYUN_IOT_PTHREAD_PARAM
{
    UINT32 stackDepth;
    UINT32 priority;
    INT8 threadName[THREAD_NAME_LEN];
}ALIYUN_IOT_PTHREAD_PARAM_S;


INT32 aliyun_iot_mutex_init(ALIYUN_IOT_MUTEX_S *mutex);

INT32 aliyun_iot_mutex_lock(ALIYUN_IOT_MUTEX_S *mutex);

INT32 aliyun_iot_mutex_unlock(ALIYUN_IOT_MUTEX_S *mutex);

INT32 aliyun_iot_mutex_destory(ALIYUN_IOT_MUTEX_S *mutex);

INT32 aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S *handle, void*(*func)(void*), void *arg, ALIYUN_IOT_PTHREAD_PARAM_S *param);

INT32 aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S *handle);

INT32 aliyun_iot_pthread_taskdelay( INT32 MsToDelay);

INT32 aliyun_iot_pthread_setname(char* name);

INT32 aliyun_iot_pthread_param_set(ALIYUN_IOT_PTHREAD_PARAM_S *param, INT8 *threadName, UINT32 stackSize, UINT32 threadPriority);

#endif




#ifndef ALIYUN_IOT_PLATFORM_PTHREAD_H
#define ALIYUN_IOT_PLATFORM_PTHREAD_H

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_platform_memory.h"

#ifndef IOT_SDK_PLATFORM_UCOSIII

#include "ucos_ii.h"
#include "os_app_cfg.h"
#include "os_cfg.h"
#else

#include "os.h"

#endif

#ifndef IOT_SDK_PLATFORM_UCOSIII

typedef struct ALIYUN_IOT_MUTEX
{
	OS_EVENT *osEvent;
}ALIYUN_IOT_MUTEX_S;

typedef struct ALIYUN_IOT_PTHREAD
{
    UINT32 taskPri;
	OS_STK *osSTK;
}ALIYUN_IOT_PTHREAD_S;

#define THREAD_NAME_LEN 64
typedef struct ALIYUN_IOT_PTHREAD_PARAM
{
    UINT32 stackDepth;
    UINT32 priority;
    INT8 threadName[THREAD_NAME_LEN];
}ALIYUN_IOT_PTHREAD_PARAM_S;
#else

typedef struct ALIYUN_IOT_MUTEX
{
	OS_MUTEX osMutex;
}ALIYUN_IOT_MUTEX_S;

typedef struct ALIYUN_IOT_PTHREAD
{
    OS_TCB  *taskTcb;
	CPU_STK	*cpuStk;
}ALIYUN_IOT_PTHREAD_S;

#define THREAD_NAME_LEN 64
typedef struct ALIYUN_IOT_PTHREAD_PARAM
{
    UINT32 stackDepth;
    UINT32 priority;
    INT8 threadName[THREAD_NAME_LEN];
}ALIYUN_IOT_PTHREAD_PARAM_S;
#endif

INT32 aliyun_iot_mutex_init(ALIYUN_IOT_MUTEX_S*mutex);

INT32 aliyun_iot_mutex_lock(ALIYUN_IOT_MUTEX_S*mutex);

INT32 aliyun_iot_mutex_unlock(ALIYUN_IOT_MUTEX_S *mutex);

INT32 aliyun_iot_mutex_destory(ALIYUN_IOT_MUTEX_S *mutex);

INT32 aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S* handle,void*(*func)(void*),void *arg,ALIYUN_IOT_PTHREAD_PARAM_S* param);

INT32 aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S*handle);

INT32 aliyun_iot_pthread_taskdelay( INT32 MsToDelay);

INT32 aliyun_iot_pthread_setname(char* name);

INT32 aliyun_iot_pthread_param_set(ALIYUN_IOT_PTHREAD_PARAM_S *param, INT8 *threadName, UINT32 stackSize, UINT32 threadPriority);

#endif




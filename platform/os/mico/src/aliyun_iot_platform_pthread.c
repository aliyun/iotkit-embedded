#include "aliyun_iot_platform_pthread.h"
#include <string.h>

INT32 aliyun_iot_mutex_init( ALIYUN_IOT_MUTEX_S *mutex )
{
	if(NULL == mutex)
	{
		IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
	}
	
	return mico_rtos_init_mutex(&mutex->micoMutex);
}

INT32 aliyun_iot_mutex_destory( ALIYUN_IOT_MUTEX_S *mutex )
{
	if(NULL == mutex)
	{
		IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
	}

	return mico_rtos_deinit_mutex(&mutex->micoMutex);
}

INT32 aliyun_iot_mutex_lock( ALIYUN_IOT_MUTEX_S *mutex )
{
	if(NULL == mutex)
	{
		IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
	}

	return mico_rtos_lock_mutex(&mutex->micoMutex);
}


INT32 aliyun_iot_mutex_unlock( ALIYUN_IOT_MUTEX_S *mutex )
{
	if(NULL == mutex)
	{
		IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
	}

	return mico_rtos_unlock_mutex(&mutex->micoMutex);
}

INT32 aliyun_iot_pthread_param_set(ALIYUN_IOT_PTHREAD_PARAM_S *param, INT8 *threadName, UINT32 stackSize, UINT32 threadPriority)
{
    strncpy(param->threadName, threadName, THREAD_NAME_LEN);
    param->stackDepth = stackSize;
    param->priority = threadPriority;
    return SUCCESS_RETURN;
}

#define IOT_MICO_THREAD_STACK 8*1024
INT32 aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S* handle,void*(*func)(void*),void *arg,ALIYUN_IOT_PTHREAD_PARAM_S*param)
{
	if(NULL == handle)
	{
		IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
	}
	mico_thread_function_t micoThreadFun = (mico_thread_function_t)func;

#ifndef MICO_SDK_V3
	if(NULL != param)
	{
		mico_rtos_create_thread(&handle->micoThread, MICO_APPLICATION_PRIORITY, param->threadName, micoThreadFun, param->stackDepth, arg);
	}
	else
	{
		mico_rtos_create_thread(&handle->micoThread, MICO_APPLICATION_PRIORITY, "mqtt_thread", micoThreadFun, IOT_MICO_THREAD_STACK, arg);
	}

#else
	if(NULL != param)
	{
		mico_rtos_create_thread(&handle->micoThread, MICO_APPLICATION_PRIORITY, param->threadName, micoThreadFun, param->stackDepth, (unsigned int)arg);
	}
	else
	{
		mico_rtos_create_thread(&handle->micoThread, MICO_APPLICATION_PRIORITY, "mqtt_thread", micoThreadFun, IOT_MICO_THREAD_STACK, (unsigned int)arg);
	}
#endif

	if(NULL == handle->micoThread)
	{
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);

}

INT32 aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S*handle)
{
	if(NULL == handle)
	{
		IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
	}
	
	mico_rtos_delete_thread(&handle->micoThread);
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);

}

INT32 aliyun_iot_pthread_taskdelay(int MsToDelay)
{
	int secondDelay = (MsToDelay/1000 == 0) ? 1: (MsToDelay/1000);

	mico_thread_sleep(secondDelay);
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_pthread_setname(char* name)
{
    return SUCCESS_RETURN;
}

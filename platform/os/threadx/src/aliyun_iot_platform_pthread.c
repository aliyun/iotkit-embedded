#include "aliyun_iot_platform_pthread.h"

INT32 aliyun_iot_mutex_init( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL )
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

	mutex->pMutex = (TX_MUTEX *)aliyun_iot_memory_malloc(sizeof(TX_MUTEX));
	if(NULL == mutex->pMutex)
	{
		WRITE_IOT_ERROR_LOG("threadx:malloc TX_MUTEX failed");
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}
	
	tx_mutex_create(mutex->pMutex, "threadx_mutex", TX_NO_INHERIT);
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);

}

INT32 aliyun_iot_mutex_destory( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL)
    {
    	IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

    tx_mutex_delete(mutex->pMutex);

	if(NULL != mutex->pMutex)
	{
		aliyun_iot_memory_free(mutex->pMutex);
	}

    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_mutex_lock( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL)
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

	int status = tx_mutex_get(mutex->pMutex, TX_WAIT_FOREVER);
	if (status != TX_SUCCESS) 
	{
		WRITE_IOT_ERROR_LOG("threadx:lock mutex failed \n");
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	 }

    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}


INT32 aliyun_iot_mutex_unlock( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL)
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

	int status =  tx_mutex_put(mutex->pMutex);
  	if (status != TX_SUCCESS) 
	{
    	WRITE_IOT_ERROR_LOG("threadx:unlock mutex failed \n");
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
  	}
	
    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_pthread_param_set(ALIYUN_IOT_PTHREAD_PARAM_S *param, INT8 *threadName, UINT32 stackSize, UINT32 threadPriority)
{
    strncpy(param->threadName, threadName, THREAD_NAME_LEN);
    param->stackDepth = stackSize;
    param->priority = threadPriority;
    return SUCCESS_RETURN;
}

#define MQTT_TASK_STACK_SIZE 8192
INT32 aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S *handle, void*(*func)(void*), void *arg, ALIYUN_IOT_PTHREAD_PARAM_S *param)
{
	int ret = 0;
	
	if(NULL == handle)
	{
		IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
	}

	handle->pThreadID = (TX_THREAD *)aliyun_iot_memory_malloc(sizeof(TX_THREAD));
	if(NULL == handle->pThreadID)
	{
		WRITE_IOT_ERROR_LOG("threadx:malloc TX_THREAD failed");
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}

	handle->pThreadStack = aliyun_iot_memory_malloc(MQTT_TASK_STACK_SIZE);
	if(NULL == handle->pThreadStack)
	{
		WRITE_IOT_ERROR_LOG("threadx:malloc thread stack failed");
		aliyun_iot_memory_free(handle->pThreadID);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}
	
	if(NULL == param)
	{
		ret = tx_thread_create(handle->pThreadID, "threadx", (void(*)(ULONG))(func), (ULONG) arg, handle->pThreadStack, MQTT_TASK_STACK_SIZE, 16, 16, 4, TX_AUTO_START);
		if(0 != ret)
		{
			WRITE_IOT_ERROR_LOG("threadx: create thread failed ret = %d", ret);
			aliyun_iot_memory_free(handle->pThreadID);
			aliyun_iot_memory_free(handle->pThreadStack);
			IOT_FUNC_EXIT_RC(FAIL_RETURN);
		}
	}
	else
	{
		ret = tx_thread_create(handle->pThreadID, param->threadName, (void(*)(ULONG))(func), (ULONG) arg, handle->pThreadStack, param->stackDepth, 16, 16, 4, TX_AUTO_START);
		if(0 != ret)
		{
			WRITE_IOT_ERROR_LOG("threadx: create thread failed ret = %d", ret);
			aliyun_iot_memory_free(handle->pThreadID);
			aliyun_iot_memory_free(handle->pThreadStack);
			IOT_FUNC_EXIT_RC(FAIL_RETURN);
		}
	}

    return SUCCESS_RETURN;
}

INT32 aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S *handle)
{
	int ret = 0;
	
	if(NULL == handle)
	{
		IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
	}
	
	ret = tx_thread_terminate(handle->pThreadID);
	if(ret != 0)
	{
		WRITE_IOT_ERROR_LOG("threadx: terminate thread failed ret = %d", ret);
	}
	
    ret = tx_thread_delete(handle->pThreadID);
	if(ret != 0)
	{
		WRITE_IOT_ERROR_LOG("threadx: delete thread failed ret = %d", ret);
	}
	
	if(NULL != handle->pThreadID)
	{
		aliyun_iot_memory_free(handle->pThreadID);
		handle->pThreadID = NULL;
	}

	if(NULL != handle->pThreadStack)
	{
		aliyun_iot_memory_free(handle->pThreadStack);
		handle->pThreadStack = NULL;
	}
	
	return SUCCESS_RETURN;
}

INT32 aliyun_iot_pthread_taskdelay(int MsToDelay)
{
	tx_thread_sleep(MsToDelay);
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}


INT32 aliyun_iot_pthread_setname(char* name)
{
    return SUCCESS_RETURN;
}

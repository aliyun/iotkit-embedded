#include "aliyun_iot_platform_pthread.h"
#include <string.h>

INT32 aliyun_iot_mutex_init( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL )
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }
	
    mutex->mutex = xSemaphoreCreateMutex();
    mutex->is_valid = mutex->mutex != NULL;
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);

}

INT32 aliyun_iot_mutex_destory( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL || ! mutex->is_valid )
    {
    	IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

    (void) vSemaphoreDelete(mutex->mutex);

    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_mutex_lock( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL || ! mutex->is_valid )
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

    if( xSemaphoreTake( mutex->mutex, portMAX_DELAY ) != pdTRUE )
    {
        IOT_FUNC_EXIT_RC(FAIL_RETURN);
    }

    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}


INT32 aliyun_iot_mutex_unlock( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL || ! mutex->is_valid )
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }
	
    if( xSemaphoreGive( mutex->mutex ) != pdTRUE )
    {
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

INT32 aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S* handle, void*(*func)(void*), void *arg, ALIYUN_IOT_PTHREAD_PARAM_S*param)
{
	if(NULL == param)
	{
	    if(pdTRUE != xTaskCreate((void(*)(void*))func, "mqtt_thread", 2048, arg, 1, &handle->threadID))
	    {
	        return FAIL_RETURN;
	    }
	}
	else
	{
	    if(pdTRUE != xTaskCreate((void(*)(void*))func, param->threadName, param->stackDepth/4, arg, 1, &handle->threadID))
	    {
	        return FAIL_RETURN;
	    }
	}

    return SUCCESS_RETURN;
}

INT32 aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S*handle)
{
    vTaskDelete(handle->threadID);
	return SUCCESS_RETURN;
}

INT32 aliyun_iot_pthread_taskdelay(int MsToDelay)
{
	vTaskDelay(MsToDelay);
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

/***********************************************************
* 函数名称: aliyun_iot_pthread_setname
* 描       述: 设置线程名字
* 输入参数: char* name
* 输出参数:
* 返 回  值:
* 说       明:
************************************************************/
INT32 aliyun_iot_pthread_setname(char* name)
{
    return SUCCESS_RETURN;
}


/*
INT32 aliyun_iot_pthread_get_stack_size()
{
     return uxTaskGetStackHighWaterMark(NULL);
}
*/

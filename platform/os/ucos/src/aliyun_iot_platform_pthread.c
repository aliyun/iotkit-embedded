#include "aliyun_iot_platform_pthread.h"
#include <string.h>

INT32 aliyun_iot_pthread_param_set(ALIYUN_IOT_PTHREAD_PARAM_S *param, INT8 *threadName, UINT32 stackSize, UINT32 threadPriority)
{
    strncpy(param->threadName, threadName, THREAD_NAME_LEN);
    param->stackDepth = stackSize;
    param->priority = threadPriority;
    return SUCCESS_RETURN;
}

#ifndef IOT_SDK_PLATFORM_UCOSIII

#define  IOT_MQTT_TASK_STK_SIZE      3072
#define  IOT_MQTT_TASK_PRIO                 (OS_TASK_TMR_PRIO + 22)
#define  IOT_MQTT_MUTEX_PRIO                 (OS_TASK_TMR_PRIO + 15)


static UINT8 taskPri = IOT_MQTT_TASK_PRIO;
static UINT8 mutexPri = IOT_MQTT_MUTEX_PRIO;

INT32 aliyun_iot_mutex_init( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL )
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }
	
	UINT8 Err = 0;
    
    mutex->osEvent = OSMutexCreate(mutexPri, &Err);

	if(OS_ERR_NONE != Err)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-II:create mutex failed: %d", Err);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}

	mutexPri--;
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);

}

INT32 aliyun_iot_mutex_destory( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL )
    {
    	IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }
	
	UINT8 Opt = OS_DEL_NO_PEND;
	UINT8 Err = 0;
    
    OSMutexDel(mutex->osEvent, Opt, &Err);

	if(OS_ERR_NONE != Err)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-II:destory mutex failed: %d", Err);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}

    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_mutex_lock( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL )
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }
	
	UINT32 Timeout = 0;
    UINT8 Err = 0; 

	/*wait forever*/
    OSMutexPend(mutex->osEvent, Timeout, &Err);

	if(OS_ERR_NONE != Err)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-II:lock mutex failed: %d", Err);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}

    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}


INT32 aliyun_iot_mutex_unlock( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL )
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }
	
	UINT8 ret = 0;
	
	ret = OSMutexPost(mutex->osEvent);

	if(OS_ERR_NONE != ret)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-II:unlock mutex failed: %d", ret);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}
	
    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S* handle, void*(*func)(VOID*), void *arg, ALIYUN_IOT_PTHREAD_PARAM_S*param)
{
	UINT8 ret = 0;
	UINT8 taskName[16];
	
	OS_STK  *IoTAppStartTaskStack = (OS_STK *)aliyun_iot_memory_malloc(IOT_MQTT_TASK_STK_SIZE);
	if(NULL == IoTAppStartTaskStack)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-II:malloc IoTAppTaskStack failed");
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}
	
    ret = OSTaskCreate((void(*)(void*))(func), arg, &IoTAppStartTaskStack[IOT_MQTT_TASK_STK_SIZE -1], taskPri); 
	if(0 != ret)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-II:TaskCreate failed: %d", ret);
		aliyun_iot_memory_free(IoTAppStartTaskStack);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}

	snprintf(taskName, 16, "iot_thread_%d", taskPri);
	OSTaskNameSet(taskPri, taskName, &ret);
	if(0 != ret)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-II:TaskNameSet failed: %d", ret);
		aliyun_iot_memory_free(IoTAppStartTaskStack);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}

	handle->taskPri = taskPri;	
	handle->osSTK = IoTAppStartTaskStack;
	taskPri++;
    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S*handle)
{
	UINT8 ret = 0;
    ret = OSTaskDel(handle->taskPri);
	
	if(0 != ret)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-II:OSTaskDel failed: %d", ret);
		if(NULL != handle->osSTK)
		{
			aliyun_iot_memory_free(handle->osSTK);
		}
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}

	/*release memory*/
	if(NULL != handle->osSTK)
	{
		aliyun_iot_memory_free(handle->osSTK);
	}
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

UINT32 tickRateMS = 1000/OS_TICKS_PER_SEC;

INT32 aliyun_iot_pthread_taskdelay(int MsToDelay)
{
	UINT32 msToTick = (MsToDelay/tickRateMS == 0) ? 1: (MsToDelay/tickRateMS);

	OSTimeDly(msToTick);
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_pthread_setname(char* name)
{
    return SUCCESS_RETURN;
}

#else

INT32 aliyun_iot_mutex_init( ALIYUN_IOT_MUTEX_S *mutex )
{
    OS_ERR     err;
	
    if( mutex == NULL )
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

	OSMutexCreate ((OS_MUTEX  *)&mutex->osMutex,
                   (CPU_CHAR  *)"mutex",
                   (OS_ERR    *)&err);

    if (OS_ERR_NONE != err) 
	{
		WRITE_IOT_ERROR_LOG("uC/OS-III: mutex create failed: %d", err);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
    }

    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_mutex_destory( ALIYUN_IOT_MUTEX_S *mutex )
{
	OS_ERR err = 0;
	
    if( mutex == NULL )
    {
    	IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }
    
    OSMutexDel(&mutex->osMutex, OS_OPT_DEL_ALWAYS, &err);

	if(OS_ERR_NONE != err)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-III: destory mutex failed: %d", err);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}

    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_mutex_lock( ALIYUN_IOT_MUTEX_S *mutex )
{
    OS_ERR err;
	CPU_TS ts;

	if( mutex == NULL )
    {
    	IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }
		
    OSMutexPend ((OS_MUTEX  *)&mutex->osMutex,
                 (OS_TICK    )0,
                 (OS_OPT     )OS_OPT_PEND_BLOCKING,
                 (CPU_TS    *)&ts,
                 (OS_ERR    *)&err);	

    if (OS_ERR_NONE != err) 
	{
		WRITE_IOT_ERROR_LOG("uC/OS-III: mutex lock failed: %d", err);
       IOT_FUNC_EXIT_RC(FAIL_RETURN);
    }

   IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}


INT32 aliyun_iot_mutex_unlock( ALIYUN_IOT_MUTEX_S *mutex )
{
    if( mutex == NULL )
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

	OS_ERR err = 0;
	
	OSMutexPost((OS_MUTEX  *)&mutex->osMutex, 
				(OS_OPT     )OS_OPT_POST_NONE, 
				(OS_ERR    *)&err);

	if(OS_ERR_NONE != err)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-III: unlock mutex failed: %d", err);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}
	
    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

#define IOT_MQTT_TASK_STK_SIZE      3072  
#define IOT_MQTT_TASK_PRIO          (OS_CFG_PRIO_MAX - 3u)

INT32 aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S* handle, void*(*func)(void*), void *arg, ALIYUN_IOT_PTHREAD_PARAM_S*param)
{
	OS_ERR err = 0;

	OS_TCB *pIoTAppTaskTCB = (OS_TCB *)aliyun_iot_memory_malloc(sizeof(OS_TCB));
	if(NULL == pIoTAppTaskTCB)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-III: malloc OS_TCB failed");
		return FAIL_RETURN;
	}
	
	CPU_STK	*pIoTAppTaskStk = (CPU_STK *)aliyun_iot_memory_malloc(IOT_MQTT_TASK_STK_SIZE);
	if(NULL == pIoTAppTaskStk)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-III: malloc CPU_STK failed");
		aliyun_iot_memory_free(pIoTAppTaskTCB);
		return FAIL_RETURN;
	}
	
	OSTaskCreate((OS_TCB       *)pIoTAppTaskTCB,             
             (CPU_CHAR     *)"IoT_Thread",
             (OS_TASK_PTR   )func, 
             (void         *)arg,
             (OS_PRIO       )IOT_MQTT_TASK_PRIO,
             (CPU_STK      *)&pIoTAppTaskStk[0],
             (CPU_STK_SIZE  )IOT_MQTT_TASK_STK_SIZE / 10,
             (CPU_STK_SIZE  )IOT_MQTT_TASK_STK_SIZE,
             (OS_MSG_QTY    )0,
             (OS_TICK       )0,
             (void         *)0,
             (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
             (OS_ERR       *)&err);

	if(OS_ERR_NONE != err)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-III: create task failed %d", err);
		aliyun_iot_memory_free(pIoTAppTaskTCB);
		aliyun_iot_memory_free(pIoTAppTaskStk);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}

	handle->taskTcb = pIoTAppTaskTCB;
	handle->cpuStk = pIoTAppTaskStk;

    IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S*handle)
{
	OS_ERR err = 0;
	
    OSTaskDel((OS_TCB  *)handle->taskTcb, 
			 (OS_ERR  *)&err);
	
	if(OS_ERR_NONE != err)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-III: task del failed: %d", err);
		if(NULL != handle->taskTcb)
		{
			aliyun_iot_memory_free(handle->taskTcb);
		}

		if(NULL != handle->cpuStk)
		{
			aliyun_iot_memory_free(handle->cpuStk);
		}
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}

	/*release memory*/
	if(NULL != handle->taskTcb)
	{
		aliyun_iot_memory_free(handle->taskTcb);
	}

	if(NULL != handle->cpuStk)
	{
		aliyun_iot_memory_free(handle->cpuStk);
	}
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

INT32 aliyun_iot_pthread_taskdelay(int MsToDelay)
{
	OS_ERR err = 0;
	
	OSTimeDly((OS_TICK)MsToDelay, 
		(OS_OPT)OS_OPT_TIME_DLY ,
		(OS_ERR *)&err);

	if(OS_ERR_NONE != err)
	{
		WRITE_IOT_ERROR_LOG("uC/OS-III: task delay failed: %d", err);
		IOT_FUNC_EXIT_RC(FAIL_RETURN);
	}
	
	IOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

#endif


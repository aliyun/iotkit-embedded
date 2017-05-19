#include "aliyun_iot_platform_threadsync.h"

INT32 aliyun_iot_sem_init( ALIYUN_IOT_SEM_S * sem)
{
    sem->sem = xSemaphoreCreateBinary();
    sem->is_valid = sem->sem != NULL;
	
    return (SUCCESS_RETURN);
}

INT32 aliyun_iot_sem_destory( ALIYUN_IOT_SEM_S * sem)
{
    (void) vSemaphoreDelete(sem->sem);

    return (SUCCESS_RETURN);
}


INT32 aliyun_iot_sem_gettimeout( ALIYUN_IOT_SEM_S *sem, UINT32 time_ms)
{
    if( !sem->is_valid )
	{
        return (ERROR_NULL_VALUE);
	}

	if( xSemaphoreTake( sem->sem, time_ms ) != SUCCESS_RETURN )
    {
	    return (ERROR_NET_TIMEOUT);
	}

	return (SUCCESS_RETURN);
}

INT32 aliyun_iot_sem_post( ALIYUN_IOT_SEM_S * sem)
{
    if(!sem->is_valid )
    {
        return (ERROR_NULL_VALUE);
    }

    if( xSemaphoreGive( sem->sem ) != SUCCESS_RETURN )
    {
        return (FAIL_RETURN);
    }

    return (SUCCESS_RETURN);
}



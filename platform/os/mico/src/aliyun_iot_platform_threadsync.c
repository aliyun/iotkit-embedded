#include "aliyun_iot_platform_threadsync.h"

INT32 aliyun_iot_sem_init( ALIYUN_IOT_SEM_S * sem)
{
    if( sem == NULL )
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }
	
	return mico_rtos_init_semaphore(&sem->micoSem, 1);
}

INT32 aliyun_iot_sem_destory( ALIYUN_IOT_SEM_S * sem)
{
    if( sem == NULL )
    {
        IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

	return mico_rtos_deinit_semaphore(&sem->micoSem);
}


INT32 aliyun_iot_sem_gettimeout( ALIYUN_IOT_SEM_S *sem, UINT32 time_ms)
{
    if( sem == NULL)
	{
		IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
	}

	return mico_rtos_get_semaphore( &sem->micoSem, time_ms );
}

INT32 aliyun_iot_sem_post( ALIYUN_IOT_SEM_S * sem)
{
    if( sem == NULL)
    {
		IOT_FUNC_EXIT_RC(ERROR_NULL_VALUE);
    }

	return mico_rtos_set_semaphore(&sem->micoSem);
}



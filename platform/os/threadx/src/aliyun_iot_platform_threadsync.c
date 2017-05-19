
#include "aliyun_iot_platform_threadsync.h"

INT32 aliyun_iot_sem_init(ALIYUN_IOT_SEM_S *pSemIoT)
{
	int ret = -1;
	pSemIoT->pSem = (TX_SEMAPHORE *)aliyun_iot_memory_malloc(sizeof(TX_SEMAPHORE));
	if(NULL == pSemIoT->pSem)
	{
		WRITE_IOT_ERROR_LOG("malloc tx sem failed");
		return -1;
	}
	
	ret = tx_semaphore_create(pSemIoT->pSem, "sem_iot", 0);
	if(ret != 0)
	{
		WRITE_IOT_ERROR_LOG("threadx create sem failed");
		aliyun_iot_memory_free(pSemIoT->pSem);
		pSemIoT->pSem = NULL;
	}
	return ret;
}

INT32 aliyun_iot_sem_destory(ALIYUN_IOT_SEM_S *pSemIoT)
{
	int ret = -1;

	ret = tx_semaphore_delete(pSemIoT->pSem);
	if(ret != 0)
	{
		WRITE_IOT_ERROR_LOG("threadx delete sem failed");
	}

	if(NULL != pSemIoT->pSem)
	{
		aliyun_iot_memory_free(pSemIoT->pSem);
		pSemIoT->pSem = NULL;
	}

	return ret;
}

INT32 aliyun_iot_sem_gettimeout(ALIYUN_IOT_SEM_S *pSemIoT, UINT32 timeout_ms)
{
	int ret = -1;
	ret = tx_semaphore_get(pSemIoT->pSem, timeout_ms);
	if(ret != 0)
	{
		//WRITE_IOT_ERROR_LOG("threadx get sem failed");
	}
	return ret;
}

INT32 aliyun_iot_sem_post(ALIYUN_IOT_SEM_S *pSemIoT)
{
	int ret = -1;

	ret = tx_semaphore_put(pSemIoT->pSem);
	if(ret != 0)
	{
		WRITE_IOT_ERROR_LOG("threadx put sem failed");
	}
	return ret;
}



#include "aliyun_iot_platform_threadsync.h"

#ifndef IOT_SDK_PLATFORM_UCOSIII

INT32 aliyun_iot_sem_init(ALIYUN_IOT_SEM_S *pSemIoT)
{
	pSemIoT->pOsEvent = OSSemCreate(0);
	if(NULL == pSemIoT->pOsEvent)
	{
		WRITE_IOT_ERROR_LOG("ucos-II: create sem failed");
		return -1;
	}
	return 0;
}

INT32 aliyun_iot_sem_destory(ALIYUN_IOT_SEM_S *pSemIoT)
{
	UINT8 ucErr = 0;
    OSSemDel(pSemIoT->pOsEvent, OS_DEL_ALWAYS, &ucErr);
	if(ucErr != 0)
	{
		WRITE_IOT_ERROR_LOG("ucos-II: delete sem failed error:%d", ucErr);
	}
	return ucErr;
}

INT32 aliyun_iot_sem_gettimeout(ALIYUN_IOT_SEM_S *pSemIoT, UINT32 timeout_ms)
{
    UINT8 ucErr = 0;
    UINT32 timeout_wait = 0;
    
    if(timeout_ms != 0)
    {
        timeout_wait = (timeout_ms * OS_TICKS_PER_SEC) / 1000;
        if(timeout_wait < 1)
        {
            timeout_wait = 1;
        }
        else if(timeout_wait > 65536)
        {
            timeout_wait = 65535;
        }
    }
    else
    {
        timeout_wait = 0;
    }
        
    OSSemPend(pSemIoT->pOsEvent, (UINT16)timeout_wait, &ucErr);
	if(ucErr != 0)
	{
		WRITE_IOT_ERROR_LOG("ucos-II: sem get failed err:%d", ucErr);
	}

	return ucErr;

}

INT32 aliyun_iot_sem_post(ALIYUN_IOT_SEM_S *pSemIoT)
{
	UINT8 ret = 0;
	ret = OSSemPost(pSemIoT->pOsEvent);
	if(ret != 0)
	{
		WRITE_IOT_ERROR_LOG("ucos-II: sem post failed err:%d", ret);
	}
	return ret;
}

#else

INT32 aliyun_iot_sem_init(ALIYUN_IOT_SEM_S *pSemIoT)
{
	OS_ERR err = 0;	
	
	pSemIoT->pOsSem = (OS_SEM *)aliyun_iot_memory_malloc(sizeof(OS_SEM));
	if(NULL == pSemIoT->pOsSem)
	{
		WRITE_IOT_ERROR_LOG("ucos-III:malloc OS_SEM failed");
		return -1;
	}
	
    OSSemCreate(pSemIoT->pOsSem, "iot_sem", 0, &err);
    if(0 != err)
    {
    	WRITE_IOT_ERROR_LOG("ucos-III:create sem failed err:%d", err);
		aliyun_iot_memory_free(pSemIoT->pOsSem);
		pSemIoT->pOsSem = NULL;
   	} 

	return (INT32)err;
}

INT32 aliyun_iot_sem_destory(ALIYUN_IOT_SEM_S *pSemIoT)
{
	OS_ERR err = 0;
    OSSemDel(pSemIoT->pOsSem, OS_OPT_DEL_ALWAYS, &err);
	if(err != 0)
	{
		WRITE_IOT_ERROR_LOG("ucos-III: delete sem failed err:%d", err);
	}

	if(NULL != pSemIoT->pOsSem)
	{
		aliyun_iot_memory_free(pSemIoT->pOsSem);
		pSemIoT->pOsSem = NULL;
	}
	return (INT32)err;
}

INT32 aliyun_iot_sem_gettimeout(ALIYUN_IOT_SEM_S *pSemIoT, INT32 timeout_ms)
{
	OS_ERR err = 0;
	UINT32 ts = 0;
    UINT32 timeout_wait = 0;
    
    if(timeout_ms != 0)
    {
        timeout_wait = (timeout_ms * OS_CFG_TICK_RATE_HZ) / 1000;
        if(timeout_wait < 1)
        {
            timeout_wait = 1;
        }
        else if(timeout_wait > 65536)
        {
            timeout_wait = 65535;
        }
    }
    else
    {
        timeout_wait = 0;
    }

	OSSemPend(pSemIoT->pOsSem, (UINT16)timeout_wait, OS_OPT_PEND_BLOCKING, &ts, &err);
	if(0 != err)
	{
		WRITE_IOT_ERROR_LOG("ucos-III: sem get failed err:%d", err);
	}

	return (INT32)err;
	
}


INT32 aliyun_iot_sem_post(ALIYUN_IOT_SEM_S *pSemIoT)
{
 	OS_ERR err = 0;
    OSSemPost(pSemIoT->pOsSem, OS_OPT_POST_ALL, &err);
	if(0 != err)
	{
		WRITE_IOT_ERROR_LOG("ucos-III: sem post failed err:%d", err);
	}

	return (INT32)err;
}

#endif

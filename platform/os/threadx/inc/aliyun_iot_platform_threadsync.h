
#ifndef ALIYUN_IOT_PLATFORM_THREADSYNC_H
#define ALIYUN_IOT_PLATFORM_THREADSYNC_H

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_platform_memory.h"

#include "threadx/tx_api.h"

typedef struct ALIYUN_IOT_SEM
{
	TX_SEMAPHORE *pSem;
}ALIYUN_IOT_SEM_S;

INT32 aliyun_iot_sem_init(ALIYUN_IOT_SEM_S *pSemIoT);

INT32 aliyun_iot_sem_destory(ALIYUN_IOT_SEM_S *pSemIoT);

INT32 aliyun_iot_sem_gettimeout(ALIYUN_IOT_SEM_S *pSemIoT, UINT32 timeout_ms);

INT32 aliyun_iot_sem_post(ALIYUN_IOT_SEM_S *pSemIoT);

#endif

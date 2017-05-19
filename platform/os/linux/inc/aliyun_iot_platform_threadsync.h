#ifndef ALIYUN_IOT_PLATFORM_THREADSYNC_H
#define ALIYUN_IOT_PLATFORM_THREADSYNC_H

#include <pthread.h>
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"

typedef struct ALIYUN_IOT_SEM
{
    INT32           count;
    pthread_mutex_t lock;
    pthread_cond_t  sem;
}ALIYUN_IOT_SEM_S;

/***********************************************************
* 函数名称: aliyun_iot_sem_init
* 描       述: 线程同步信号初始化
* 输入参数: ALIYUN_IOT_SEM_S*semaphore
* 输出参数:
* 返 回  值: 0：成功，
*          FAIL_RETURN：异常
* 说       明:
************************************************************/
INT32 aliyun_iot_sem_init(ALIYUN_IOT_SEM_S *semaphore);

/***********************************************************
* 函数名称: aliyun_iot_sem_destory
* 描       述: 线程同步信号资源释放
* 输入参数: ALIYUN_IOT_SEM_S*semaphore
* 输出参数:
* 返 回  值: 0：成功，
*          FAIL_RETURN：异常
* 说       明:
************************************************************/
INT32 aliyun_iot_sem_destory(ALIYUN_IOT_SEM_S *semaphore);

/***********************************************************
* 函数名称: aliyun_iot_sem_gettimeout
* 描       述: 等待同步信号
* 输入参数: ALIYUN_IOT_SEM_S*semaphore
*          INT32 timeout_ms
* 输出参数:
* 返 回  值: 0：成功，
*          ERROR_NET_TIMEOUT：等待超时，
* 说       明: 等待同步信号，超时退出
************************************************************/
INT32 aliyun_iot_sem_gettimeout(ALIYUN_IOT_SEM_S*semaphore,UINT32 timeout_ms);

/***********************************************************
* 函数名称: aliyun_iot_sem_post
* 描       述: 发送同步信号
* 输入参数: ALIYUN_IOT_SEM_S*semaphore
* 输出参数:
* 返 回  值:
* 说       明: 发送同步信号
************************************************************/
INT32 aliyun_iot_sem_post(ALIYUN_IOT_SEM_S*semaphore);

#endif


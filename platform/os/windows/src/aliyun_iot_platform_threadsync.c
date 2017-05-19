#include "../../os/windows/inc/aliyun_iot_platform_threadsync.h"

/***********************************************************
* 函数名称: aliyun_iot_sem_init
* 描       述: 线程同步信号初始化
* 输入参数: ALIYUN_IOT_SEM_S*semaphore
* 输出参数:
* 返 回  值: 0：成功，
*          FAIL_RETURN：异常
* 说       明:
************************************************************/
int32_t aliyun_iot_sem_init(ALIYUN_IOT_SEM_S *semaphore)
{
    semaphore->hSemaphore = CreateSemaphore(NULL, 1, 1, NULL);

    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliyun_iot_sem_destory
* 描       述: 线程同步信号资源释放
* 输入参数: ALIYUN_IOT_SEM_S*semaphore
* 输出参数:
* 返 回  值: 0：成功，
*          FAIL_RETURN：异常
* 说       明:
************************************************************/
int32_t aliyun_iot_sem_destory(ALIYUN_IOT_SEM_S *semaphore)
{
    CloseHandle(semaphore->hSemaphore);
    return SUCCESS_RETURN;
}

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
int32_t aliyun_iot_sem_gettimeout(ALIYUN_IOT_SEM_S*semaphore,uint32_t timeout_ms)
{
    WaitForSingleObject(semaphore->hSemaphore,timeout_ms);
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliyun_iot_sem_post
* 描       述: 发送同步信号
* 输入参数: ALIYUN_IOT_SEM_S*semaphore
* 输出参数:
* 返 回  值:
* 说       明: 发送同步信号
************************************************************/
int32_t aliyun_iot_sem_post(ALIYUN_IOT_SEM_S*semaphore)
{
    ReleaseSemaphore(semaphore->hSemaphore, 1, NULL);
    return SUCCESS_RETURN;
}


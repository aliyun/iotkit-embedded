/*********************************************************************************
 * 文件名称: aliyun_iot_platform_threadsync.c
 * 作       者:
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/

#include <sys/time.h>
#include <errno.h>
#include "aliyun_iot_platform_threadsync.h"

/***********************************************************
* 函数名称: aliyun_iot_sem_init
* 描       述: 线程同步信号初始化
* 输入参数: ALIYUN_IOT_SEM_S*semaphore
* 输出参数:
* 返 回  值: 0：成功，
*          FAIL_RETURN：异常
* 说       明:
************************************************************/
INT32 aliyun_iot_sem_init(ALIYUN_IOT_SEM_S *semaphore)
{
    int result = 0;

    result = pthread_mutex_init(&semaphore->lock,NULL);
    if(0 != result)
    {
        return FAIL_RETURN;
    }

    result = pthread_cond_init(&semaphore->sem,NULL);
    if(0 != result)
    {
        return FAIL_RETURN;
    }

    semaphore->count = 0;

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
INT32 aliyun_iot_sem_destory(ALIYUN_IOT_SEM_S *semaphore)
{
    INT32 result = 0;

    result = pthread_mutex_destroy(&semaphore->lock);
    if(0!=result)
    {
        return FAIL_RETURN;
    }

    result = pthread_cond_destroy(&semaphore->sem);
    if(0!=result)
    {
        return FAIL_RETURN;
    }

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
INT32 aliyun_iot_sem_gettimeout(ALIYUN_IOT_SEM_S*semaphore,UINT32 timeout_ms)
{
    int result = 0;
    int rc = 0;
    struct timespec timeout;
    struct timeval now;

    pthread_mutex_lock(&semaphore->lock);

    gettimeofday(&now, NULL);
    unsigned long nsec = (now.tv_usec + timeout_ms%1000 * 1000)*1000;
    timeout.tv_sec = now.tv_sec + timeout_ms/1000 + nsec/1000000000;
    timeout.tv_nsec = nsec % 1000000000;

    while(0 == semaphore->count)
    {
        result = pthread_cond_timedwait(&semaphore->sem,&semaphore->lock,&timeout);
        if(ETIMEDOUT == result)
        {
            //超时则退出等待
            break;
        }
        else if(0 == result)
        {
            //等到信号
            continue;
        }
        else
        {
            //异常退出
            break;
        }
    }

    if(0 == result)
    {
        rc = SUCCESS_RETURN;
        semaphore->count--;
    }
    else if(ETIMEDOUT == result)
    {
        rc = ERROR_NET_TIMEOUT;
    }
    else
    {
        rc = FAIL_RETURN;
    }

    pthread_mutex_unlock(&semaphore->lock);

    return rc;
}

/***********************************************************
* 函数名称: aliyun_iot_sem_post
* 描       述: 发送同步信号
* 输入参数: ALIYUN_IOT_SEM_S*semaphore
* 输出参数:
* 返 回  值:
* 说       明: 发送同步信号
************************************************************/
INT32 aliyun_iot_sem_post(ALIYUN_IOT_SEM_S*semaphore)
{
    pthread_mutex_lock(&semaphore->lock);
    semaphore->count++;
    pthread_cond_signal(&semaphore->sem);
    pthread_mutex_unlock(&semaphore->lock);
    return SUCCESS_RETURN;
}

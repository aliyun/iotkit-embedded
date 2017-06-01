/*********************************************************************************
 * 文件名称: aliyun_iot_platform_pthread.c
 * 作       者:
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/
#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_platform_pthread.h"
#include <string.h>

/***********************************************************
* 函数名称: aliyun_iot_mutex_init
* 描       述: 互斥初始化
* 输入参数: ALIYUN_IOT_MUTEX_S *mutex 互斥锁句柄
* 输出参数:
* 返 回  值: 同linux系统下的pthread_mutex_init返回值
* 说       明: linux系统下互斥锁初始化
*           源码中使用
************************************************************/
int32_t aliyun_iot_mutex_init(ALIYUN_IOT_MUTEX_S *mutex)
{
    return (int32_t)pthread_mutex_init(&(mutex->lock), NULL);
}

/***********************************************************
* 函数名称: aliyun_iot_mutex_lock
* 描       述: 互斥锁上锁
* 输入参数: ALIYUN_IOT_MUTEX_S *mutex 互斥锁句柄
* 输出参数:
* 返 回  值: 同linux系统下的aliyun_iot_mutex_lock返回值
* 说       明: linux系统下互斥锁上锁
*           源码中使用
************************************************************/
int32_t aliyun_iot_mutex_lock(ALIYUN_IOT_MUTEX_S *mutex)
{
    return (int32_t)pthread_mutex_lock(&mutex->lock);
}

/***********************************************************
* 函数名称: aliyun_iot_mutex_unlock
* 描       述: 互斥锁解锁
* 输入参数: ALIYUN_IOT_MUTEX_S *mutex 互斥锁句柄
* 输出参数:
* 返 回  值: 同linux系统下的pthread_mutex_unlock返回值
* 说       明: linux系统下解除互斥锁
*           源码中使用
************************************************************/
int32_t aliyun_iot_mutex_unlock(ALIYUN_IOT_MUTEX_S *mutex)
{
    return (int32_t)pthread_mutex_unlock(&mutex->lock);
}

/***********************************************************
* 函数名称: aliyun_iot_mutex_destory
* 描       述: 销毁互斥锁资源
* 输入参数: ALIYUN_IOT_MUTEX_S *mutex 互斥锁句柄
* 输出参数:
* 返 回  值: 同linux系统下的pthread_mutex_destroy返回值
* 说       明: linux系统下销毁一个互斥锁资源
*           源码中使用
************************************************************/
int32_t aliyun_iot_mutex_destory(ALIYUN_IOT_MUTEX_S *mutex)
{
    return (int32_t)pthread_mutex_destroy(&mutex->lock);
}

int32_t aliyun_iot_pthread_param_set(ALIYUN_IOT_PTHREAD_PARAM_S *param, int8_t *threadName, uint32_t stackSize,
                                     uint32_t threadPriority)
{
    strncpy(param->threadName, threadName, THREAD_NAME_LEN);
    param->stackDepth = stackSize;
    param->priority = threadPriority;
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliyun_iot_pthread_create
* 描       述: 创建线程接口
* 输入参数: ALIYUN_IOT_PTHREAD_S* handle 线程ID
*          void*(*func)(void*) 线程入口函数
*          void *arg 线程参数
*          ALIYUN_IOT_PTHREAD_PARAM_S*param 线程属性参数
* 输出参数:
* 返 回  值: 同linux系统下的pthread_create返回值
* 说       明: linux系统下创建一个分离线程的实现
*           源码中使用
************************************************************/
int32_t aliyun_iot_pthread_create(ALIYUN_IOT_PTHREAD_S *handle, void *(*func)(void *), void *arg,
                                  ALIYUN_IOT_PTHREAD_PARAM_S *param)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    return pthread_create(&handle->threadID, &attr, func, arg);
}

/***********************************************************
* 函数名称: aliyun_iot_pthread_cancel
* 描       述: 线程结束接口
* 输入参数: ALIYUN_IOT_PTHREAD_S*threadID 线程ID
* 输出参数:
* 返 回  值: 同linux系统下的pthread_cancel返回值
* 说       明: linux系统下关闭某一个线程的接口
*           源码中使用
************************************************************/
int32_t aliyun_iot_pthread_cancel(ALIYUN_IOT_PTHREAD_S *threadID)
{
    if (threadID->threadID != 0) {
        return pthread_cancel(threadID->threadID);
    }

    return 0;
}

/***********************************************************
* 函数名称: aliyun_iot_pthread_taskdelay
* 描       述: 睡眠延时
* 输入参数: int MsToDelay 延时的毫秒数
* 输出参数:
* 返 回  值: 同linux系统下的usleep返回值
* 说       明: linux系统下的线程睡眠
*           源码和mbedtls中使用
************************************************************/
int32_t aliyun_iot_pthread_taskdelay(int MsToDelay)
{
    return usleep(MsToDelay * 1000);
}


/***********************************************************
* 函数名称: aliyun_iot_pthread_setname
* 描       述: 设置线程名字
* 输入参数: char* name
* 输出参数:
* 返 回  值:
* 说       明:
************************************************************/
int32_t aliyun_iot_pthread_setname(char *name)
{
    prctl(PR_SET_NAME, name);
    return SUCCESS_RETURN;
}



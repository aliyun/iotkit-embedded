/*********************************************************************************
 * 文件名称: aliyun_iot_platform_timer.c
 * 作       者:
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include "aliyun_iot_platform_timer.h"

/***********************************************************
* 函数名称: aliyun_iot_timer_assignment
* 描       述: 时间变量赋值
* 输入参数: INT32 millisecond
* 输出参数: ALIYUN_IOT_TIME_TYPE_S* timer
* 返 回  值:
* 说       明: linux系统下根据设定的毫秒数，初始化时间变量
*           源码中使用
************************************************************/
void aliyun_iot_timer_assignment(INT32 millisecond,ALIYUN_IOT_TIME_TYPE_S *timer)
{
    timer->time = (struct timeval){millisecond / 1000, (millisecond % 1000) * 1000};
}

/***********************************************************
* 函数名称: aliyun_iot_timer_start_clock
* 描       述: 获得当前时间
* 输入参数:
* 输出参数: ALIYUN_IOT_TIME_TYPE_S* timer
* 返 回  值:
* 说       明: linux系统下获取当前时间，时间单位为ms，us
*           源码中使用
************************************************************/
INT32 aliyun_iot_timer_start_clock(ALIYUN_IOT_TIME_TYPE_S *timer)
{
    gettimeofday(&timer->time, NULL);

    return (INT32)SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliyun_iot_timer_spend
* 描       述: 距离起始时间已经度过的时长
* 输入参数: ALIYUN_IOT_TIME_TYPE_S* timer
* 输出参数: 距离时间
* 返 回  值:
* 说       明: linux系统下起始时间和当前时间做差
*           源码中使用
************************************************************/
INT32 aliyun_iot_timer_spend(ALIYUN_IOT_TIME_TYPE_S *start)
{
    struct timeval now, res;

    gettimeofday(&now, NULL);
    timersub(&now, &start->time, &res);
    return (res.tv_sec)*1000 + (res.tv_usec)/1000;
}

/***********************************************************
* 函数名称: aliyun_iot_timer_remain
* 描       述: 距离终止时间还有多少时长
* 输入参数: ALIYUN_IOT_TIME_TYPE_S* timer
* 输出参数: 距离时间
* 返 回  值:
* 说       明: linux系统下终止时间和当前时间做差
*           源码中使用
************************************************************/
INT32 aliyun_iot_timer_remain(ALIYUN_IOT_TIME_TYPE_S *end)
{
    struct timeval now, res;

    gettimeofday(&now, NULL);
    timersub(&end->time, &now, &res);
    return (res.tv_sec)*1000 + (res.tv_usec)/1000;
}

/***********************************************************
* 函数名称: aliyun_iot_timer_expired
* 描       述: 定时时间是否已经超时
* 输入参数: ALIYUN_IOT_TIME_TYPE_S* timer
* 输出参数:
* 返 回  值:
* 说       明: linux系统下判断定时时间是否已经超时
*           源码中使用
************************************************************/
INT32 aliyun_iot_timer_expired(ALIYUN_IOT_TIME_TYPE_S *timer)
{
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->time, &now, &res);
    return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}

/***********************************************************
* 函数名称: aliyun_iot_timer_init
* 描       述: 初始化时间变量
* 输入参数: ALIYUN_IOT_TIME_TYPE_S* timer
* 输出参数:
* 返 回  值:
* 说       明: linux系统下初始化时间变量
*           源码中使用
************************************************************/
void aliyun_iot_timer_init(ALIYUN_IOT_TIME_TYPE_S* timer)
{
    timer->time = (struct timeval){0, 0};
}

/***********************************************************
* 函数名称: aliyun_iot_timer_cutdown
* 描       述: 计算当前时间一定间隔后的时间值
* 输入参数: UINT32 millisecond 间隔时间（ms）
* 输出参数: ALIYUN_IOT_TIME_TYPE_S* timer 输出时间
* 返 回  值:
* 说       明: linux系统下实现计算在当前时间millisecond后的时间
*           源码中使用
************************************************************/
void aliyun_iot_timer_cutdown(ALIYUN_IOT_TIME_TYPE_S* timer,UINT32 millisecond)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {millisecond / 1000, (millisecond % 1000) * 1000};
    timeradd(&now, &interval, &timer->time);
}

/***********************************************************
* 函数名称: aliyun_iot_timer_now
* 描       述: 获取当前时间值
* 输入参数:
* 输出参数:
* 返 回  值: 时间值
* 说       明: linux系统下实现计算两个时间间隔
*           源码中使用
************************************************************/
UINT32 aliyun_iot_timer_now()
{
    return (UINT32) (time(NULL) * 1000);
}

/***********************************************************
* 函数名称: aliyun_iot_timer_interval
* 描       述: 时间间隔
* 输入参数: ALIYUN_IOT_TIME_TYPE_S *start 起始时间
*           ALIYUN_IOT_TIME_TYPE_S *end 终止时间
* 输出参数:
* 返 回  值: 时间间隔值
* 说       明: linux系统下实现计算两个时间间隔
*           mbedtls中使用
************************************************************/
INT32 aliyun_iot_timer_interval(ALIYUN_IOT_TIME_TYPE_S *start,ALIYUN_IOT_TIME_TYPE_S *end)
{
    return (end->time.tv_sec - start->time.tv_sec)*1000 + (end->time.tv_usec - start->time.tv_usec)/1000;
}

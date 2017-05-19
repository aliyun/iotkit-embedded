/*********************************************************************************
 * 文件名称: aliyun_iot_platform_timer.h
 * 作       者:
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/

#ifndef ALIYUN_IOT_PLATFORM_TIMER_H
#define ALIYUN_IOT_PLATFORM_TIMER_H

//#include <windows.h>
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"

typedef struct ALIYUN_IOT_TIME_TYPE
{
    long time;
}ALIYUN_IOT_TIME_TYPE_S;

void aliyun_iot_timer_assignment(int32_t millisecond,ALIYUN_IOT_TIME_TYPE_S *timer);

int32_t aliyun_iot_timer_start_clock(ALIYUN_IOT_TIME_TYPE_S *timer);

int32_t aliyun_iot_timer_spend(ALIYUN_IOT_TIME_TYPE_S *start);

int32_t aliyun_iot_timer_remain(ALIYUN_IOT_TIME_TYPE_S *end);

int32_t aliyun_iot_timer_expired(ALIYUN_IOT_TIME_TYPE_S *timer);

void aliyun_iot_timer_init(ALIYUN_IOT_TIME_TYPE_S* timer);

void aliyun_iot_timer_cutdown(ALIYUN_IOT_TIME_TYPE_S* timer,uint32_t millisecond);

uint32_t aliyun_iot_timer_now();

int32_t aliyun_iot_timer_interval(ALIYUN_IOT_TIME_TYPE_S *start,ALIYUN_IOT_TIME_TYPE_S *end);

#endif


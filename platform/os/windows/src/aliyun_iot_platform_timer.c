/*********************************************************************************
 * 文件名称: aliyun_iot_platform_timer.c
 * 作       者:
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/

#include "../../os/windows/inc/aliyun_iot_platform_timer.h"

#include <stdlib.h>
#include <windows.h>

void aliyun_iot_timer_assignment(int32_t millisecond,ALIYUN_IOT_TIME_TYPE_S *timer)
{
    timer->time = millisecond;
}

int32_t aliyun_iot_timer_start_clock(ALIYUN_IOT_TIME_TYPE_S *timer)
{
    timer->time = GetTickCount();

    return (int32_t)SUCCESS_RETURN;
}

int32_t aliyun_iot_timer_spend(ALIYUN_IOT_TIME_TYPE_S *start)
{
    DWORD now, res;

    now = GetTickCount();
    res = now - start->time;
    return res;
}

int32_t aliyun_iot_timer_remain(ALIYUN_IOT_TIME_TYPE_S *end)
{
    DWORD now, res;

    now = GetTickCount();
    res = end->time - now;
    return res;
}

int32_t aliyun_iot_timer_expired(ALIYUN_IOT_TIME_TYPE_S *timer)
{
    DWORD cur_time = 0;
    cur_time = GetTickCount();
    if (timer->time < cur_time || timer->time == cur_time)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void aliyun_iot_timer_init(ALIYUN_IOT_TIME_TYPE_S* timer)
{
    timer->time = 0;
}

void aliyun_iot_timer_cutdown(ALIYUN_IOT_TIME_TYPE_S* timer,uint32_t millisecond)
{
    timer->time = GetTickCount() + millisecond;
}

uint32_t aliyun_iot_timer_now()
{
    return (uint32_t)(GetTickCount());
}

int32_t aliyun_iot_timer_interval(ALIYUN_IOT_TIME_TYPE_S *start,ALIYUN_IOT_TIME_TYPE_S *end)
{
    return (end->time - start->time);
}



uint32_t aliot_platform_time_get_ms(void)
{
    return (uint32_t)(GetTickCount());
}


uint64_t aliot_time_get_ms_64b(void)
{
    static uint32_t overflow_cnt = 0;
    static uint32_t last;

    uint32_t new = aliot_platform_time_get_ms();

    //#BUG: it may lose if this interface is not called over 49.7 days
    if (new < last) {
        ++overflow_cnt;
    }

    return ((uint64_t)overflow_cnt << 32) | (new);
}


uint32_t aliot_time_get_ms(void)
{
    return (uint32_t)aliot_time_get_ms_64b();
}


uint32_t aliot_time_get_s(void)
{
    return (uint32_t)(aliot_time_get_ms_64b()/1000);
}

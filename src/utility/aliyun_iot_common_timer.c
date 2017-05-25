
#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_platform_timer.h"

#include "aliyun_iot_common_timer.h"

void aliyun_iot_timer_assignment(int32_t millisecond, aliot_time_t *timer)
{
    timer->time = millisecond;
}

void aliyun_iot_timer_start_clock(aliot_time_t *timer)
{
    timer->time = aliot_platform_time_get_ms();
}

uint32_t aliyun_iot_timer_spend(aliot_time_t *start)
{
    uint32_t now, res;

    now = aliot_platform_time_get_ms();
    res = now - start->time;
    return res;
}

uint32_t aliyun_iot_timer_remain(aliot_time_t *end)
{
    uint32_t now, res;

    now = aliot_platform_time_get_ms();
    res = end->time - now;
    return res;
}

uint32_t aliyun_iot_timer_expired(aliot_time_t *timer)
{
    uint32_t cur_time = 0;
    cur_time = aliot_platform_time_get_ms();

    //hanle overflow
    if ((timer->time <= cur_time) && ((cur_time - timer->time) < 0x80000000))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void aliyun_iot_timer_init(aliot_time_t *timer)
{
    timer->time = 0;
}

void aliyun_iot_timer_cutdown(aliot_time_t *timer, uint32_t millisecond)
{
    //NOTE it may overflow
    timer->time = aliot_platform_time_get_ms() + millisecond;
}

uint32_t aliyun_iot_timer_now()
{
    return (uint32_t)(aliot_platform_time_get_ms());
}

uint32_t aliyun_iot_timer_interval(aliot_time_t *start, aliot_time_t *end)
{
    return (end->time - start->time);
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

#include "aliyun_iot_platform_timer.h"

INT32 aliyun_iot_timer_start_clock(ALIYUN_IOT_TIME_TYPE_S *timer)
{
    timer->time = mico_get_time();

    return (INT32)SUCCESS_RETURN;
}


INT32 aliyun_iot_timer_spend(ALIYUN_IOT_TIME_TYPE_S *start)
{
    UINT32 now, res;

    now = mico_get_time();
    res = now - start->time;
    return res;
}

INT32 aliyun_iot_timer_remain(ALIYUN_IOT_TIME_TYPE_S *end)
{
    UINT32 now, res;

    now = mico_get_time();
    res = end->time - now;
    return res;
}

INT32 aliyun_iot_timer_expired(ALIYUN_IOT_TIME_TYPE_S *timer)
{
	unsigned int cur_time = 0;
	cur_time = mico_get_time();
	if (timer->time < cur_time || timer->time == cur_time) {
		return 1;
	} else {
		return 0;
	}
}

void aliyun_iot_timer_init(ALIYUN_IOT_TIME_TYPE_S* timer)
{
    timer->time = 0;
}

void aliyun_iot_timer_assignment(INT32 millisecond, ALIYUN_IOT_TIME_TYPE_S *timer)
{
    timer->time = millisecond;
}

void aliyun_iot_timer_cutdown(ALIYUN_IOT_TIME_TYPE_S* timer, UINT32 millisecond)
{
    timer->time = mico_get_time() + millisecond;
}

UINT32 aliyun_iot_timer_now()
{
    return mico_get_time();
}


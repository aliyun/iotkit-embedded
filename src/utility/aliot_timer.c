
#include "aliot_platform.h"
#include "aliot_debug.h"
#include "aliot_timer.h"


void aliot_time_start(aliot_time_t *timer)
{
    timer->time = aliot_platform_time_get_ms();
}

uint32_t aliot_time_spend(aliot_time_t *start)
{
    uint32_t now, res;

    now = aliot_platform_time_get_ms();
    res = now - start->time;
    return res;
}

uint32_t aliot_time_left(aliot_time_t *end)
{
    uint32_t now, res;

    if (aliot_time_is_expired(end)) {
        return 0;
    }

    now = aliot_platform_time_get_ms();
    res = end->time - now;
    return res;
}

uint32_t aliot_time_is_expired(aliot_time_t *timer)
{
    uint32_t cur_time = aliot_platform_time_get_ms();

    /*
     *  WARNING: Do NOT change the following code until you know exactly what it do!
     *
     *  check whether it reach destination time or not.
     */
    if ((cur_time - timer->time) < (UINT32_MAX / 2)) {
        return 1;
    } else {
        return 0;
    }
}

void aliot_time_init(aliot_time_t *timer)
{
    timer->time = 0;
}

void aliot_time_cutdown(aliot_time_t *timer, uint32_t millisecond)
{
    ALIOT_ASSERT(millisecond < (UINT32_MAX / 2), "time should NOT exceed UINT32_MAX/2!");
    timer->time = aliot_platform_time_get_ms() + millisecond;
}

uint32_t aliot_time_get_ms(void)
{
    return aliot_platform_time_get_ms();
}


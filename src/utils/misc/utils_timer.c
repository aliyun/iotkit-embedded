
#include "iot_import.h"
#include "utils_debug.h"
#include "utils_timer.h"


void iotx_time_start(iotx_time_t *timer)
{
    timer->time = iotx_platform_time_get_ms();
}

uint32_t utils_time_spend(iotx_time_t *start)
{
    uint32_t now, res;

    now = iotx_platform_time_get_ms();
    res = now - start->time;
    return res;
}

uint32_t iotx_time_left(iotx_time_t *end)
{
    uint32_t now, res;

    if (utils_time_is_expired(end)) {
        return 0;
    }

    now = iotx_platform_time_get_ms();
    res = end->time - now;
    return res;
}

uint32_t utils_time_is_expired(iotx_time_t *timer)
{
    uint32_t cur_time = iotx_platform_time_get_ms();

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

void iotx_time_init(iotx_time_t *timer)
{
    timer->time = 0;
}

void utils_time_cutdown(iotx_time_t *timer, uint32_t millisecond)
{
    IOTX_ASSERT(millisecond < (UINT32_MAX / 2), "time should NOT exceed UINT32_MAX/2!");
    timer->time = iotx_platform_time_get_ms() + millisecond;
}

uint32_t utils_time_get_ms(void)
{
    return iotx_platform_time_get_ms();
}


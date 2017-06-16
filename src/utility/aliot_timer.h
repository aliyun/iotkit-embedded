

#ifndef _ALIYUN_IOT_COMMON_TIMER_H_
#define _ALIYUN_IOT_COMMON_TIMER_H_

#include "aliot_platform.h"

typedef struct {
    long time;
} aliot_time_t;


void aliot_time_assign(int32_t millisecond, aliot_time_t *timer);

void aliot_time_start(aliot_time_t *timer);

uint32_t aliot_time_spend(aliot_time_t *start);

uint32_t aliot_time_left(aliot_time_t *end);

uint32_t aliot_time_is_expired(aliot_time_t *timer);

void aliot_time_init(aliot_time_t *timer);

void aliot_time_cutdown(aliot_time_t *timer, uint32_t millisecond);

uint32_t aliot_time_get_now(void);

//Get the difference value between @start and @end
uint32_t aliot_time_get_dvalue(aliot_time_t *start, aliot_time_t *end);

uint32_t aliot_time_get_ms(void);

uint64_t aliot_time_get_ms_64b(void);

uint32_t aliot_time_get_s(void);

#endif /* _ALIYUN_IOT_COMMON_TIMER_H_ */

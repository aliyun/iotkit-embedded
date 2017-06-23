

#ifndef _ALIOT_COMMON_TIMER_H_
#define _ALIOT_COMMON_TIMER_H_

#include "aliot_platform.h"

typedef struct {
    uint32_t time;
} aliot_time_t;


void aliot_time_start(aliot_time_t *timer);

uint32_t aliot_time_spend(aliot_time_t *start);

uint32_t aliot_time_left(aliot_time_t *end);

uint32_t aliot_time_is_expired(aliot_time_t *timer);

void aliot_time_init(aliot_time_t *timer);

void aliot_time_cutdown(aliot_time_t *timer, uint32_t millisecond);

uint32_t aliot_time_get_ms(void);

#endif /* _ALIOT_COMMON_TIMER_H_ */

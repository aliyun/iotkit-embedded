

#ifndef _ALIYUN_IOT_COMMON_TIMER_H_
#define _ALIYUN_IOT_COMMON_TIMER_H_


typedef struct {
    long time;
}aliot_time_t;


void aliyun_iot_timer_assignment(int32_t millisecond, aliot_time_t *timer);

void aliyun_iot_timer_start_clock(aliot_time_t *timer);

uint32_t aliyun_iot_timer_spend(aliot_time_t *start);

uint32_t aliyun_iot_timer_remain(aliot_time_t *end);

uint32_t aliyun_iot_timer_expired(aliot_time_t *timer);

void aliyun_iot_timer_init(aliot_time_t* timer);

void aliyun_iot_timer_cutdown(aliot_time_t* timer, uint32_t millisecond);

uint32_t aliyun_iot_timer_now();

uint32_t aliyun_iot_timer_interval(aliot_time_t *start,aliot_time_t *end);


#endif /* _ALIYUN_IOT_COMMON_TIMER_H_ */

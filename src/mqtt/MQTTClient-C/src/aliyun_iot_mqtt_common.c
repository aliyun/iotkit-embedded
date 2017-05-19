/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/
#include "aliyun_iot_mqtt_common.h"

char expired(aliot_timer_t* timer)
{
	return (char)aliyun_iot_timer_expired(&timer->end_time);
}

void countdown_ms(aliot_timer_t* timer, unsigned int timeout)
{
	aliyun_iot_timer_cutdown(&timer->end_time,timeout);
}

void countdown(aliot_timer_t* timer, unsigned int timeout)
{
	aliyun_iot_timer_cutdown(&timer->end_time,(timeout*1000));
}

int left_ms(aliot_timer_t* timer)
{
	return aliyun_iot_timer_remain(&timer->end_time);
}

int spend_ms(aliot_timer_t* timer)
{
    return aliyun_iot_timer_spend(&timer->end_time);
}

void InitTimer(aliot_timer_t* timer)
{
    aliyun_iot_timer_init(&timer->end_time);
}

void StartTimer(aliot_timer_t* timer)
{
    aliyun_iot_timer_start_clock(&timer->end_time);
}

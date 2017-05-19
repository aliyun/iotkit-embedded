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

#ifndef ALIYUN_IOT_MQTT_NET_H
#define ALIYUN_IOT_MQTT_NET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aliyun_iot_platform_memory.h"
#include "aliyun_iot_platform_network.h"
#include "aliyun_iot_platform_timer.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_net.h"


typedef struct Timer aliot_timer_t, *aliot_timer_pt;

struct Timer {
    ALIYUN_IOT_TIME_TYPE_S end_time;
};

char expired(aliot_timer_t*);
void countdown_ms(aliot_timer_t*, unsigned int);
void countdown(aliot_timer_t*, unsigned int);
int left_ms(aliot_timer_t*);
int spend_ms(aliot_timer_t*);
void InitTimer(aliot_timer_t*);
void StartTimer(aliot_timer_t* timer);

/** Define 0 to disable logging, define 1 to enable logging. */
#define ALI_IOT_MQTT_DEBUG 0

#endif

/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef _IOTX_MQTT_CLIENT_H_
#define _IOTX_MQTT_CLIENT_H_
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iot_import.h"

/* maximum number of successful subscribe */
#define IOTX_MC_SUB_NUM_MAX                     (10)

/* maximum republish elements in list */
#define IOTX_MC_REPUB_NUM_MAX                   (20)

#if defined(__cplusplus)
}
#endif
#endif  /* #ifndef _IOTX_MQTT_CLIENT_H_ */
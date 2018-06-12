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


#ifndef __MQTT_DEBUG_H__
#define __MQTT_DEBUG_H__

#include "lite-log.h"

#ifdef SDK_DEBUG_LEVEL_NONE
#define mqtt_emerg
#define mqtt_crit
#define mqtt_err
#define mqtt_warning
#define mqtt_info
#define mqtt_debug
#else
#define mqtt_emerg(...)    log_emerg("mqtt", __VA_ARGS__)
#define mqtt_crit(...)     log_crit("mqtt", __VA_ARGS__)
#define mqtt_err(...)      log_err("mqtt", __VA_ARGS__)
#define mqtt_warning(...)  log_warning("mqtt", __VA_ARGS__)
#define mqtt_info(...)     log_info("mqtt", __VA_ARGS__)
#define mqtt_debug(...)    log_debug("mqtt", __VA_ARGS__)
#endif

#endif  /* __MQTT_DEBUG_H__ */

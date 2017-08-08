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


#ifndef _IOTX_DEVICE_H_
#define _IOTX_DEVICE_H_
#if defined(__cplusplus)
extern "C" {
#endif

#include "iot_import.h"
#include "iot_export.h"

#define MQTT_SDK_VERSION    "2.0"

int     iotx_device_info_init(void);

int     iotx_device_info_set(
            const char *product_key,
            const char *device_name,
            const char *device_secret);

iotx_device_info_pt iotx_device_info_get(void);
iotx_conn_info_pt iotx_conn_info_get(void);

#if defined(__cplusplus)
}
#endif
#endif  /* #ifndef _IOTX_DEVICE_H_ */

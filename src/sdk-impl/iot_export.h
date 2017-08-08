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



#ifndef __IOT_EXPORT_H__
#define __IOT_EXPORT_H__
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>

typedef enum _IOT_LogLevel {
    IOT_LOG_EMERG = 0,
    IOT_LOG_CRIT,
    IOT_LOG_ERROR,
    IOT_LOG_WARNING,
    IOT_LOG_INFO,
    IOT_LOG_DEBUG,
} IOT_LogLevel;

void    IOT_OpenLog(const char *ident);
void    IOT_CloseLog(void);
void    IOT_SetLogLevel(IOT_LogLevel level);
void    IOT_DumpMemoryStats(IOT_LogLevel level);
int     IOT_SetupConnInfo(const char *product_key,
                          const char *device_name,
                          const char *device_secret,
                          void **info_ptr);

#include "exports/iot_export_errno.h"
#include "exports/iot_export_mqtt.h"
#include "exports/iot_export_device.h"
#include "exports/iot_export_shadow.h"
#include "exports/iot_export_coap.h"
#include "exports/iot_export_ota.h"

#if defined(__cplusplus)
}
#endif
#endif  /* __IOT_EXPORT_H__ */

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


#ifndef __CMP_OTA_H__
#define __CMP_OTA_H__


#ifdef SERVICE_OTA_ENABLED
#include "iot_import.h"

#include "utils_list.h"
#include "lite-utils.h"
#include "lite-system.h"
#include "iot_export.h"
#include "iot_export_ota.h"
#include "iotx_cmp_common.h"

void* iotx_cmp_ota_init(iotx_cmp_conntext_pt cmp_pt, const char* version);

int iotx_cmp_ota_yield(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_ota_pt ota_pt);

int iotx_cmp_ota_deinit(iotx_cmp_conntext_pt cmp_pt);

int iotx_cmp_ota_request_image(iotx_cmp_conntext_pt cmp_pt, const char* version);

int iotx_cmp_ota_get_config(iotx_cmp_conntext_pt cmp_pt, const char* configScope, const char* getType, const char* attributeKeys);

#endif /* SERVICE_OTA_ENABLED */


#endif /* __CMP_OTA_H__ */



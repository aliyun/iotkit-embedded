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


#ifdef CMP_VIA_CLOUN_CONN

#ifndef __CMP_VIA_CLOUD_CONN_H__
#define __CMP_VIA_CLOUD_CONN_H__

#include "iot_import.h"

#include "utils_list.h"
#include "lite-utils.h"
#include "lite-system.h"
#include "iot_export.h"
#include "iotx_cmp_common.h"


int iotx_cmp_cloud_conn_disconnect_handler(iotx_cmp_conntext_pt cmp_pt);
int iotx_cmp_cloud_conn_reconnect_handler(iotx_cmp_conntext_pt cmp_pt);
int iotx_cmp_cloud_conn_register_handler(iotx_cmp_conntext_pt cmp_pt,
                    char* URI, int result, int is_register);
int iotx_cmp_cloud_conn_response_handler(iotx_cmp_conntext_pt cmp_pt,
                    iotx_cmp_message_info_pt message_info);

void* iotx_cmp_cloud_conn_init(void* handler, iotx_cmp_init_param_pt pparam);
int iotx_cmp_cloud_conn_register(void* handler, void* connectivity_pt, const char* topic_filter);
int iotx_cmp_cloud_conn_unregister(void* handler, void* connectivity_pt, const char* topic_filter);
int iotx_cmp_cloud_conn_send(void* handler,
                    void* connectivity_pt,
                    const char* topic_filter,
                    iotx_cmp_message_ack_types_t ack_type,
                    const void* payload,
                    int payload_length);
int iotx_cmp_cloud_conn_send_sync(void* handler,
                    void* connectivity_pt,
                    const char* topic_filter,
                    iotx_cmp_message_ack_types_t ack_type,
                    const void* payload,
                    int payload_length);
int iotx_cmp_cloud_conn_yield(void* connectivity_pt, int timeout_ms);
int iotx_cmp_cloud_conn_deinit(void* connectivity_pt);

#endif /* __CMP_VIA_CLOUD_CONN_H__ */
#endif


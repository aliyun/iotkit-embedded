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

#include "iotx_cmp_common.h"


/* text for log */
/* error */
const char cmp_log_error_parameter[] CMP_READ_ONLY = "parameters error!";
const char cmp_log_error_param_1[] CMP_READ_ONLY = "param error: lack of product_key or device_name or device_secret";
const char cmp_log_error_auth[] CMP_READ_ONLY = "auth error";
const char cmp_log_error_memory[] CMP_READ_ONLY = "memory error";
const char cmp_log_error_fail[] CMP_READ_ONLY = "failed";
const char cmp_log_error_fail_ota[] CMP_READ_ONLY = "ota failed";
const char cmp_log_error_fail_1[] CMP_READ_ONLY = "register fail: add mapping fail.";
const char cmp_log_error_fail_rc[] CMP_READ_ONLY = "fail: rc = %d.";
const char cmp_log_error_status[] CMP_READ_ONLY = "status error!";
const char cmp_log_error_splice_URI[] CMP_READ_ONLY = "splice URI fail!";
const char cmp_log_error_parse_URI[] CMP_READ_ONLY = "parse URI fail!";
const char cmp_log_error_splice_payload[] CMP_READ_ONLY = "splice payload fail!";
const char cmp_log_error_parse_payload[] CMP_READ_ONLY = "parse payload fail!";
const char cmp_log_error_get_node[] CMP_READ_ONLY = "get list node fail!";
const char cmp_log_error_push_node[] CMP_READ_ONLY = "push list node fail!";
const char cmp_log_error_process[] CMP_READ_ONLY = "too many process";
const char cmp_log_error_parse_id[] CMP_READ_ONLY = "parse id error";
const char cmp_log_error_parse_code[] CMP_READ_ONLY = "parse code error";
const char cmp_log_error_parse_data[] CMP_READ_ONLY = "parse data error";
const char cmp_log_error_parse_params[] CMP_READ_ONLY = "parse params error";
const char cmp_log_error_parse_method[] CMP_READ_ONLY = "parse method error";
const char cmp_log_error_type[] CMP_READ_ONLY = "type error";
const char cmp_log_error_pk[] CMP_READ_ONLY = "product_key is empty";
const char cmp_log_error_dn[] CMP_READ_ONLY = "device_name is empty";
const char cmp_log_error_di[] CMP_READ_ONLY = "device_id is empty";
const char cmp_log_error_ret_code[] CMP_READ_ONLY = "\r\n ret_code = %d (!= 200), abort!\r\n ";
const char cmp_log_error_secret_1[] CMP_READ_ONLY = "secret type is product_secret, but the product_secret is null";
const char cmp_log_error_secret_2[] CMP_READ_ONLY = "device_secret is null";

/* warning */
const char cmp_log_warning_ota_started[] CMP_READ_ONLY = "ota is started";
const char cmp_log_warning_cloud_disconnected[] CMP_READ_ONLY = "cloud is not connect";
const char cmp_log_warning_not_support[] CMP_READ_ONLY = "NOT_SUPPORT";
const char cmp_log_warning_not_arrived[] CMP_READ_ONLY = "Should NOT arrived here";
const char cmp_log_warning_not_mapping[] CMP_READ_ONLY = "can not find mapping";
const char cmp_log_warning_not_func[] CMP_READ_ONLY = "can not find func";
const char cmp_log_warning_no_list[] CMP_READ_ONLY = "no list";
const char cmp_log_warning_buffer_overflow[] CMP_READ_ONLY = "buffer overflow, %s";

/* info */
const char cmp_log_info_init[] CMP_READ_ONLY = "cmp context initialize";
const char cmp_log_info_registered[] CMP_READ_ONLY = "URI have been registered, can not register again";
const char cmp_log_info_event_id[] CMP_READ_ONLY = "event_id %d";
const char cmp_log_info_event_type[] CMP_READ_ONLY = "event_type %d";
const char cmp_log_info_rsp_type[] CMP_READ_ONLY = "rsp_type %d";
const char cmp_log_info_cloud_disconnect[] CMP_READ_ONLY = "cloud disconnect";
const char cmp_log_info_cloud_reconnect[] CMP_READ_ONLY = "cloud reconnect";
const char cmp_log_info_result[] CMP_READ_ONLY = "rc = %d";
const char cmp_log_info_URI[] CMP_READ_ONLY = "URI = %s";
const char cmp_log_info_URI_1[] CMP_READ_ONLY = "URI: %.*s";
const char cmp_log_info_URI_sys[] CMP_READ_ONLY = "URI type: /SYS/ ";
const char cmp_log_info_URI_ext[] CMP_READ_ONLY = "URI type: /EXT/ ";
const char cmp_log_info_URI_undefined[] CMP_READ_ONLY = "URI type: /UNDEFINE/ ";
const char cmp_log_info_URI_length[] CMP_READ_ONLY = "URI length = %d";
const char cmp_log_info_payload_length[] CMP_READ_ONLY = "payload length = %d";
const char cmp_log_info_firmware[] CMP_READ_ONLY = "The firmware is valid";
const char cmp_log_info_MQTT_disconnect[] CMP_READ_ONLY = "MQTT disconnect.";
const char cmp_log_info_MQTT_reconnect[] CMP_READ_ONLY = "MQTT reconnect";
const char cmp_log_info_remove_mapping[] CMP_READ_ONLY = "find mapping and remove it";
const char cmp_log_info_enter_process_1[] CMP_READ_ONLY = "enter cloud process";
const char cmp_log_info_enter_process_2[] CMP_READ_ONLY = "enter local process";
const char cmp_log_info_process_type[] CMP_READ_ONLY = "process %d";
const char cmp_log_info_raw_data[] CMP_READ_ONLY = "raw data";
const char cmp_log_info_auth_req[] CMP_READ_ONLY = "req_str = '%s'";
const char cmp_log_info_auth_payload_req[] CMP_READ_ONLY = "req_payload: \r\n\r\n%s\r\n";
const char cmp_log_info_auth_payload_rsp[] CMP_READ_ONLY = "response payload: \r\n\r\n%s\r\n";
const char cmp_log_info_auth_rsp[] CMP_READ_ONLY = "http response: \r\n\r\n%s\r\n";


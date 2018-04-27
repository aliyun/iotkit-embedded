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


#ifndef __SRC_CMP_COMMON_H__
#define __SRC_CMP_COMMON_H__

#include "iot_export_cmp.h"

#ifdef CMP_SUPPORT_MULTI_THREAD
#ifdef CMP_SUPPORT_MULTI_THREAD_VIA_HAL
#include "iot_import.h"
#else
/* multi-thread */
#include <pthread.h>
#endif
#endif


#ifdef CMP_SUPPORT_MEMORY_MAGIC
#define CMP_malloc(size) LITE_malloc(size, MEM_MAGIC, "CMP")
#else
#define CMP_malloc(size) LITE_malloc(size)
#endif


#ifdef ESP8266
#include "esp_common.h"
#define CMP_READ_ONLY ICACHE_RODATA_ATTR STORE_ATTR
#else
#define CMP_READ_ONLY
#endif

#define CMP_INFO(...)         LITE_syslog(__FUNCTION__, __LINE__, LOG_INFO_LEVEL, __VA_ARGS__)
#define CMP_WARNING(...)      LITE_syslog(__FUNCTION__, __LINE__, LOG_WARNING_LEVEL, __VA_ARGS__)
#define CMP_ERR(...)          LITE_syslog(__FUNCTION__, __LINE__, LOG_ERR_LEVEL, __VA_ARGS__)


#ifdef CMP_SUPPORT_TOPIC_DISPATCH
#define CMP_MAPPING_USE_POOL
#define CMP_SUPPORT_MAX_MAPPING_SIZE            (20)
#endif

#ifdef CMP_SUPPORT_MULTI_THREAD

#define CMP_PROCESS_NODE_USE_POOL
#define CMP_SUPPORT_MAX_PROCESS_NODE_SIZE       (30)
#endif


/* text for log */
/* error */
extern const char cmp_log_error_parameter[] CMP_READ_ONLY;
extern const char cmp_log_error_param_1[] CMP_READ_ONLY;
extern const char cmp_log_error_auth[] CMP_READ_ONLY;
extern const char cmp_log_error_memory[] CMP_READ_ONLY;
extern const char cmp_log_error_fail[] CMP_READ_ONLY;
extern const char cmp_log_error_fail_ota[] CMP_READ_ONLY;
extern const char cmp_log_error_fail_1[] CMP_READ_ONLY;
extern const char cmp_log_error_fail_rc[] CMP_READ_ONLY;
extern const char cmp_log_error_status[] CMP_READ_ONLY;
extern const char cmp_log_error_splice_URI[] CMP_READ_ONLY;
extern const char cmp_log_error_parse_URI[] CMP_READ_ONLY;
extern const char cmp_log_error_splice_payload[] CMP_READ_ONLY;
extern const char cmp_log_error_parse_payload[] CMP_READ_ONLY;
extern const char cmp_log_error_get_node[] CMP_READ_ONLY;
extern const char cmp_log_error_push_node[] CMP_READ_ONLY;
extern const char cmp_log_error_process[] CMP_READ_ONLY;
extern const char cmp_log_error_parse_id[] CMP_READ_ONLY;
extern const char cmp_log_error_parse_code[] CMP_READ_ONLY;
extern const char cmp_log_error_parse_data[] CMP_READ_ONLY;
extern const char cmp_log_error_parse_params[] CMP_READ_ONLY;
extern const char cmp_log_error_parse_method[] CMP_READ_ONLY;
extern const char cmp_log_error_type[] CMP_READ_ONLY;
extern const char cmp_log_error_pk[] CMP_READ_ONLY;
extern const char cmp_log_error_dn[] CMP_READ_ONLY;
extern const char cmp_log_error_di[] CMP_READ_ONLY;
extern const char cmp_log_error_secret_1[] CMP_READ_ONLY;
extern const char cmp_log_error_secret_2[] CMP_READ_ONLY;
extern const char cmp_log_error_ret_code[] CMP_READ_ONLY;

/* warning */
extern const char cmp_log_warning_ota_started[] CMP_READ_ONLY;
extern const char cmp_log_warning_cloud_disconnected[] CMP_READ_ONLY;
extern const char cmp_log_warning_not_support[] CMP_READ_ONLY;
extern const char cmp_log_warning_not_arrived[] CMP_READ_ONLY;
extern const char cmp_log_warning_not_mapping[] CMP_READ_ONLY;
extern const char cmp_log_warning_not_func[] CMP_READ_ONLY;
extern const char cmp_log_warning_no_list[] CMP_READ_ONLY;
extern const char cmp_log_warning_buffer_overflow[] CMP_READ_ONLY;

/* info */
extern const char cmp_log_info_init[] CMP_READ_ONLY;
extern const char cmp_log_info_registered[] CMP_READ_ONLY;
extern const char cmp_log_info_event_id[] CMP_READ_ONLY;
extern const char cmp_log_info_event_type[] CMP_READ_ONLY;
extern const char cmp_log_info_rsp_type[] CMP_READ_ONLY;
extern const char cmp_log_info_cloud_disconnect[] CMP_READ_ONLY;
extern const char cmp_log_info_cloud_reconnect[] CMP_READ_ONLY;
extern const char cmp_log_info_result[] CMP_READ_ONLY;
extern const char cmp_log_info_URI[] CMP_READ_ONLY;
extern const char cmp_log_info_URI_1[] CMP_READ_ONLY;
extern const char cmp_log_info_URI_sys[] CMP_READ_ONLY;
extern const char cmp_log_info_URI_ext[] CMP_READ_ONLY;
extern const char cmp_log_info_URI_undefined[] CMP_READ_ONLY;
extern const char cmp_log_info_URI_length[] CMP_READ_ONLY;
extern const char cmp_log_info_payload_length[] CMP_READ_ONLY;
extern const char cmp_log_info_firmware[] CMP_READ_ONLY;
extern const char cmp_log_info_MQTT_disconnect[] CMP_READ_ONLY;
extern const char cmp_log_info_MQTT_reconnect[] CMP_READ_ONLY;
extern const char cmp_log_info_remove_mapping[] CMP_READ_ONLY;
extern const char cmp_log_info_enter_process_1[] CMP_READ_ONLY;
extern const char cmp_log_info_enter_process_2[] CMP_READ_ONLY;
extern const char cmp_log_info_process_type[] CMP_READ_ONLY;
extern const char cmp_log_info_raw_data[] CMP_READ_ONLY;
extern const char cmp_log_info_auth_req[] CMP_READ_ONLY;
extern const char cmp_log_info_auth_rsp[] CMP_READ_ONLY;
extern const char cmp_log_info_auth_payload_req[] CMP_READ_ONLY;
extern const char cmp_log_info_auth_payload_rsp[] CMP_READ_ONLY;


/* The structure of cmp event msg */
typedef struct {
    char                                       *URI;
    void                                       *payload;
    unsigned int                                payload_length;
} iotx_cmp_response_info_t, *iotx_cmp_response_info_pt;

typedef void (*iotx_cmp_response_func_fpt)(void* pcontext, iotx_cmp_message_info_pt msg_info);


/* The structure of cloud connection context */
typedef struct iotx_cmp_mapping_st {
    void                                       *next;
    char                                       *URI;
#ifdef CMP_MAPPING_USE_POOL
    int                                         is_used;
#endif
    iotx_cmp_message_types_t                    type;
    iotx_cmp_register_func_fpt                  func;
    void                                       *user_data;
    void                                       *mail_box;
} iotx_cmp_mapping_t, *iotx_cmp_mapping_pt;

#ifdef CMP_SUPPORT_MULTI_THREAD

typedef enum {
    /* cloud process */
    IOTX_CMP_PROCESS_TYPE_CLOUD = 1,

#ifdef CMP_SUPPORT_LOCAL_CONN_CONN
    /* cloud process */
    IOTX_CMP_PROCESS_TYPE_LOCAL  = 2,
#endif

    IOTX_CMP_PROCESS_TYPE_MAX
}iotx_cmp_process_types_t;


typedef enum {
    /* register */
    /* msg is iotx_cmp_process_register_pt */
    IOTX_CMP_PROCESS_CLOUD_REGISTER = 0,

    /* register result */
    /* msg is iotx_cmp_process_register_result_pt */
    IOTX_CMP_PROCESS_CLOUD_REGISTER_RESULT,

    /* unregister */
    /* msg is URI */
    IOTX_CMP_PROCESS_CLOUD_UNREGISTER,

    /* unregister result */
    /* msg is iotx_cmp_process_register_result_pt */
    IOTX_CMP_PROCESS_CLOUD_UNREGISTER_RESULT,

    /* send */
    /* msg is iotx_cmp_process_send_pt */
    IOTX_CMP_PROCESS_CLOUD_SEND,

    /* new data */
    /* msg is iotx_mqtt_topic_info_pt or iotx_cloud_connection_msg_rsp_pt */
    IOTX_CMP_PROCESS_CLOUD_NEW_DATA,

    /* cloud disconnect */
    /* msg is null*/
    IOTX_CMP_PROCESS_CLOUD_DISCONNECT,

    /* cloud reconnect */
    /* msg is null*/
    IOTX_CMP_PROCESS_CLOUD_RECONNECT,

    IOTX_CMP_PROCESS_CLOUD_MAX,


#ifdef CMP_SUPPORT_LOCAL_CONN_CONN
    /* local add device */
    /* todo */
    IOTX_CMP_PROCESS_LOCAL_ADD_DEVICE = IOTX_CMP_PROCESS_CLOUD_MAX + 1,

    /* local remoce device */
    /* todo */
    IOTX_CMP_PROCESS_LOCAL_REMOVE_DEVICE,

    /* register */
    /* msg is iotx_cmp_process_register_pt */
    IOTX_CMP_PROCESS_LOCAL_ADD_SERVICE,

    /* register result */
    /* msg is iotx_cmp_process_register_result_pt */
    IOTX_CMP_PROCESS_LOCAL_ADD_SERVICE_RESULT,

    /* unregister */
    /* msg is URI */
    IOTX_CMP_PROCESS_LOCAL_REMOVE_SERVICE,

    /* unregister result */
    /* msg is iotx_cmp_process_register_result_pt */
    IOTX_CMP_PROCESS_LOCAL_REMOVE_SERVICE_RESULT,

    /* send */
    /* msg is iotx_cmp_process_send_pt */
    IOTX_CMP_PROCESS_LOCAL_SEND,

    /* new data */
    /* msg is iotx_mqtt_topic_info_pt or iotx_cloud_connection_msg_rsp_pt */
    IOTX_CMP_PROCESS_LOCAL_NEW_DATA,

    /* local disconnect */
    /* msg is null*/
    IOTX_CMP_PROCESS_LOCAL_DISCONNECT,

    /* local reconnect */
    /* msg is null*/
    IOTX_CMP_PROCESS_LOCAL_RECONNECT,

#endif
    IOTX_CMP_PROCESS_LOCAL_MAX,

    IOTX_CMP_PROCESS_MAX = 100
}iotx_cmp_process_node_types_t;


typedef struct iotx_cmp_process_register_st {
    char*                                       URI;
    iotx_cmp_message_types_t                    type;
    iotx_cmp_register_func_fpt                  register_func;
    void                                       *user_data;
    void                                       *mail_box;
} iotx_cmp_process_register_t, *iotx_cmp_process_register_pt;

typedef struct iotx_cmp_process_send_st {
    iotx_cmp_send_peer_pt                       target;
    iotx_cmp_message_ack_types_t                ack_type;
    char*                                       URI;
    void*                                       payload;
    int                                         payload_length;
} iotx_cmp_process_send_t, *iotx_cmp_process_send_pt;

typedef struct iotx_cmp_process_register_result_st {
    char*                                       URI;
    /* 0: success, -1:fail */
    int                                         result;
    int                                         is_register;
} iotx_cmp_process_register_result_t, *iotx_cmp_process_register_result_pt;

typedef struct iotx_cmp_process_list_node_st {
    void                                       *next;
    void                                       *pre;
#ifdef CMP_PROCESS_NODE_USE_POOL
    int                                         is_used;
#endif
    iotx_cmp_process_node_types_t               type;
    void*                                       msg;
} iotx_cmp_process_list_node_t, *iotx_cmp_process_list_node_pt;

typedef struct iotx_cmp_process_list_st {
    iotx_cmp_process_list_node_pt header;
    iotx_cmp_process_list_node_pt tailer;
    int                           size;
} iotx_cmp_process_list_t, *iotx_cmp_process_list_pt;

#endif

typedef void* (*_init_function)(void* handler, iotx_cmp_init_param_pt pparam);
typedef int (*_connect_function)(void* handler, void* connectivity_pt);
typedef int (*_register_function)(void* handler, void* connectivity_pt, const char* topic_filter);
typedef int (*_unregister_function)(void* handler, void* connectivity_pt, const char* topic_filter);
typedef int (*_send_function)(void* handler, void* connectivity_pt, const char* topic_filter,
                iotx_cmp_message_ack_types_t ack_type, const void* payload, int payload_length);
typedef int (*_send_sync_function)(void* handler, void* connectivity_pt, const char* topic_filter,
                iotx_cmp_message_ack_types_t ack_type, const void* payload, int payload_length);
typedef int (*_yield_function)(void* connectivity_pt, int timeout_ms);
typedef int (*_deinit_function)(void* connectivity_pt);

typedef struct iotx_cmp_connectivity_st {
    void*                        context;
    uint8_t                      is_connected;
    _init_function               init_func;
    _connect_function            connect_func;
    _register_function           register_func;
    _unregister_function         unregister_func;
    _send_function               send_func;
    _send_sync_function          send_sync_func;
    _yield_function              yield_func;
    _deinit_function             deinit_func;
} iotx_cmp_connectivity_t, *iotx_cmp_connectivity_pt;

typedef struct iotx_cmp_connectivity_list_st {
    iotx_cmp_connectivity_pt      node;
    void*                         next;
} iotx_cmp_connectivity_list_t, *iotx_cmp_connectivity_list_pt;


typedef struct iotx_cmp_event_callback_node_st {
    iotx_cmp_event_handle_func_fpt              event_func;
    void                                       *user_data;
}iotx_cmp_event_callback_node_t, *iotx_cmp_event_callback_node_pt;


typedef struct iotx_cmp_event_callback_list_st {
    iotx_cmp_event_callback_node_pt node;
    void*                           next;
}iotx_cmp_event_callback_list_t, *iotx_cmp_event_callback_list_pt;

typedef struct iotx_cmp_conntext_st{
#ifdef CMP_SUPPORT_MULTI_THREAD
#ifdef CMP_SUPPORT_MULTI_THREAD_VIA_HAL
    void*                                       pthread_yield;
    void*                                       pthread_process;
#else
    pthread_t                                   pthread_yield;
    pthread_t                                   pthread_process;
#endif
    iotx_cmp_process_list_pt                    process_cloud_list;
    void*                                       process_cloud_lock;
#ifdef CMP_SUPPORT_LOCAL_CONN
    iotx_cmp_process_list_pt                    process_local_list;
    void*                                       process_local_lock;
#endif
    uint8_t                                     thread_stop;
    uint8_t                                     thread_is_stoped;
#endif
    uint8_t                                     is_connect;
    uint64_t                                    cmp_message_id;
    iotx_cmp_connectivity_list_pt               connectivity_list;
    iotx_cmp_mapping_pt                         mapping_list;
    iotx_cmp_response_func_fpt                  response_func;
    iotx_cmp_event_callback_list_pt             event_callback_list;
#ifdef SERVICE_OTA_ENABLED
    void*                                       ota_handler;
    iotx_cmp_fota_handle_func_fpt               fota_func;
    void                                       *fota_user_context;
    iotx_cmp_cota_handle_func_fpt               cota_func;
    void                                       *cota_user_context;
#endif /* SERVICE_OTA_ENABLED */
} iotx_cmp_conntext_t, *iotx_cmp_conntext_pt;

int iotx_cmp_auth(const char *product_key, const char *device_name, const char *client_id);

void iotx_cmp_free_message_info(iotx_cmp_message_info_pt message_info);

void iotx_cmp_response_func(void* pcontext, iotx_cmp_message_info_pt message_info);

int iotx_cmp_parse_payload(void* payload,
                    int payload_length,
                    iotx_cmp_message_info_pt msg);

int iotx_cmp_splice_payload(void* payload,
                    int* payload_length,
                    int id,
                    iotx_cmp_message_info_pt msg);

int iotx_cmp_parse_URI(char* URI,
                    int URI_length,
                    char* URI_param,
                    iotx_cmp_uri_types_t* uri_type);

int iotx_cmp_splice_URI(char* URI,
                    int* URI_length,
                    const char* URI_param,
                    iotx_cmp_uri_types_t uri_type);

int iotx_cmp_add_connectivity(iotx_cmp_conntext_pt cmp_pt,
                    iotx_cmp_connectivity_pt connectivity);

int iotx_cmp_add_connectivity_all(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_init_param_pt pparam);

int iotx_cmp_remove_connectivity_all(iotx_cmp_conntext_pt cmp_pt);

iotx_cmp_mapping_pt iotx_cmp_get_mapping_node();

int iotx_cmp_free_mapping_node(iotx_cmp_mapping_pt node);

int iotx_cmp_add_mapping(iotx_cmp_conntext_pt cmp_pt,
                    char* URI,
                    iotx_cmp_message_types_t type,
                    iotx_cmp_register_func_fpt func,
                    void* user_data,
                    void* mail_box);

int iotx_cmp_remove_mapping(iotx_cmp_conntext_pt cmp_pt, char* URI);

int iotx_cmp_remove_mapping_all(iotx_cmp_conntext_pt cmp_pt);

iotx_cmp_mapping_pt iotx_cmp_find_mapping(iotx_cmp_conntext_pt cmp_pt, char* URI, int URI_length);

int iotx_cmp_register_service(iotx_cmp_conntext_pt cmp_pt,
                char* URI,
                iotx_cmp_message_types_t type,
                iotx_cmp_register_func_fpt register_func,
                void* user_data,
                void* mail_box);

int iotx_cmp_unregister_service(iotx_cmp_conntext_pt cmp_pt, char* URI);

int iotx_cmp_parse_message(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_message_info_pt message_info,
                char** URI,
                void** payload,
                int* payload_length);

int iotx_cmp_send_data(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_send_peer_pt target,
                char* URI,
                iotx_cmp_message_ack_types_t ack_type,
                void* payload,
                int payload_length);

int iotx_cmp_trigger_event_callback(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_event_msg_pt msg);

#ifdef CMP_SUPPORT_MULTI_THREAD
iotx_cmp_process_list_node_pt iotx_cmp_get_list_node(iotx_cmp_process_types_t type);
int iotx_cmp_free_list_node(iotx_cmp_process_list_node_pt node);
int iotx_cmp_free_list_node_all(iotx_cmp_conntext_pt cmp_pt);

/* node is in */
int iotx_cmp_process_list_push(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_process_types_t type,
                iotx_cmp_process_list_node_pt node);

/* node is out */
iotx_cmp_process_list_node_pt iotx_cmp_process_list_pop(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_process_types_t type);

int iotx_cmp_process_list_get_size(iotx_cmp_conntext_pt cmp_pt,
                iotx_cmp_process_types_t type);

void* iotx_cmp_cloud_process(void *pclient);

#ifdef CMP_SUPPORT_LOCAL_CONN
void* iotx_cmp_local_process(void *pclient);
#endif

#endif

#endif /* __SRC_CMP_COMMON_H__ */



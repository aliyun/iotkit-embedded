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

/*#define CMP_SUPPORT_MULTI_THREAD*/

#ifdef CMP_SUPPORT_MULTI_THREAD
/* multi-thread */
#include <pthread.h>
#endif

/* The structure of cmp event msg */
typedef struct {
    char                                       *URI;
    void                                       *payload;
    unsigned int                                payload_length;
} iotx_cmp_response_info_t, *iotx_cmp_response_info_pt;

typedef void (*iotx_cmp_response_func_fpt)(void* pcontext, iotx_cmp_response_info_pt msg);


/* The structure of cloud connection context */
typedef struct iotx_cmp_mapping_st {
    void                                       *next;                                         
    char                                       *URI;
    iotx_cmp_register_func_fpt                  func;
    void                                       *user_data;
} iotx_cmp_mapping_t, *iotx_cmp_mapping_pt;


typedef struct iotx_cmp_process_response_st {
    char*                                       URI;
    void*                                       payload;
    int                                         payload_length;
} iotx_cmp_process_response_t, *iotx_cmp_process_response_pt;


#ifdef CMP_SUPPORT_MULTI_THREAD
 
typedef enum {    
    /* register */
    /* msg is iotx_cmp_process_register_pt */
    IOTX_CMP_PROCESS_REGISTER = 1,
        
    /* unregister */
    /* msg is URI */
    IOTX_CMP_PROCESS_UNREGISTER  = 2,
        
    /* send */
    /* msg is iotx_cmp_process_send_pt */
    IOTX_CMP_PROCESS_SEND  = 3,

    /* cloud disconnect */
    /* msg is null*/
    IOTX_CMP_PROCESS_CLOUD_DISCONNECT = 4,
        
    /* cloud reconnect */
    /* msg is null*/
    IOTX_CMP_PROCESS_CLOUD_RECONNECT = 5,
    
    /* local add device */
    /* todo */
    IOTX_CMP_PROCESS_LOCAL_ADD_DEVICE = 6,
    
    /* local remoce device */
    /* todo */
    IOTX_CMP_PROCESS_LOCAL_REMOVE_DEVICE = 7,

    /* register result */
    /* msg is iotx_cmp_process_register_result_pt */
    IOTX_CMP_PROCESS_REGISTER_RESULT = 8,
    
    /* unregister result */
    /* msg is iotx_cmp_process_register_result_pt */
    IOTX_CMP_PROCESS_UNREGISTER_RESULT = 9,

    /* new data */
    /* msg is iotx_mqtt_topic_info_pt or iotx_cloud_connection_msg_rsp_pt */
    IOTX_CMP_PROCESS_NEW_DATA = 10,
    
    IOTX_CMP_PROCESS_MAX
}iotx_cmp_process_types_t;

    
typedef struct iotx_cmp_process_register_st {
    char*                                       URI;
    iotx_cmp_register_func_fpt                  register_func;
    void                                       *user_data;      
} iotx_cmp_process_register_t, *iotx_cmp_process_register_pt;

typedef struct iotx_cmp_process_send_st {
    iotx_cmp_send_peer_pt                       target;
    iotx_cmp_message_info_pt                    message_info;
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
    iotx_cmp_process_types_t                    type;
    void*                                       msg;
} iotx_cmp_process_list_node_t, *iotx_cmp_process_list_node_pt;

typedef struct iotx_cmp_process_list_st {
    iotx_cmp_process_list_node_pt header;
    iotx_cmp_process_list_node_pt tailer;
    int                           size;
} iotx_cmp_process_list_t, *iotx_cmp_process_list_pt;

#endif


typedef struct iotx_cmp_conntext_st{
#ifdef CMP_SUPPORT_MULTI_THREAD
    pthread_t                                   pthread_yield;
    pthread_t                                   pthread_process;
    iotx_cmp_process_list_pt                    process_list;
    void*                                       process_lock;     
#endif
    /* cloud connection */
    void*                                       cloud_connection;
    /* local connection */
    void*                                       local_connection;    
#ifdef CMP_SUPPORT_OTA
    void*                                       ota_handler;
#endif
    iotx_cmp_mapping_pt                         mapping_list;
    iotx_cmp_event_handle_func_fpt              event_func;
    void                                       *user_data; 
    iotx_cmp_response_func_fpt                  response_func;
} iotx_cmp_conntext_t, *iotx_cmp_conntext_pt;

void iotx_cmp_response_func(void* pcontext, iotx_cmp_response_info_pt msg);

int iotx_cmp_parse_payload(void* payload,
                    int payload_length, 
                    iotx_cmp_message_info_pt msg);

int iotx_cmp_splice_payload(void* payload, 
                    int* payload_length, 
                    int id,
                    iotx_cmp_message_info_pt msg);

int iotx_cmp_parse_URI(char* topic, char* URI, iotx_cmp_uri_types_t* uri_type);

int iotx_cmp_splice_URI(char* topic, int* topic_length, const char* URI, iotx_cmp_uri_types_t uri_type);


int iotx_cmp_add_mapping(iotx_cmp_conntext_pt cmp_pt, char* URI, iotx_cmp_register_func_fpt func, void* user_data);

int iotx_cmp_remove_mapping(iotx_cmp_conntext_pt cmp_pt, char* URI);

int iotx_cmp_remove_mapping_all(iotx_cmp_conntext_pt cmp_pt); 

iotx_cmp_mapping_pt iotx_cmp_find_mapping(iotx_cmp_conntext_pt cmp_pt, char* URI);

int iotx_cmp_register_service(iotx_cmp_conntext_pt cmp_pt, 
                char* URI, 
                iotx_cmp_register_func_fpt register_func, 
                void* user_data);

int iotx_cmp_unregister_service(iotx_cmp_conntext_pt cmp_pt, char* URI); 

int iotx_cmp_send_data(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_send_peer_pt target, iotx_cmp_message_info_pt message_info);

#ifdef CMP_SUPPORT_MULTI_THREAD
/* node is in */
int iotx_cmp_process_list_push(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_process_list_node_pt node);

/* node is out */
iotx_cmp_process_list_node_pt iotx_cmp_process_list_pop(iotx_cmp_conntext_pt cmp_pt);

int iotx_cmp_process_list_get_size(iotx_cmp_conntext_pt cmp_pt);

void* iotx_cmp_process(void *pclient);

void* iotx_cmp_yield_process(void *pclient);
#endif


#endif /* __SRC_CMP_COMMON_H__ */



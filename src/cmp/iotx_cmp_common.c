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
#include "iotx_cmp_mqtt_direct.h"
#include "iotx_cmp_cloud_conn.h"

#define IOTX_CMP_MESSAGE_ID_MAX     (65535)
static int g_cmp_message_id = 0;

static int iotx_cmp_get_next_message_id()
{
    if (g_cmp_message_id == IOTX_CMP_MESSAGE_ID_MAX)
        g_cmp_message_id = 1;

    g_cmp_message_id++;

    return g_cmp_message_id;
}


int iotx_cmp_parse_payload(void* payload,
                    int payload_length, 
                    iotx_cmp_message_info_pt msg)
{
    char* payload_pt = (char*)payload;
    char* node = NULL; 
    
    if (NULL == payload || NULL == msg || 0 == payload_length) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    log_info("payload_length: %d", payload_length);

    log_info("payload: %s", payload_pt);

    /* parse id */
    node = LITE_json_value_of("id", payload_pt);
    if (node == NULL) {
        log_info("raw data");   
        msg->message_type = IOTX_CMP_MESSAGE_RAW;        
        msg->code = 0;
        msg->parameter = LITE_malloc(payload_length + 1);
        if (NULL == msg->parameter) {
            log_info("memory error");
            LITE_free(node);
            return FAIL_RETURN;
        }
        memset(msg->parameter, 0x0, payload_length);
        memcpy(msg->parameter, payload, payload_length);
        msg->parameter_length = payload_length;
        msg->method = NULL;
        msg->id = 0;
        return SUCCESS_RETURN;
    }
    
    msg->id = atoi(node);
    LITE_free(node);
    node = NULL;   
            
    /* parse code */
    node = LITE_json_value_of("code", payload_pt);
    if (node != NULL) { 
        log_info("response"); 
        msg->message_type = IOTX_CMP_MESSAGE_RESPONSE;
        msg->code = atoi(node);
        LITE_free(node);
        node = NULL;            
            
        /* parse data */
        node = LITE_json_value_of("data", payload_pt);
        if (node == NULL) { 
            log_err("get params of json error!");
            return FAIL_RETURN;
        }
        msg->parameter = LITE_malloc(strlen(node) + 1);
        if (NULL == msg->parameter) {
            log_info("memory error");
            LITE_free(node);
            return FAIL_RETURN;
        }
        memset(msg->parameter, 0x0, strlen(node));
        strncpy(msg->parameter, node, strlen(node));
        msg->parameter_length = strlen(msg->parameter);
        LITE_free(node);
        node = NULL; 
        
        msg->method = NULL;
    } else {
        log_info("request");    
            
        /* parse parameter */
        node = LITE_json_value_of("params", payload_pt);
        if (node == NULL) { 
            log_err("get params of json error!");
            return FAIL_RETURN;
        }
        msg->parameter = LITE_malloc(strlen(node) + 1);
        if (NULL == msg->parameter) {
            log_info("memory error");
            LITE_free(node);
            return FAIL_RETURN;
        }
        memset(msg->parameter, 0x0, strlen(node));
        strncpy(msg->parameter, node, strlen(node));
        msg->parameter_length = strlen(msg->parameter);
        LITE_free(node);
        node = NULL;  
        
        /* parse method */
        node = LITE_json_value_of("method", payload_pt);
        if (node == NULL) { 
            log_err("get method of json error!");
            return FAIL_RETURN;
        }
        msg->method  = LITE_malloc(strlen(node) + 1);
        if (NULL == msg->method ) {
            log_info("memory error");
            LITE_free(node);
            return FAIL_RETURN;
        }
        memset(msg->method , 0x0, strlen(node));
        strncpy(msg->method , node, strlen(node));
        LITE_free(node);
        node = NULL; 

        msg->code = 0;
    }
    
    return SUCCESS_RETURN;
}

int iotx_cmp_splice_payload(void* payload, 
                    int* payload_length, 
                    int id,
                    iotx_cmp_message_info_pt msg) 
{    
    if (NULL == payload || NULL == payload_length || NULL == msg) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    switch(msg->message_type) {
        case IOTX_CMP_MESSAGE_RESPONSE: {                      
            HAL_Snprintf(payload,
                   (*payload_length),
                   "{\"id\":%d,\"code\":%d,\"data\":%s}",
                   id,
                   msg->code,
                   (char*)msg->parameter);
            (*payload_length) = strlen(payload);
        }
            break;
        
        case IOTX_CMP_MESSAGE_REQUEST: {                   
            HAL_Snprintf(payload,
                   (*payload_length),
                   "{\"id\":%d,\"version\":\"1.0\",\"params\":%s,\"method\":\"%s\"}",
                   id,
                   (char*)msg->parameter,
                   msg->method);
            (*payload_length) = strlen(payload);
        }
            break;
        
        case IOTX_CMP_MESSAGE_RAW: {
            memcpy(payload, msg->parameter, msg->parameter_length);
            (*payload_length) = msg->parameter_length;
        }
            break;

        default:
            break;
    }
    
    return SUCCESS_RETURN;
}


void iotx_cmp_response_func(void* pcontext, iotx_cmp_response_info_pt msg)
{
    iotx_cmp_send_peer_pt peer = NULL;
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)pcontext;  
    iotx_cmp_message_info_t message_info = {0};
    char topic[CMP_TOPIC_LEN_MAX] = {0};  
    iotx_cmp_mapping_pt mapping = NULL;  
    iotx_cmp_uri_types_t uri_type;
    iotx_device_info_pt pdevice_info = iotx_device_info_get();
    
    if (NULL == cmp_pt || NULL == msg) {
        log_info("parameter error");
        return;
    }

    if (FAIL_RETURN == iotx_cmp_parse_URI(msg->URI, topic, &uri_type)) {
        log_info("parse URI fail");
        return;
    }

    message_info.URI = LITE_malloc(strlen(topic) + 1);
    memset(message_info.URI, 0x0, strlen(topic) + 1);
    strncpy(message_info.URI, topic, strlen(topic));
    message_info.URI_type = uri_type;    
    
    /* find mapping */
    mapping = iotx_cmp_find_mapping(cmp_pt, msg->URI);   
        
    if (NULL == mapping) {
        log_info("can not find mapping");
        LITE_free(message_info.URI);
        return;
    }
    
    if (NULL == mapping->func) {
        log_info("can not find func");
        LITE_free(message_info.URI);
        return;
    }
    
    peer = LITE_malloc(sizeof(iotx_cmp_send_peer_t)); 
    if (NULL == peer) {
        log_info("memory error");
        LITE_free(message_info.URI);
        return;
    }
    
    memset(peer, 0x0, sizeof(iotx_cmp_send_peer_t));
    strncpy(peer->product_key, pdevice_info->product_key, strlen(pdevice_info->product_key));
    strncpy(peer->device_name, pdevice_info->device_name, strlen(pdevice_info->device_name));

    if (SUCCESS_RETURN == iotx_cmp_parse_payload(msg->payload, msg->payload_length, &message_info)) {            
        log_info("mapping function call");            
        mapping->func(peer, &message_info, mapping->user_data);   
        LITE_free(peer);
        return;
    }

    if (peer)
        LITE_free(peer);
    if (message_info.URI)
        LITE_free(message_info.URI);
    if (message_info.parameter)
        LITE_free(message_info.parameter);
    if (message_info.method)
        LITE_free(message_info.method);
}


int iotx_cmp_parse_URI(char* topic, char* URI, iotx_cmp_uri_types_t* uri_type)
{    
    char pk_dv[CMP_PRODUCT_KEY_LEN + CMP_DEVICE_NAME_LEN + 10] = {0};
    char* temp = NULL;
    iotx_device_info_pt pdevice_info = iotx_device_info_get();

    if (NULL == topic || NULL == URI) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    if (0 == strlen(topic)){
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    HAL_Snprintf(pk_dv,
           CMP_PRODUCT_KEY_LEN + CMP_DEVICE_NAME_LEN + 10,
           "/sys/%s/%s/",
           pdevice_info->product_key,
           pdevice_info->device_name);

    /* find /sys/product_key/device_name */
    temp = strstr(topic, pk_dv);

    if (temp) {
        temp = topic + strlen(pk_dv);
        strncpy(URI, temp, strlen(temp));
        (*uri_type) = IOTX_CMP_URI_SYS;
        log_info("topic type: /SYS/ ");
        log_info("topic: %s", URI);
        return SUCCESS_RETURN;
    }
    
    memset(pk_dv, 0x0, CMP_PRODUCT_KEY_LEN + CMP_DEVICE_NAME_LEN + 10);
    HAL_Snprintf(pk_dv,
           CMP_PRODUCT_KEY_LEN + CMP_DEVICE_NAME_LEN + 10,
           "/ext/%s/%s/",
           pdevice_info->product_key,
           pdevice_info->device_name);

    /* find /ext/product_key/device_name */
    temp = strstr(topic, pk_dv);

    if (temp) {
        temp = topic + strlen(pk_dv);
        strncpy(URI, temp, strlen(temp));
        (*uri_type) = IOTX_CMP_URI_EXT;
        log_info("topic type: /EXT/ ");
        log_info("topic: %s", URI);
        return SUCCESS_RETURN;
    }
    
    strncpy(URI, topic, strlen(topic));
    (*uri_type) = IOTX_CMP_URI_UNDEFINE;
    log_info("topic type: /UNDEFINE/ ");
    log_info("topic: %s", URI);
    return SUCCESS_RETURN;
}


int iotx_cmp_splice_URI(char* topic, int* topic_length, const char* URI, iotx_cmp_uri_types_t uri_type)
{
    iotx_device_info_pt pdevice_info = iotx_device_info_get();
    
    if (NULL == topic || NULL == URI) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    if (0 == strlen(URI)){
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    memset(topic, 0x0, (*topic_length));

    switch(uri_type) {
        case IOTX_CMP_URI_SYS: {            
            HAL_Snprintf(topic,
                   CMP_PRODUCT_KEY_LEN + CMP_DEVICE_NAME_LEN + 10,
                   "/sys/%s/%s/%s",
                   pdevice_info->product_key,
                   pdevice_info->device_name,
                   URI);                      
        }
            break;

        case IOTX_CMP_URI_EXT: {       
            HAL_Snprintf(topic,
                   CMP_PRODUCT_KEY_LEN + CMP_DEVICE_NAME_LEN + 10,
                   "/ext/%s/%s/%s",
                   pdevice_info->product_key,
                   pdevice_info->device_name,
                   URI);          
        }
            break;

        default: {
            strncpy(topic, URI, strlen(URI));
        }
            break;
    }

    (*topic_length) = strlen(topic);

    log_info("topic_length: %d", *topic_length);
    log_info("topic: %s", topic);
    
    return SUCCESS_RETURN;
}


int iotx_cmp_add_mapping(iotx_cmp_conntext_pt cmp_pt, char* URI, iotx_cmp_register_func_fpt func, void* user_data)
{
    iotx_cmp_mapping_pt mapping = NULL;

    if (NULL == cmp_pt || NULL == URI) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    mapping = LITE_malloc(sizeof(iotx_cmp_mapping_t));
    if (NULL == mapping){
        log_info("memory error");
        return FAIL_RETURN;
    }
    memset(mapping, 0x0, sizeof(iotx_cmp_mapping_t));
    
    mapping->URI = LITE_malloc(strlen(URI) + 1);
    if (NULL == mapping){
        log_info("memory error");
        return FAIL_RETURN;
    }
    memset(mapping->URI, 0x0, strlen(URI) + 1);
    strncpy(mapping->URI, URI, strlen(URI));

    mapping->func = func;
    mapping->user_data = user_data;

    mapping->next = cmp_pt->mapping_list;
    cmp_pt->mapping_list = mapping;

    return SUCCESS_RETURN;
}

/* remove */
int iotx_cmp_remove_mapping(iotx_cmp_conntext_pt cmp_pt, char* URI)
{
    iotx_cmp_mapping_pt mapping = NULL;
    iotx_cmp_mapping_pt pre_mapping = NULL;
    
    if (NULL == cmp_pt) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    pre_mapping = mapping = cmp_pt->mapping_list;

    /* first one is match */
    if (0 == strncmp(URI, mapping->URI, strlen(URI))) {
        if (NULL == mapping->next) {
            cmp_pt->mapping_list = NULL;
        } else {
            cmp_pt->mapping_list = mapping->next;
        }
    }

    while (mapping) {
        if (0 == strncmp(URI, mapping->URI, strlen(URI))) {
            log_info("find mapping and remove it");
            pre_mapping->next = mapping->next;        

            LITE_free(mapping->URI);
            LITE_free(mapping);   
            return SUCCESS_RETURN;
        }

        pre_mapping = mapping;
        mapping = mapping->next;
    }

    return FAIL_RETURN;
}

/* remove all */
int iotx_cmp_remove_mapping_all(iotx_cmp_conntext_pt cmp_pt)
{
    iotx_cmp_mapping_pt mapping = NULL;
    iotx_cmp_mapping_pt next_mapping = NULL;
    
    if (NULL == cmp_pt) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    next_mapping = mapping = cmp_pt->mapping_list;

    while (mapping) {
        next_mapping = mapping->next;        

        LITE_free(mapping->URI);
        LITE_free(mapping);   

        mapping = next_mapping;
    }

    cmp_pt->mapping_list = NULL;

    return FAIL_RETURN;
}

iotx_cmp_mapping_pt iotx_cmp_find_mapping(iotx_cmp_conntext_pt cmp_pt, char* URI)
{
    iotx_cmp_mapping_pt mapping = NULL;
    
    if (NULL == cmp_pt) {
        log_info("parameter error");
        return NULL;
    }

    mapping = cmp_pt->mapping_list;

    while (mapping) {
        if (0 == strncmp(URI, mapping->URI, strlen(URI))) {
            log_info("find mapping");
            return mapping;
        }

        mapping = mapping->next;
    }

    return NULL;
}


int iotx_cmp_register_service(iotx_cmp_conntext_pt cmp_pt, 
                char* URI, 
                iotx_cmp_register_func_fpt register_func, 
                void* user_data) 
{    
    if (NULL == cmp_pt || NULL == URI || NULL == register_func) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
#ifdef CMP_VIA_MQTT_DIRECT
    if (FAIL_RETURN == iotx_cmp_mqtt_direct_register(cmp_pt, URI)) {
        log_info("register fail");
        return FAIL_RETURN;
    }
#else
    if (FAIL_RETURN == iotx_cmp_cloud_conn_register(cmp_pt, URI)) {
        log_info("register fail");
        return FAIL_RETURN;
    }
#endif

    if (FAIL_RETURN == iotx_cmp_add_mapping(cmp_pt, URI, register_func, user_data)) {
        log_info("add mapping fail");
        return FAIL_RETURN;
    }    

    return SUCCESS_RETURN;
}

                
int iotx_cmp_unregister_service(iotx_cmp_conntext_pt cmp_pt, char* URI) 
{    
    if (NULL == cmp_pt || NULL == URI) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
#ifdef CMP_VIA_MQTT_DIRECT
    if (FAIL_RETURN == iotx_cmp_mqtt_direct_unregister(cmp_pt, URI)) {
        log_info("register fail");
        return FAIL_RETURN;
    }
#else
    if (FAIL_RETURN == iotx_cmp_cloud_conn_unregister(cmp_pt, URI)) {
        log_info("register fail");
        return FAIL_RETURN;
    }
#endif

    if (FAIL_RETURN == iotx_cmp_remove_mapping(cmp_pt, URI)) {
        log_info("remove mapping fail");
        return FAIL_RETURN;
    }    

    return SUCCESS_RETURN;
}

static int iotx_cmp_send_dispatch(iotx_cmp_send_peer_pt target)
{
    iotx_device_info_pt pdevice_info = iotx_device_info_get();   

    if (target == NULL) {
        log_info("send all");
        return 1;
    }

    log_info("target->product_key %s", target->product_key);
    log_info("pdevice_info->product_key %s", pdevice_info->product_key);
    log_info("target->device_name %s", target->device_name);
    log_info("pdevice_info->device_name %s", pdevice_info->device_name);

    if (0 == strncmp(target->product_key, pdevice_info->product_key, strlen(target->product_key)) &&
        0 == strncmp(target->device_name, pdevice_info->device_name, strlen(target->device_name))) {
        log_info("send to cloud");
        return 2;
    }

    log_info("send to local");
    
    return 3;
}

static int iotx_cmp_send_data_to_cloud(iotx_cmp_conntext_pt cmp_pt, char* URI, void* payload, int payload_length)
{
#ifdef CMP_VIA_MQTT_DIRECT
    return iotx_cmp_mqtt_direct_send(cmp_pt, URI, payload, payload_length);
#else
    return iotx_cmp_cloud_conn_send(cmp_pt, URI, payload, payload_length);
#endif    
}

static int iotx_cmp_send_data_to_local(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_send_peer_pt target, char* URI, void* payload, int payload_length)
{
    return FAIL_RETURN;
}

static int iotx_cmp_send_data_to_all(iotx_cmp_conntext_pt cmp_pt, char* URI, void* payload, int payload_length)
{
    if (FAIL_RETURN == iotx_cmp_send_data_to_cloud(cmp_pt, URI, payload, payload_length)) {
        log_err("send to cloud fail");
    }

    /* iotx_cmp_send_data_to_local */
    return FAIL_RETURN;
}

static void iotx_cmp_free_message_info(iotx_cmp_message_info_pt message_info)
{
    if (NULL == message_info) {
        log_info("parameter error");
        return;
    }
    
    if (message_info->URI)
        LITE_free(message_info->URI);
    if (message_info->parameter)
        LITE_free(message_info->parameter);
    if (message_info->method)
        LITE_free(message_info->method);
}


int iotx_cmp_send_data(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_send_peer_pt target, iotx_cmp_message_info_pt message_info)
{
    int peer = -1;
    char* payload = NULL;
    int payload_length = 0;
    char* URI = NULL; 
    int URI_length = CMP_TOPIC_LEN_MAX;
    int rc = -1;

    URI = LITE_malloc(CMP_TOPIC_LEN_MAX);
    if (NULL == URI) {
        log_info("memory error");
        iotx_cmp_free_message_info(message_info);     
        return FAIL_RETURN;
    }

    if (FAIL_RETURN == iotx_cmp_splice_URI(URI, &URI_length, message_info->URI, message_info->URI_type)) {
        log_info("splice payload fail");
        iotx_cmp_free_message_info(message_info);      
        LITE_free(URI);        

        return FAIL_RETURN;
    }

    payload_length = strlen(message_info->method) + 
        message_info->parameter_length + 
        strlen("{\"id\":%d,\"version\":\"1.0\",\"params\":%s,\"method\":%s}");

    log_info("payload_length %d", payload_length);
    
    payload = LITE_malloc(payload_length);
    if (NULL == payload) {
        log_info("memory error");
        iotx_cmp_free_message_info(message_info);     
        LITE_free(URI);        
        LITE_free(payload); 
        return FAIL_RETURN;
    }

    if (0 == message_info->id) {
        message_info->id = iotx_cmp_get_next_message_id();
    }

    if (FAIL_RETURN == iotx_cmp_splice_payload(payload, &payload_length, message_info->id, message_info)) {
        log_info("splice payload fail");
        iotx_cmp_free_message_info(message_info);     
        LITE_free(URI);        
        LITE_free(payload);         
        return FAIL_RETURN;
    }

    /* free memory */
    iotx_cmp_free_message_info(message_info); 

    peer = iotx_cmp_send_dispatch(target); 

    switch (peer) {
        /* send all */
        case 1: {
            rc = iotx_cmp_send_data_to_all(cmp_pt, URI, payload, payload_length);
        }
            break;

        /* send cloud */
        case 2:{
            rc = iotx_cmp_send_data_to_cloud(cmp_pt, URI, payload, payload_length);
        }
            break;
        
        case 3:{
            rc = iotx_cmp_send_data_to_local(cmp_pt, target, URI, payload, payload_length);
        }
            break;
        default:{
            log_info("send dispatch fail");
            return FAIL_RETURN;
        }
            break;
    }
     
    LITE_free(URI);     
    LITE_free(payload); 
    return rc;
}


#ifdef CMP_SUPPORT_MULTI_THREAD
/* node is in */
int iotx_cmp_process_list_push(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_process_list_node_pt node)
{
    if (NULL == cmp_pt || NULL == node) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    HAL_MutexLock(cmp_pt->process_lock);    

    if (NULL == cmp_pt->process_list) {
        cmp_pt->process_list = LITE_malloc(sizeof(iotx_cmp_process_list_t));

        if (NULL == cmp_pt->process_list) {
            log_info("memory error");
            return FAIL_RETURN;
        }

        cmp_pt->process_list->header = cmp_pt->process_list->tailer = NULL;
        cmp_pt->process_list->size = 0;
    }    
 
    if (cmp_pt->process_list->size == 0) {
        cmp_pt->process_list->header = cmp_pt->process_list->tailer = node;
        cmp_pt->process_list->size = 1;
    } else {
        node->pre = cmp_pt->process_list->tailer;
        cmp_pt->process_list->tailer->next = node;
        cmp_pt->process_list->tailer = node;
        cmp_pt->process_list->size++;
    }
    log_info("list_size %d", cmp_pt->process_list->size);
    
    HAL_MutexUnlock(cmp_pt->process_lock);

    if (IOTX_CMP_PROCESS_NEW_DATA == node->type) {
        iotx_cmp_process_response_pt rsp_pt = (iotx_cmp_process_response_pt)node->msg;
        log_info("URI %s", (char*)rsp_pt->URI);
        log_info("payload %s", (char*)rsp_pt->payload);
    }
        
    return SUCCESS_RETURN;
}


iotx_cmp_process_list_node_pt iotx_cmp_process_list_pop(iotx_cmp_conntext_pt cmp_pt)
{
    iotx_cmp_process_list_node_pt node = NULL;

    if (NULL == cmp_pt) {
        log_info("parameter error");
        return NULL;
    }

    if (NULL == cmp_pt->process_list) {
        log_info("status error");
        return NULL;
    }
    
    HAL_MutexLock(cmp_pt->process_lock);        

    if (cmp_pt->process_list->size == 1) {
        node = cmp_pt->process_list->header;
        cmp_pt->process_list->header = cmp_pt->process_list->tailer = NULL;
        cmp_pt->process_list->size = 0;
        LITE_free(cmp_pt->process_list);
        cmp_pt->process_list = NULL;
    } else {
        node = cmp_pt->process_list->header;
        cmp_pt->process_list->header = cmp_pt->process_list->header->next;
        cmp_pt->process_list->header->pre = NULL;
        cmp_pt->process_list->size--;
    }
    
    HAL_MutexUnlock(cmp_pt->process_lock);    
    
    if (IOTX_CMP_PROCESS_NEW_DATA == node->type) {
        iotx_cmp_process_response_pt rsp_pt = (iotx_cmp_process_response_pt)node->msg;
        log_info("URI %s", (char*)rsp_pt->URI);
        log_info("payload %s", (char*)rsp_pt->payload);
    }
    
    return node;
}


int iotx_cmp_process_list_get_size(iotx_cmp_conntext_pt cmp_pt)
{
    if (NULL == cmp_pt) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    if (NULL == cmp_pt->process_list) {
        log_info("list is null");
        return 0;
    }

    log_info("%d", cmp_pt->process_list->size);
    
    return cmp_pt->process_list->size;
}


void* iotx_cmp_process(void *pclient)
{
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)pclient;

    if (NULL == cmp_pt) {
        log_info("parameter error");
        return NULL;
    }

    log_info("enter cmp process");

    while (1) {
        if (iotx_cmp_process_list_get_size(cmp_pt) > 0) {
            iotx_cmp_process_list_node_pt node = NULL;
            if (NULL != (node = iotx_cmp_process_list_pop(cmp_pt))) {
                log_info("process %d", node->type);

                switch(node->type) {
                    case IOTX_CMP_PROCESS_REGISTER: {
                        iotx_cmp_process_register_pt register_pt = (iotx_cmp_process_register_pt)node->msg;                        
                        iotx_cmp_register_service(cmp_pt, register_pt->URI, register_pt->register_func, register_pt->user_data);
                        LITE_free(register_pt->URI);
                        LITE_free(register_pt);
                    }
                        break;
                
                    case IOTX_CMP_PROCESS_UNREGISTER: {
                        char* URI = (char*)node->msg;
                        iotx_cmp_unregister_service(cmp_pt, URI);
                        LITE_free(URI);
                    }
                        break;
                        
                    case IOTX_CMP_PROCESS_SEND: {
                        iotx_cmp_process_send_pt send_pt = (iotx_cmp_process_send_pt)node->msg;
                        iotx_cmp_send_data(cmp_pt, send_pt->target, send_pt->message_info);                        
                        LITE_free(send_pt->target);
                        LITE_free(send_pt);
                    }
                        break;

                    case IOTX_CMP_PROCESS_CLOUD_DISCONNECT: {
                    #ifdef CMP_VIA_MQTT_DIRECT
                        iotx_cmp_mqtt_direct_disconnect_handler(cmp_pt);
                    #else
                        iotx_cmp_cloud_conn_disconnect_handler(cmp_pt);
                    #endif
                    }
                        break;

                    case IOTX_CMP_PROCESS_CLOUD_RECONNECT: {
                    #ifdef CMP_VIA_MQTT_DIRECT
                        iotx_cmp_mqtt_direct_reconnect_handler(cmp_pt);
                    #else
                        iotx_cmp_cloud_conn_reconnect_handler(cmp_pt);
                    #endif
                    }
                        break;

                    case IOTX_CMP_PROCESS_REGISTER_RESULT:
                    case IOTX_CMP_PROCESS_UNREGISTER_RESULT: {
                        iotx_cmp_process_register_result_pt register_pt = (iotx_cmp_process_register_result_pt)node->msg;
                    #ifdef CMP_VIA_MQTT_DIRECT
                        iotx_cmp_mqtt_direct_register_handler(cmp_pt, register_pt->URI, register_pt->result, register_pt->is_register);
                    #else
                        iotx_cmp_cloud_conn_register_handler(cmp_pt, register_pt->URI, register_pt->result, register_pt->is_register);
                        LITE_free(register_pt->URI);                
                    #endif
                        LITE_free(register_pt);
                    }
                        break;

                    case IOTX_CMP_PROCESS_NEW_DATA: {
                        iotx_cmp_process_response_pt response_data = (iotx_cmp_process_response_pt)node->msg;
                    #ifdef CMP_VIA_MQTT_DIRECT
                        iotx_cmp_mqtt_direct_response_handler(cmp_pt, response_data);
                    #else
                        iotx_cmp_cloud_conn_response_handler(cmp_pt, response_data);
                    #endif    
                        LITE_free(response_data->URI);  
                        LITE_free(response_data->payload);
                        LITE_free(node->msg);
                        break;
                    }
                        
                    default:
                        break;
                }

                LITE_free(node);
            }
        } else {
            HAL_SleepMs(1000);
        }
        
        HAL_SleepMs(200);
    }
    
    return NULL;
}

void* iotx_cmp_yield_process(void *pclient)
{
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)pclient;

    if (NULL == cmp_pt) {
        log_info("parameter error");
        return NULL;
    }

    log_info("enter yield process");

    while (1) {
        /* cloud yield */
    #ifdef CMP_VIA_MQTT_DIRECT
        iotx_cmp_mqtt_direct_yield(cmp_pt, 200);
    #else
        iotx_cmp_cloud_conn_yield(cmp_pt, 200);
    #endif    

        /* local yield */

        HAL_SleepMs(200);
    }
    
    return NULL;
}
#endif



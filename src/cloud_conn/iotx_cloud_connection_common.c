#include "iot_import.h"
#include "lite-log.h"
#include "lite-utils.h"
#include "utils_timer.h"
#include "utils_list.h"
#include "lite-system.h"
#include "utils_hmac.h"

#include "iotx_cloud_connection_common.h"

int iotx_cloud_connection_check_function_context(iotx_cloud_connection_pt connection)
{
    if (NULL == connection) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    if (NULL == connection->function_pt){
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    if (NULL == connection->function_pt->pcontext){
        log_info("parameter error");
        return FAIL_RETURN;
    }    

    return SUCCESS_RETURN;
}

int iotx_cloud_connection_add_mapping(iotx_cloud_connection_pt connection, char* topic, iotx_cloud_connection_msg_rsp_func_fpt func, void* pcontext)
{
    iotx_cloud_connection_mapping_pt mapping = NULL;

    if (NULL == connection) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    mapping = LITE_malloc(sizeof(iotx_cloud_connection_mapping_t));
    if (NULL == mapping){
        log_info("memory error");
        return FAIL_RETURN;
    }
    memset(mapping, 0x0, sizeof(iotx_cloud_connection_mapping_t));
    
    mapping->topic = LITE_malloc(strlen(topic) + 1);
    if (NULL == mapping){
        log_info("memory error");
        return FAIL_RETURN;
    }
    memset(mapping->topic, 0x0, strlen(topic) + 1);
    strncpy(mapping->topic, topic, strlen(topic));

    mapping->func = func;
    mapping->pcontext = pcontext;

    mapping->next = connection->mapping_list;
    connection->mapping_list = mapping;

    return SUCCESS_RETURN;
}

/* remove */
int iotx_cloud_connection_remove_mapping(iotx_cloud_connection_pt connection, char* topic)
{
    iotx_cloud_connection_mapping_pt mapping = NULL;
    iotx_cloud_connection_mapping_pt pre_mapping = NULL;
    
    if (NULL == connection) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    pre_mapping = mapping = connection->mapping_list;

    /* first one is match */
    if (0 == strncmp(topic, mapping->topic, strlen(topic))) {
        if (NULL == mapping->next) {
            connection->mapping_list = NULL;
        } else {
            connection->mapping_list = mapping->next;
        }
    }

    while (mapping) {
        if (0 == strncmp(topic, mapping->topic, strlen(topic))) {
            log_info("find mapping and remove it");
            pre_mapping->next = mapping->next;        

            LITE_free(mapping->topic);
            LITE_free(mapping);   
            return SUCCESS_RETURN;
        }

        pre_mapping = mapping;
        mapping = mapping->next;
    }

    return FAIL_RETURN;
}

/* remove all */
int iotx_cloud_connection_remove_mapping_all(iotx_cloud_connection_pt connection)
{
    iotx_cloud_connection_mapping_pt mapping = NULL;
    iotx_cloud_connection_mapping_pt next_mapping = NULL;
    
    if (NULL == connection) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    next_mapping = mapping = connection->mapping_list;

    while (mapping) {
        next_mapping = mapping->next;        

        LITE_free(mapping->topic);
        LITE_free(mapping);   

        mapping = next_mapping;
    }

    return FAIL_RETURN;
}

iotx_cloud_connection_mapping_pt iotx_cloud_connection_find_mapping(iotx_cloud_connection_pt connection, char* topic)
{
    iotx_cloud_connection_mapping_pt mapping = NULL;
    
    if (NULL == connection) {
        log_info("parameter error");
        return NULL;
    }

    mapping = connection->mapping_list;

    while (mapping) {
        if (0 == strncmp(topic, mapping->topic, strlen(topic))) {
            log_info("find mapping");
            return mapping;
        }

        mapping = mapping->next;
    }

    return NULL;
}

int iotx_cloud_connection_resposne(void* handle, iotx_cloud_connection_msg_rsp_pt msg)
{    
    iotx_cloud_connection_mapping_pt mapping = NULL;
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)handle;
    
    if (NULL == connection || NULL == msg) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    mapping = iotx_cloud_connection_find_mapping(connection, msg->URI);

    if (mapping == NULL || NULL == mapping->func) {
        log_info("not find callback");
        return FAIL_RETURN;
    }

    mapping->func(mapping->pcontext, connection, msg);

    if (msg->rsp_type == IOTX_CLOUD_CONNECTION_RESPONSE_UNSUBSCRIBE_SUCCESS || 
        msg->rsp_type == IOTX_CLOUD_CONNECTION_RESPONSE_UNSUBSCRIBE_FAIL) {
        iotx_cloud_connection_remove_mapping(connection, msg->URI);
    }

    return SUCCESS_RETURN;
}



#include "iot_import.h"

#include "utils_list.h"
#include "lite-utils.h"
#include "device.h"
#include "iotx_cloud_connection_common.h"

iotx_cloud_connection_pt g_cloud_connection_t = NULL;

void* IOT_Cloud_Connection_Init(iotx_cloud_connection_param_pt connection_param)
{
    /* parameter check */
    if (NULL == connection_param) {
        log_info("parameter error: connection_param is null!");
        return NULL;
    }

    if (NULL == connection_param->device_info){
        log_info("parameter error: device_info is null!");
        return NULL;
    }

    if (IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_MQTT != connection_param->protocol_type
        && IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_COAP != connection_param->protocol_type
        && IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_HTTP != connection_param->protocol_type) {
        log_info("parameter error: protocol type is wrong, it is just support MQTT, CoAP and HTTP");
        return NULL;
    }

    /* initial */
    g_cloud_connection_t = LITE_malloc(sizeof(iotx_cloud_connection_t)); 
    if (NULL == g_cloud_connection_t) {
        log_info("memory alloc fail");
        return NULL;
    }

    g_cloud_connection_t->device_info = LITE_malloc(sizeof(iotx_deviceinfo_t));
    if (NULL == g_cloud_connection_t->device_info) {
        LITE_free(g_cloud_connection_t);
        g_cloud_connection_t = NULL;
        log_info("memory alloc fail");
        return NULL;
    }

    /* initial device info */
    if (NULL != connection_param->device_info->device_id)
        strncpy(g_cloud_connection_t->device_info->device_id, connection_param->device_info->device_id, strlen(connection_param->device_info->device_id));

    if (NULL != connection_param->device_info->product_key)
        strncpy(g_cloud_connection_t->device_info->product_key, connection_param->device_info->product_key, strlen(connection_param->device_info->product_key));
    
    if (NULL != connection_param->device_info->device_name)
        strncpy(g_cloud_connection_t->device_info->device_name, connection_param->device_info->device_name, strlen(connection_param->device_info->device_name));

    if (NULL != connection_param->device_info->device_secret)
        strncpy(g_cloud_connection_t->device_info->device_secret, connection_param->device_info->device_secret, strlen(connection_param->device_info->device_secret));

    if (NULL != connection_param->device_info->module_vendor_id)
        strncpy(g_cloud_connection_t->device_info->module_vendor_id, connection_param->device_info->module_vendor_id, strlen(connection_param->device_info->module_vendor_id));
    
    g_cloud_connection_t->clean_session = connection_param->clean_session;
    g_cloud_connection_t->event_handler = connection_param->event_handler;
    g_cloud_connection_t->event_pcontext = connection_param->event_pcontext;
    g_cloud_connection_t->request_timeout_ms = connection_param->request_timeout_ms;
    g_cloud_connection_t->resposne_handler = iotx_cloud_connection_resposne;   

    g_cloud_connection_t->function_pt = LITE_malloc(sizeof(iotx_cloud_connection_function_t));
    if (NULL == g_cloud_connection_t->function_pt) {
        LITE_free(g_cloud_connection_t->device_info);
        LITE_free(g_cloud_connection_t);
        g_cloud_connection_t = NULL;
        log_info("memory alloc fail");
        return NULL;
    }

    /* initial function */
    switch(connection_param->protocol_type) {
        case IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_MQTT:
        {            
            g_cloud_connection_t->function_pt->init_func = iotx_cloud_connection_mqtt_init;
            g_cloud_connection_t->function_pt->deinit_func = iotx_cloud_connection_mqtt_deinit;
            g_cloud_connection_t->function_pt->sub_func = iotx_cloud_connection_mqtt_subscribe;
            g_cloud_connection_t->function_pt->unsub_func = iotx_cloud_connection_mqtt_unsubscribe;
            g_cloud_connection_t->function_pt->pub_func = iotx_cloud_connection_mqtt_publish;
            g_cloud_connection_t->function_pt->yield_func = iotx_cloud_connection_mqtt_yield;

        }
        break;
        
        case IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_COAP:
        {            
            g_cloud_connection_t->function_pt->init_func = iotx_cloud_connection_coap_init;
            g_cloud_connection_t->function_pt->deinit_func = iotx_cloud_connection_coap_deinit;
            g_cloud_connection_t->function_pt->sub_func = iotx_cloud_connection_coap_subscribe;
            g_cloud_connection_t->function_pt->unsub_func = iotx_cloud_connection_coap_unsubscribe;
            g_cloud_connection_t->function_pt->pub_func = iotx_cloud_connection_coap_publish;
            g_cloud_connection_t->function_pt->yield_func = iotx_cloud_connection_coap_yield;

        }
        break;
        
        case IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_HTTP:
        {            
            g_cloud_connection_t->function_pt->init_func = iotx_cloud_connection_http_init;
            g_cloud_connection_t->function_pt->deinit_func = iotx_cloud_connection_http_deinit;
            g_cloud_connection_t->function_pt->sub_func = iotx_cloud_connection_http_subscribe;
            g_cloud_connection_t->function_pt->unsub_func = iotx_cloud_connection_http_unsubscribe;
            g_cloud_connection_t->function_pt->pub_func = iotx_cloud_connection_http_publish;
            g_cloud_connection_t->function_pt->yield_func = iotx_cloud_connection_http_yield;

        }
        break;

        default:
            log_info("error");
        break;
    }
    
    if (NULL == (g_cloud_connection_t->function_pt->pcontext = g_cloud_connection_t->function_pt->init_func(connection_param, g_cloud_connection_t))) {
        LITE_free(g_cloud_connection_t->function_pt);
        LITE_free(g_cloud_connection_t->device_info);
        LITE_free(g_cloud_connection_t);
        g_cloud_connection_t = NULL;
        log_info("initial fail");
        return NULL;
    }

    return g_cloud_connection_t;
}

int IOT_Cloud_Connection_Deinit(void** handle)
{   
    int rc = 0;
    iotx_cloud_connection_pt cloud_connection = NULL;

    /* parameter check */
    if (handle == NULL) {
        log_info("parameter error: handle is null");
        return FAIL_RETURN;
    }
    cloud_connection = (iotx_cloud_connection_pt)(*handle);
    if (cloud_connection != g_cloud_connection_t) {
        log_info("parameter error: handle is not g_cloud_connection_t");
        return FAIL_RETURN;
    }

    if (NULL == cloud_connection->function_pt){
        log_info("parameter error: function_pt is null");
        return FAIL_RETURN;
    }
    
    if (NULL == cloud_connection->function_pt->deinit_func){
        log_info("parameter error: deinit_func is null");
        return FAIL_RETURN;
    }

    /* deiniet */
    rc = cloud_connection->function_pt->deinit_func(cloud_connection);

    log_info("deinit");

    iotx_cloud_connection_remove_mapping_all(cloud_connection);
    log_info("deinit");
    /* free memory */
    LITE_free(cloud_connection->function_pt);
    LITE_free(cloud_connection->device_info);
    LITE_free(cloud_connection);
    g_cloud_connection_t = NULL;    
    (*handle) = NULL;
    
    return rc;
}


int IOT_Cloud_Connection_Send_Message(void* handle, iotx_cloud_connection_msg_pt msg)
{
    iotx_cloud_connection_pt cloud_connection = NULL;

    /* parameter check */
    if (NULL == handle) {
        log_info("parameter error: handle is null");
        return FAIL_RETURN;
    }
    
    if (NULL == msg) {
        log_info("parameter error: msg is null");
        return FAIL_RETURN;
    }
    
    cloud_connection = (iotx_cloud_connection_pt)(handle);
    if (cloud_connection != g_cloud_connection_t) {
        log_info("parameter error: handle is not g_cloud_connection_t");
        return FAIL_RETURN;
    }

    if (NULL == cloud_connection->function_pt){
        log_info("parameter error: function_pt is null");
        return FAIL_RETURN;
    }

    /* dispatch function */
    switch(msg->type) {
        case IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_SUBSCRIBE:{ 
            if (NULL == cloud_connection->function_pt->sub_func){
                log_info("parameter error: sub_func is null");
                return FAIL_RETURN;
            }

            cloud_connection->function_pt->sub_func(cloud_connection, msg->URI, msg->QoS);
            
            iotx_cloud_connection_add_mapping(handle, msg->URI, msg->response_handler, msg->response_pcontext);            
            return SUCCESS_RETURN;
        }
            break;
        
        case IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_UNSUBSCRIBE:{
            if (NULL == cloud_connection->function_pt->unsub_func){
                log_info("parameter error: unsub_func is null");
                return FAIL_RETURN;
            }
            
            cloud_connection->function_pt->unsub_func(cloud_connection, msg->URI);
            return SUCCESS_RETURN;
        }
            break;
        
        case IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_PUBLISH:{
            int rc  = 0;
            if (NULL == cloud_connection->function_pt->pub_func){
                log_info("parameter error: pub_func is null");
                return FAIL_RETURN;
            }

            rc = cloud_connection->function_pt->pub_func(cloud_connection, msg);
            
            if (SUCCESS_RETURN == rc) {
                iotx_cloud_connection_add_mapping(handle, msg->URI, msg->response_handler, msg->response_pcontext);
            }
            
            return rc;
        }
            break;
        
        default: {
            log_info("msg_type error, just support: subscribe, unsubscribe and publish");
        }
            break;   
    }

    return FAIL_RETURN;
}

int IOT_Cloud_Connection_Yield(void *handle, int timeout_ms)
{
    iotx_cloud_connection_pt cloud_connection = NULL;

    /* parameter check */
    if (NULL == handle) {
        log_info("parameter error: handle is null");
        return FAIL_RETURN;
    }
    
    cloud_connection = (iotx_cloud_connection_pt)(handle);
    if (cloud_connection != g_cloud_connection_t) {
        log_info("parameter error: handle is not g_cloud_connection_t");
        return FAIL_RETURN;
    }

    if (NULL == cloud_connection->function_pt){
        log_info("parameter error: function_pt is null");
        return FAIL_RETURN;
    }
    
    if (NULL == cloud_connection->function_pt->yield_func){
        log_info("parameter error: sub_func is null");
        return FAIL_RETURN;
    }

    /* yield */
    return cloud_connection->function_pt->yield_func(cloud_connection, timeout_ms);
}



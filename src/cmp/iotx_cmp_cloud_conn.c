#ifdef CMP_VIA_CLOUN_CONN

#include "iot_import.h"

#include "utils_list.h"
#include "lite-utils.h"
#include "device.h"
#include "iot_export.h"
#include "iotx_cmp_common.h"
#include "iotx_cmp_cloud_conn.h"


static void iotx_cmp_cloud_conn_event_callback(void *pcontext, iotx_cloud_connection_event_msg_pt msg)
{
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)pcontext;  

    if (NULL == cmp_pt || NULL == msg) {
        log_info("error");
        return;
    }

    log_info("event_id %d", msg->event_id);

    switch (msg->event_id) {
        case IOTX_CLOUD_CONNECTION_EVENT_DISCONNECT: {
        #ifdef CMP_SUPPORT_MULTI_THREAD
        {            
            /* send message to itself thread */
            int rc = 0;
            iotx_cmp_process_list_node_pt node = NULL;
                
            node = LITE_malloc(sizeof(iotx_cmp_process_list_node_t));
            if (NULL == node) {
                log_info("memory error");
                return;
            }

            memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));

            node->type = IOTX_CMP_PROCESS_CLOUD_DISCONNECT;
            node->msg = NULL;
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);

            if (FAIL_RETURN == rc) {  
                LITE_free(node);
            }
        }            
        #else
            iotx_cmp_cloud_conn_disconnect_handler(cmp_pt);
        #endif
        }
            break;

        case IOTX_CLOUD_CONNECTION_EVENT_RECONNECT: {
        #ifdef CMP_SUPPORT_MULTI_THREAD
        {            
            /* send message to itself thread */
            int rc = 0;
            iotx_cmp_process_list_node_pt node = NULL;
                
            node = LITE_malloc(sizeof(iotx_cmp_process_list_node_t));
            if (NULL == node) {
                log_info("memory error");
                return;
            }

            memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));

            node->type = IOTX_CMP_PROCESS_CLOUD_RECONNECT;
            node->msg = NULL;
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);

            if (FAIL_RETURN == rc) {  
                LITE_free(node);
            }
        }
        #else
            iotx_cmp_cloud_conn_reconnect_handler(cmp_pt);
        #endif
        }
            break;
        
        default:
            log_info("Should NOT arrive here.");
            break;
    }
}

static void iotx_cmp_cloud_conn_response_callback(void *pcontext, void *pconnection, iotx_cloud_connection_msg_rsp_pt msg)
{
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)pcontext;  

    if (NULL == cmp_pt || NULL == pconnection || NULL == msg) {
        log_info("error");
        return;
    }
    
    log_info("rsp_type %d", msg->rsp_type);    

    switch (msg->rsp_type) {
        case IOTX_CLOUD_CONNECTION_RESPONSE_SUBSCRIBE_SUCCESS: {  
        #ifdef CMP_SUPPORT_MULTI_THREAD
        {            
            /* send message to itself thread */
            int rc = 0;
            iotx_cmp_process_list_node_pt node = NULL;
            iotx_cmp_process_register_result_pt result_msg = NULL;
                
            node = LITE_malloc(sizeof(iotx_cmp_process_list_node_t));
            if (NULL == node) {
                log_info("memory error");
                return;
            }

            memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));

            node->type = IOTX_CMP_PROCESS_REGISTER_RESULT;
            node->msg = LITE_malloc(sizeof(iotx_cmp_process_register_result_t));
            if (NULL == node) {
                log_info("memory error");
                LITE_free(node);
                return;
            }

            result_msg = node->msg;

            result_msg->URI = LITE_malloc(strlen(msg->URI) + 1);
            if (NULL == result_msg->URI) {                
                LITE_free(node->msg);
                LITE_free(node);
                return;
            }
            memset(result_msg->URI, 0x0, strlen(msg->URI) + 1);
            strncpy(result_msg->URI, (msg->URI), strlen(msg->URI) + 1);
            result_msg->result = 1;
            result_msg->is_register = 1;
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);

            if (FAIL_RETURN == rc) {                  
                LITE_free(result_msg->URI);
                LITE_free(node->msg);
                LITE_free(node);
            }
        }
        #else
            iotx_cmp_cloud_conn_register_handler(cmp_pt, msg->URI, 1, 1);
        #endif
        }
            break;
        
        case IOTX_CLOUD_CONNECTION_RESPONSE_SUBSCRIBE_FAIL: { 
        #ifdef CMP_SUPPORT_MULTI_THREAD
        {            
            /* send message to itself thread */
            int rc = 0;
            iotx_cmp_process_list_node_pt node = NULL;
            iotx_cmp_process_register_result_pt result_msg = NULL;
                
            node = LITE_malloc(sizeof(iotx_cmp_process_list_node_t));
            if (NULL == node) {
                log_info("memory error");
                return;
            }

            memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));

            node->type = IOTX_CMP_PROCESS_REGISTER_RESULT;
            node->msg = LITE_malloc(sizeof(iotx_cmp_process_register_result_t));
            if (NULL == node) {
                log_info("memory error");
                LITE_free(node);
                return;
            }

            result_msg = node->msg;

            result_msg->URI = LITE_malloc(strlen(msg->URI) + 1);
            if (NULL == result_msg->URI) {                
                LITE_free(node->msg);
                LITE_free(node);
                return;
            }
            memset(result_msg->URI, 0x0, strlen(msg->URI) + 1);
            strncpy(result_msg->URI, (msg->URI), strlen(msg->URI) + 1);

            result_msg->result = 0;
            result_msg->is_register = 1;
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);

            if (FAIL_RETURN == rc) {  
                LITE_free(node->msg);
                LITE_free(node);
            }
        }
        #else
            iotx_cmp_cloud_conn_register_handler(cmp_pt, msg->URI, 0, 1);
        #endif
        }
            break;
        
        case IOTX_CLOUD_CONNECTION_RESPONSE_UNSUBSCRIBE_SUCCESS: {  
        #ifdef CMP_SUPPORT_MULTI_THREAD
        {            
            /* send message to itself thread */
            int rc = 0;
            iotx_cmp_process_list_node_pt node = NULL;
            iotx_cmp_process_register_result_pt result_msg = NULL;
                
            node = LITE_malloc(sizeof(iotx_cmp_process_list_node_t));
            if (NULL == node) {
                log_info("memory error");
                return;
            }

            memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));

            node->type = IOTX_CMP_PROCESS_UNREGISTER_RESULT;
            node->msg = LITE_malloc(sizeof(iotx_cmp_process_register_result_t));
            if (NULL == node) {
                log_info("memory error");
                LITE_free(node);
                return;
            }

            result_msg = node->msg;

            result_msg->URI = LITE_malloc(strlen(msg->URI) + 1);
            if (NULL == result_msg->URI) {                
                LITE_free(node->msg);
                LITE_free(node);
                return;
            }
            memset(result_msg->URI, 0x0, strlen(msg->URI) + 1);
            strncpy(result_msg->URI, (msg->URI), strlen(msg->URI) + 1);
            result_msg->result = 1;
            result_msg->is_register = 0;
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);

            if (FAIL_RETURN == rc) {  
                LITE_free(node->msg);
                LITE_free(node);
            }
        }
        #else
            iotx_cmp_cloud_conn_register_handler(cmp_pt, msg->URI, 1, 0);
        #endif
        }
            break;
        
        case IOTX_CLOUD_CONNECTION_RESPONSE_UNSUBSCRIBE_FAIL: {  
        #ifdef CMP_SUPPORT_MULTI_THREAD
        {            
            /* send message to itself thread */
            int rc = 0;
            iotx_cmp_process_list_node_pt node = NULL;
            iotx_cmp_process_register_result_pt result_msg = NULL;
                
            node = LITE_malloc(sizeof(iotx_cmp_process_list_node_t));
            if (NULL == node) {
                log_info("memory error");
                return;
            }

            memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));

            node->type = IOTX_CMP_PROCESS_UNREGISTER_RESULT;
            node->msg = LITE_malloc(sizeof(iotx_cmp_process_register_result_t));
            if (NULL == node) {
                log_info("memory error");
                LITE_free(node);
                return;
            }

            result_msg = node->msg;

            result_msg->URI = LITE_malloc(strlen(msg->URI) + 1);
            if (NULL == result_msg->URI) {                
                LITE_free(node->msg);
                LITE_free(node);
                return;
            }
            memset(result_msg->URI, 0x0, strlen(msg->URI) + 1);
            strncpy(result_msg->URI, (msg->URI), strlen(msg->URI) + 1);
            result_msg->result = 0;
            result_msg->is_register = 0;
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);

            if (FAIL_RETURN == rc) {  
                LITE_free(node->msg);
                LITE_free(node);
            }
        }
        #else
            iotx_cmp_cloud_conn_register_handler(cmp_pt, msg->URI, 0, 0);
        #endif
        }
            break;

        case IOTX_CLOUD_CONNECTION_RESPONSE_SEND_SUCCESS:
        case IOTX_CLOUD_CONNECTION_RESPONSE_SEND_FAIL:
        case IOTX_CLOUD_CONNECTION_RESPONSE_NEW_DATA: {
        #ifdef CMP_SUPPORT_MULTI_THREAD
        {            
            /* send message to itself thread */            
            int rc = 0;
            iotx_cmp_process_list_node_pt node = NULL;
            iotx_cmp_process_response_pt rsp_msg = NULL;
                
            node = LITE_malloc(sizeof(iotx_cmp_process_list_node_t));
            if (NULL == node) {
                log_info("memory error");
                return;
            }

            memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));

            node->type = IOTX_CMP_PROCESS_NEW_DATA;
            node->msg = LITE_malloc(sizeof(iotx_cmp_process_response_t));
            if (NULL == node->msg) {
                log_info("memory error");
                LITE_free(node);
                return;
            }

            /* todo memory */
            rsp_msg = node->msg;

            rsp_msg->URI = LITE_malloc(msg->URI_length + 1);
            if (NULL == rsp_msg->URI) {
                log_info("memory error");
                LITE_free(node->msg);
                LITE_free(node);
                return;
            }
            memset(rsp_msg->URI, 0x0, msg->URI_length + 1);
            strncpy(rsp_msg->URI, msg->URI, msg->URI_length);

            rsp_msg->payload = LITE_malloc(msg->payload_length + 1);
            if (NULL == rsp_msg->payload) {
                log_info("memory error");
                LITE_free(rsp_msg->URI);
                LITE_free(node->msg);
                LITE_free(node);
                return;
            }
            memset(rsp_msg->payload, 0x0, msg->payload_length + 1);
            strncpy(rsp_msg->payload, msg->payload, msg->payload_length);         

            rsp_msg->payload_length = msg->payload_length;
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);

            if (FAIL_RETURN == rc) {
                LITE_free(rsp_msg->URI);  
                LITE_free(rsp_msg->payload);
                LITE_free(node->msg);
                LITE_free(node);
            }
        }
        #else
            iotx_cmp_process_response_t response;

            response.URI = msg->URI;
            response.payload = msg->payload;
            response.payload_length = msg->payload_length;
            iotx_cmp_cloud_conn_response_handler(cmp_pt, &response);
        #endif      
        }
            break;
        
        default:
            log_info("Should NOT arrive here.");
            break;
    }
}


int iotx_cmp_cloud_conn_disconnect_handler(iotx_cmp_conntext_pt cmp_pt)
{
    iotx_cmp_event_msg_t event;

    if (NULL == cmp_pt) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
            
    event.event_id = IOTX_CMP_EVENT_CLOUD_DISCONNECT;
    event.msg = NULL;
    
    log_info("cloud_conn disconnect.");
    
    if (cmp_pt->event_func)
        cmp_pt->event_func(cmp_pt, &event, cmp_pt->user_data);

    return SUCCESS_RETURN;
}

int iotx_cmp_cloud_conn_reconnect_handler(iotx_cmp_conntext_pt cmp_pt)
{
    iotx_cmp_event_msg_t event;

    if (NULL == cmp_pt) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
            
    event.event_id = IOTX_CMP_EVENT_CLOUD_RECONNECT;
    event.msg = NULL;
    
    log_info("cloud_conn reconnect.");
    
    if (cmp_pt->event_func)
        cmp_pt->event_func(cmp_pt, &event, cmp_pt->user_data);
    
    return SUCCESS_RETURN;
}

int iotx_cmp_cloud_conn_register_handler(iotx_cmp_conntext_pt cmp_pt, char* URI, int result, int is_register)
{    
    iotx_cmp_event_msg_t event;
    iotx_cmp_event_result_t result_pt = {0};
    
    if (NULL == cmp_pt || NULL == URI) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    result_pt.result = result;
    result_pt.URI = LITE_malloc(CMP_TOPIC_LEN_MAX);
    if (NULL == result_pt.URI) {
        log_info("memory error");
        return FAIL_RETURN;
    }
    memset(result_pt.URI, 0x0, CMP_TOPIC_LEN_MAX);
    
    if (FAIL_RETURN == iotx_cmp_parse_URI(URI, result_pt.URI, &result_pt.URI_type)) {
        log_info("fail");
        LITE_free(result_pt.URI);
        return FAIL_RETURN;
    }
    
    if (is_register)
        event.event_id = IOTX_CMP_EVENT_REGISTER_RESULT;
    else 
        event.event_id = IOTX_CMP_EVENT_UNREGISTER_RESULT;
        
    event.msg = (void*)&result_pt;
    
    log_info("register success.");
    
    if (cmp_pt->event_func)
        cmp_pt->event_func(cmp_pt, &event, cmp_pt->user_data);
    
    LITE_free(result_pt.URI);

    return SUCCESS_RETURN;
}


int iotx_cmp_cloud_conn_response_handler(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_process_response_pt msg)
{    
    iotx_cmp_response_info_t response = {0};
    
    if (NULL == cmp_pt || NULL == msg) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    log_info("new data.");
    
    memset(&response, 0x0, sizeof(iotx_cmp_response_info_t));  
    
    response.URI = msg->URI;
    response.payload = (char*)msg->payload;
    response.payload_length = msg->payload_length;
    
    if (cmp_pt->response_func)
        cmp_pt->response_func(cmp_pt, &response);  
    
    return SUCCESS_RETURN;
}


void* iotx_cmp_cloud_conn_init(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_init_param_pt pparam)
{
    void* handle = NULL;
    iotx_cloud_connection_param_t cloud_param = {0};
    
    cloud_param.device_info = HAL_Malloc(sizeof(iotx_deviceinfo_t));
    if (NULL == cloud_param.device_info) {
        log_info("memory error!");         
        return NULL;
    }

    strncpy(cloud_param.device_info->product_key, pparam->product_key, strlen(pparam->product_key));
    strncpy(cloud_param.device_info->device_name, pparam->device_name, strlen(pparam->device_name));
    strncpy(cloud_param.device_info->device_secret, pparam->device_secret, strlen(pparam->device_secret));
    strncpy(cloud_param.device_info->device_id, pparam->device_id, strlen(pparam->device_id));
    
    cloud_param.clean_session = 0;    
    cloud_param.keepalive_interval_ms = 60000;
    cloud_param.request_timeout_ms = 2000;

#ifdef CMP_VIA_CLOUD_CONN_MQTT
    cloud_param.protocol_type = IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_MQTT;
#else
#ifdef CMP_VIA_CLOUD_CONN_COAP
    cloud_param.protocol_type = IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_COAP;
#else
    cloud_param.protocol_type = IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_HTTP;
#endif
#endif

    cloud_param.event_handler = iotx_cmp_cloud_conn_event_callback;
    cloud_param.event_pcontext = cmp_pt;
 
    handle = IOT_Cloud_Connection_Init(&cloud_param);

    return handle;
}

int iotx_cmp_cloud_conn_register(iotx_cmp_conntext_pt cmp_pt, const char* topic_filter)
{
    int rc = 0;
    iotx_cloud_connection_msg_t msg = {0};

    if (NULL == cmp_pt || NULL == topic_filter) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    msg.type = IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_SUBSCRIBE;
    msg.QoS = IOTX_MESSAGE_QOS1;
    msg.URI = (char*)topic_filter;
    msg.URI_length = strlen(topic_filter);
    msg.payload = NULL;
    msg.payload_length = 0;
    msg.response_handler = iotx_cmp_cloud_conn_response_callback;
    msg.response_pcontext = cmp_pt;
    rc = IOT_Cloud_Connection_Send_Message(cmp_pt->cloud_connection, &msg);

    log_info("rc = %d", rc);
    
    return rc;
}


int iotx_cmp_cloud_conn_unregister(iotx_cmp_conntext_pt cmp_pt, const char* topic_filter)
{
    int rc = 0;
    iotx_cloud_connection_msg_t msg = {0};

    if (NULL == cmp_pt || NULL == topic_filter) {
        log_info("parameter error");
        return FAIL_RETURN;
    }    
    
    msg.type = IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_UNSUBSCRIBE;
    msg.QoS = IOTX_MESSAGE_QOS0;
    msg.URI = (char*)topic_filter;
    msg.URI_length = strlen(topic_filter);
    msg.payload = NULL;
    msg.payload_length = 0;
    msg.response_handler = iotx_cmp_cloud_conn_response_callback;
    msg.response_pcontext = cmp_pt;
    rc = IOT_Cloud_Connection_Send_Message(cmp_pt->cloud_connection, &msg);
    log_info("%d", rc);
    return rc;
}


int iotx_cmp_cloud_conn_send(iotx_cmp_conntext_pt cmp_pt, const char* topic_filter, const void* payload, int payload_length)
{
    int rc = 0;
    iotx_cloud_connection_msg_t msg = {0};
    
    if (NULL == cmp_pt || NULL == topic_filter || NULL == payload) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    msg.type = IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_PUBLISH;
    msg.QoS = IOTX_MESSAGE_QOS0;
    msg.URI = (char*)topic_filter;
    msg.URI_length = strlen(topic_filter);
    msg.payload = (char*)payload;
    msg.payload_length = payload_length;
    msg.response_handler = iotx_cmp_cloud_conn_response_callback;
    msg.response_pcontext = cmp_pt;
    rc = IOT_Cloud_Connection_Send_Message(cmp_pt->cloud_connection, &msg);

    return rc;
}


int iotx_cmp_cloud_conn_send_sync(iotx_cmp_conntext_pt cmp_pt, const char* topic_filter, const void* payload, int payload_length)
{
    return FAIL_RETURN;
}

int iotx_cmp_cloud_conn_yield(iotx_cmp_conntext_pt cmp_pt, int timeout_ms)
{
    if (NULL == cmp_pt) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    return IOT_Cloud_Connection_Yield(cmp_pt->cloud_connection, timeout_ms);
}

int iotx_cmp_cloud_conn_deinit(iotx_cmp_conntext_pt cmp_pt)
{    
    if (NULL == cmp_pt)
        return FAIL_RETURN; 
    
    return IOT_Cloud_Connection_Deinit(&(cmp_pt->cloud_connection));
}

#endif


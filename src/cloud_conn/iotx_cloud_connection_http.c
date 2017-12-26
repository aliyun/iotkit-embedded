#include "iot_import.h"
#include "lite-log.h"
#include "lite-utils.h"
#include "utils_timer.h"
#include "utils_list.h"
#include "device.h"
#include "utils_hmac.h"
#include "iot_export.h"

#include "iotx_cloud_connection_common.h"

#define IOTX_CLOUD_CLIENT_HTTP_RSP_LEN (1024)


void* iotx_cloud_connection_http_init(void *pparam, void* handle)
{
    void *http_pt = NULL;
    iotx_http_param_t http_param;
    iotx_cloud_connection_param_pt param = (iotx_cloud_connection_param_pt)pparam;
    
    memset(&http_param, 0, sizeof(http_param));
    
    http_param.device_info = param->device_info;
    http_param.timeout_ms = param->request_timeout_ms;
    http_param.keep_alive = !(param->clean_session);
    
    http_pt = IOT_HTTP_Init(&http_param);
    if (NULL == http_pt) {
        log_info("http init fail");
        return NULL;
    }
    
    if (IOT_HTTP_DeviceNameAuth(http_pt)) {
        log_info("http auth fail");
        return NULL;
    }    

    return http_pt;
}
                        
int iotx_cloud_connection_http_subscribe(void* handle,
                        const char *topic_filter,
                        iotx_message_qos_t qos)
{
    log_info("http not support subscribe");
    
    return SUCCESS_RETURN;
}
                        
int iotx_cloud_connection_http_unsubscribe(void* handle,
                        const char *topic_filter)
{
    log_info("http not support unsubscribe");
    
    return SUCCESS_RETURN;
}
                        
int iotx_cloud_connection_http_publish(void* handle,
                        iotx_cloud_connection_msg_pt message)
{
    iotx_http_message_param_t msg_param;
    iotx_cloud_connection_msg_rsp_t msg_rsp;
    int rc = 0;
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)handle;  
    
    if (FAIL_RETURN == iotx_cloud_connection_check_function_context(connection))
        return FAIL_RETURN;

    if (NULL == message) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    memset(&msg_param, 0x0, sizeof(iotx_http_message_param_t));
    msg_param.timeout_ms = connection->request_timeout_ms;
    msg_param.request_payload = (char *)message->payload;
    msg_param.request_payload_len = message->payload_length;
    msg_param.response_payload = LITE_malloc(IOTX_CLOUD_CLIENT_HTTP_RSP_LEN);
    if (NULL == msg_param.response_payload) {
        log_info("memory error");
        return FAIL_RETURN;
    }
    msg_param.response_payload_len = IOTX_CLOUD_CLIENT_HTTP_RSP_LEN;
    msg_param.topic_path = message->URI;

    rc = IOT_HTTP_SendMessage(connection->function_pt->pcontext, &msg_param);
    
    memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
    msg_rsp.URI = message->URI;
    msg_rsp.URI_length = message->URI_length;

    if (SUCCESS_RETURN == rc) {          
        log_info("http response success");
        
        msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_SEND_SUCCESS;
        msg_rsp.payload = msg_param.response_payload;
        msg_rsp.payload_length = msg_param.response_payload_len;
        
        if (message->response_handler) {
            message->response_handler(message->response_pcontext, connection, &msg_rsp);
        }
    } else {       
        log_info("http response fail");
    
        msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_SEND_FAIL;
        msg_rsp.payload = NULL;
        msg_rsp.payload_length = 0;
        
        if (message->response_handler)
            message->response_handler(message->response_pcontext, connection, &msg_rsp);       
    }    

    LITE_free(msg_param.response_payload);        
    return 1;
}
                        
int iotx_cloud_connection_http_deinit(void* handle)
{
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)handle;  
    
    if (FAIL_RETURN == iotx_cloud_connection_check_function_context(connection))
        return FAIL_RETURN;


    IOT_HTTP_DeInit(&(connection->function_pt->pcontext));

    return SUCCESS_RETURN;
}
                        
int iotx_cloud_connection_http_yield(void* handle, int timeout_ms)
{
    log_info("http not need yield");
    
    return SUCCESS_RETURN;
}
                        


#include "iot_import.h"
#include "lite-log.h"
#include "lite-utils.h"
#include "utils_timer.h"
#include "utils_list.h"
#include "lite-system.h"
#include "utils_hmac.h"
#include "iot_export.h"
#include "iotx_cloud_connection_common.h"


#define IOTX_ONLINE_DTLS_SERVER_URL     "coaps://%s.iot-as-coap.cn-shanghai.aliyuncs.com:5684"

typedef struct iotx_cloud_connection_coap_st {
    iotx_cloud_connection_pt                    connection;  
    char*                                       URI;
} iotx_cloud_connection_coap_t, *iotx_cloud_connection_coap_pt;


static void iotx_cloud_connection_coap_event_handle(void *context, iotx_coap_event_t event, void *p_data)
{    
    log_info("coap event handle %d", event);
    
    switch(event) {
        case IOTX_COAP_EVENT_AUTH_FAILED:
            break;
        
        case IOTX_COAP_EVENT_RECV_FAILED:{
        }
            break;
        
        case IOTX_COAP_EVENT_SEND_FAILED:{
        }
            break;

        default:
            log_info("event type error");
            break;
    }

}

static void iotx_cloud_connection_coap_response_handler(void *arg, void *p_response)
{
    int len = 0;
    char *payload = NULL;
    iotx_coap_resp_code_t resp_code;
    iotx_cloud_connection_coap_pt coap_pt = (iotx_cloud_connection_coap_pt)arg;

    log_info("coap response");

    if (NULL == coap_pt) {
        log_info("parameter error");
        return;
    }
    
    if (NULL == coap_pt->connection || NULL == coap_pt->URI) {
        log_info("coap_pt error");
        return;
    } 
    
    IOT_CoAP_GetMessageCode(p_response, &resp_code);
    log_info("resp_code: %d", resp_code);
    IOT_CoAP_GetMessagePayload(p_response, (unsigned char **)&payload, &len);
    log_info("payload: %d, %s", len, payload);

    if (IOTX_COAP_RESP_CODE_CONTENT == resp_code) {        
        iotx_cloud_connection_msg_rsp_t msg_rsp;
    
        log_info("coap response success");
    
        memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
        msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_SEND_SUCCESS;
        log_info("coap response success");
        msg_rsp.URI = coap_pt->URI;
        log_info("URI: %s", coap_pt->URI);
        msg_rsp.URI_length = strlen(msg_rsp.URI);
        log_info("URI_length: %d", msg_rsp.URI_length);
        msg_rsp.payload = payload;
        msg_rsp.payload_length = len;
        
        log_info("coap response success");
        
        if (coap_pt->connection->resposne_handler)
            coap_pt->connection->resposne_handler(coap_pt->connection, &msg_rsp);

        LITE_free(coap_pt->URI);        
        LITE_free(coap_pt);
        coap_pt = NULL;
    } else {        
        iotx_cloud_connection_msg_rsp_t msg_rsp;
    
        log_info("coap response fail");
    
        memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
        msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_SEND_FAIL;
        msg_rsp.URI = coap_pt->URI;
        msg_rsp.URI_length = strlen(coap_pt->URI);
        msg_rsp.payload = payload;
        msg_rsp.payload_length = strlen(payload);
        
        if (coap_pt->connection->resposne_handler)
            coap_pt->connection->resposne_handler(coap_pt->connection, &msg_rsp);

        LITE_free(coap_pt->URI);        
        LITE_free(coap_pt);    
        coap_pt = NULL;    
    }
}


void* iotx_cloud_connection_coap_init(void *pparam, void* handle)
{
    iotx_coap_config_t      config;
    iotx_cloud_connection_param_pt param = (iotx_cloud_connection_param_pt)pparam;
    char url[256] = {0};
    iotx_coap_context_t *p_ctx = NULL;

    log_info("init");

    /* set config */
    memset(&config, 0x00, sizeof(iotx_coap_config_t));
    HAL_Snprintf(url, sizeof(url), IOTX_ONLINE_DTLS_SERVER_URL, (char *)param->device_info->product_key);
    config.p_url = url;

    config.p_devinfo = param->device_info;
    config.wait_time_ms = param->request_timeout_ms;
    config.event_handle = iotx_cloud_connection_coap_event_handle;    

    log_info("init");
    /* initial */
    p_ctx = IOT_CoAP_Init(&config);
    if (NULL == p_ctx) {
        log_info("coap init fail");
        return NULL;
    }

    log_info("init");

    /* auth */
    if (IOT_CoAP_DeviceNameAuth(p_ctx)){
        log_info("coap auth fail");
        IOT_CoAP_Deinit(&p_ctx);
        return NULL;
    }

    log_info("init");
    
    return p_ctx;
}

int iotx_cloud_connection_coap_subscribe(void* handle,
                        const char *topic_filter,
                        iotx_message_qos_t qos)
{
    log_info("coap not support subscribe");
    return SUCCESS_RETURN;
}
                   
int iotx_cloud_connection_coap_unsubscribe(void* handle,
                        const char *topic_filter)
{
    log_info("coap not support unsubscribe");
    return SUCCESS_RETURN;
}
                        
int iotx_cloud_connection_coap_publish(void* handle,
                        iotx_cloud_connection_msg_pt message)
{    
    iotx_message_t coap_message;
    iotx_cloud_connection_coap_pt coap_pt = NULL; 
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)handle;  
    
    
    if (FAIL_RETURN == iotx_cloud_connection_check_function_context(connection))
        return FAIL_RETURN;

    if (NULL == message) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
        
    coap_pt = LITE_malloc(sizeof(iotx_cloud_connection_coap_t));
    if (NULL == coap_pt) {
        log_info("memory error");
        return FAIL_RETURN;
    }
    memset(coap_pt, 0x0, sizeof(iotx_cloud_connection_coap_t));
    
    coap_pt->URI = LITE_malloc(message->URI_length + 1);
    if (NULL == coap_pt) {
        log_info("memory error");
        LITE_free(coap_pt);
        return FAIL_RETURN;
    }
    memset(coap_pt->URI, 0x0, message->URI_length + 1);
    strncpy(coap_pt->URI, message->URI, message->URI_length);
    log_info("send message %s", coap_pt->URI);
    coap_pt->connection = connection;
    
    coap_message.p_payload = (unsigned char*)message->payload;
    coap_message.payload_len = message->payload_length;
    coap_message.msg_type = message->message_type;
    coap_message.content_type = message->content_type;
    coap_message.user_data = (void*)coap_pt;
    coap_message.resp_callback = iotx_cloud_connection_coap_response_handler;

    return IOT_CoAP_SendMessage(connection->function_pt->pcontext, message->URI, &coap_message);
}
                        
int iotx_cloud_connection_coap_deinit(void* handle)
{
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)handle;  
    
    if (FAIL_RETURN == iotx_cloud_connection_check_function_context(connection))
        return FAIL_RETURN;

    IOT_CoAP_Deinit(&(connection->function_pt->pcontext));

    return  SUCCESS_RETURN;
}
                        
int iotx_cloud_connection_coap_yield(void* handle, int timeout_ms)
{
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)handle;  
    
    if (FAIL_RETURN == iotx_cloud_connection_check_function_context(connection))
        return FAIL_RETURN;

    return IOT_CoAP_Yield(connection->function_pt->pcontext);
}
                        


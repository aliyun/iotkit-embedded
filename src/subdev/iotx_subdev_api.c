#include "iot_import.h"

#include "utils_list.h"
#include "lite-utils.h"
#include "device.h"
#include "iotx_subdev_common.h"

iotx_gateway_pt g_gateway_subdevice_t = NULL;

static void iotx_mqtt_reconnect_callback(iotx_gateway_pt gateway);

static int iotx_gateway_recv_publish_callbacks(iotx_gateway_pt gateway, 
        char* recv_topic,
        char* recv_payload);

static int iotx_subdevice_recv_rrpc_callback(iotx_gateway_pt gateway,         
        char* recv_topic,
        char* recv_payload);


#ifdef SUBDEV_VIA_CLOUD_CONN
static void _event_handle(void *pcontext, iotx_cloud_connection_event_msg_pt msg)
{
    log_info("_event_handle type %d", msg->event_id);

    if (msg->event_id == IOTX_CLOUD_CONNECTION_EVENT_RECONNECT) {
        iotx_mqtt_reconnect_callback(pcontext);        
    }
}

void _response_handle(void *pcontext, void *pconnection, iotx_cloud_connection_msg_rsp_pt msg)
{
    iotx_gateway_pt gateway = (iotx_gateway_pt)pcontext; 

    log_info("response %d", msg->rsp_type);
    
    switch(msg->rsp_type) {
        case IOTX_CLOUD_CONNECTION_RESPONSE_SUBSCRIBE_SUCCESS: 
        case IOTX_CLOUD_CONNECTION_RESPONSE_UNSUBSCRIBE_SUCCESS:
        #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
            HAL_MutexLock(gateway->gateway_data.lock_sync);
        #endif
            gateway->gateway_data.sync_status = 0;
        #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
            HAL_MutexUnlock(gateway->gateway_data.lock_sync);
        #endif
            break;
        
        case IOTX_CLOUD_CONNECTION_RESPONSE_SUBSCRIBE_FAIL:
        case IOTX_CLOUD_CONNECTION_RESPONSE_UNSUBSCRIBE_FAIL:
        #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
            HAL_MutexLock(gateway->gateway_data.lock_sync);
        #endif
            gateway->gateway_data.sync_status = -1;
        #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
            HAL_MutexUnlock(gateway->gateway_data.lock_sync);
        #endif
            break;
        
        case IOTX_CLOUD_CONNECTION_RESPONSE_SEND_SUCCESS: 
            break;
            
        case IOTX_CLOUD_CONNECTION_RESPONSE_SEND_FAIL:
            break;
            
        case IOTX_CLOUD_CONNECTION_RESPONSE_NEW_DATA:{
            char* publish_topic = NULL;
            char* publish_payload = NULL;

            /* printf payload */
            log_info("topic message arrived : topic [%.*s]\n",
                     msg->URI_length,
                     msg->URI); 

            log_info("payload length [%d]", msg->payload_length);

            MALLOC_MEMORY(publish_topic, msg->payload_length + 1);
            MALLOC_MEMORY_WITH_FREE(publish_payload, msg->payload_length + 1, publish_topic);

            strncpy(publish_topic, msg->URI, msg->URI_length);
            strncpy(publish_payload, msg->payload, msg->payload_length);
            

            if (SUCCESS_RETURN == iotx_gateway_recv_publish_callbacks(gateway, publish_topic, publish_payload)
                || SUCCESS_RETURN == iotx_subdevice_recv_rrpc_callback(gateway, publish_topic, publish_payload)) {
                /*LITE_free(publish_topic);
                LITE_free(publish_payload);*/
                return;
            }           
            
            LITE_free(publish_topic);
            LITE_free(publish_payload);
        }
            
            break;
        default:
            break;
    }
}

#endif


static void iotx_subdevice_common_reply_proc(iotx_gateway_pt gateway, 
        char* payload,
        int is_login)
{
    char* node = NULL;
#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
    void* lock = NULL;
#endif
    iotx_common_reply_data_pt reply_data = NULL;    

    log_info("recv reply");

    if (gateway == NULL || payload == NULL) {
        log_info("param error");
        return;
    }
    
    if (is_login) {
    #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
        lock = gateway->gateway_data.lock_login;
    #endif
        reply_data = &gateway->gateway_data.login_reply;
    } else {
    #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
        lock = gateway->gateway_data.lock_logout;
    #endif
        reply_data = &gateway->gateway_data.logout_reply;
    }

    /* parse result */
    /* parse   id */
    node = LITE_json_value_of("id", payload);
    if (node == NULL) { 
        log_err("get id of json error!");
        return;
    }
    
#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
    HAL_MutexLock(lock);
#endif
    if (reply_data->id == atoi(node)) {
        reply_data->id = 0;
    }
#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
    HAL_MutexUnlock(lock);
#endif

    LITE_free(node);
    node = NULL;
    /* parse   code */
    node = LITE_json_value_of("code", payload);
    if (node == NULL) {
        log_err("get code of json error!");
        return;
    }
    
#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
    HAL_MutexLock(lock);
#endif
    reply_data->code = atoi(node);
#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
    HAL_MutexUnlock(lock);
#endif
    LITE_free(node);
}

static int iotx_parse_rrpc_message_id(const char* topic, char* message, uint32_t message_len)
{
    char* temp;
    temp = strstr(topic, "/rrpc/request/");        
    if (temp == NULL) {
        log_err("parse error");
        return FAIL_RETURN;
    }

    temp = temp + strlen("/rrpc/request/");

    if (strlen(temp) + 1 > message_len) {
        log_info("message len error");
        return FAIL_RETURN;
    }

    strncpy(message, temp, strlen(temp));

    log_info("message %s", message);

    return SUCCESS_RETURN;
}


/*recv subdev publish message proc*/ 
static int iotx_subdevice_recv_rrpc_callback(iotx_gateway_pt gateway,         
        char* recv_topic,
        char* recv_payload)
{    
    char topic[GATEWAY_TOPIC_LEN_MAX] = {0}; 
    iotx_subdevice_session_pt session = NULL;
    
    if (gateway == NULL || recv_topic == NULL || recv_payload == NULL) {
        log_info("param error");
        return FAIL_RETURN;
    }

    session = gateway->session_list;

    while (session) {        
        memset(topic, 0x0, GATEWAY_TOPIC_LEN_MAX);
        HAL_Snprintf(topic, GATEWAY_TOPIC_LEN_MAX, TOPIC_SYS_RRPC_FMT, session->product_key, 
                session->device_name, "request");
        if ((strlen(recv_topic) >= strlen(topic) - 2) && 
          (0 == strncmp(recv_topic, topic, strlen(topic) - 2))) {
            /* subdev rrpc */
            log_info("session rrpc callback");
            
            if (session->rrpc_callback) {
                char message_id[200] = {0};
                if (SUCCESS_RETURN == iotx_parse_rrpc_message_id(recv_topic, message_id, 20)) {
                    session->rrpc_callback((void*)gateway, 
                                    session->product_key,
                                    session->device_name,
                                    message_id, 
                                    recv_payload);
                }
            }
            else
                log_info("recv rrpc request, but not register callback");
            return SUCCESS_RETURN;
        }
        session = session->next;
    }

    return FAIL_RETURN;
}

/*recv gateway publish message proc*/ 
static int iotx_gateway_recv_publish_callbacks(iotx_gateway_pt gateway, 
        char* recv_topic,
        char* recv_payload)
{
    char topic[GATEWAY_TOPIC_LEN_MAX] = {0}; 
    iotx_device_info_pt pdevice_info = NULL;
    
    if (gateway == NULL || recv_topic == NULL || recv_payload == NULL) {
        log_info("param error");
        return FAIL_RETURN;
    }
    
    pdevice_info = iotx_device_info_get();

    /* login_reply */
    HAL_Snprintf(topic,
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SESSION_COMBINE_FMT, 
            pdevice_info->product_key, 
            pdevice_info->device_name, 
            "login_reply");
    if ((strlen(recv_topic) == strlen(topic)) && 
      (0 == strncmp(recv_topic, topic, strlen(topic)))) {
        iotx_subdevice_common_reply_proc(gateway, recv_payload, 1);
        return SUCCESS_RETURN;
    }

    /* logout_reply */
    HAL_Snprintf(topic,
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SESSION_COMBINE_FMT, 
            pdevice_info->product_key, 
            pdevice_info->device_name, 
            "logout_reply");
    if ((strlen(recv_topic) == strlen(topic)) && 
      (0 == strncmp(recv_topic, topic, strlen(topic)))) {
        iotx_subdevice_common_reply_proc(gateway, recv_payload, 0);
        return SUCCESS_RETURN;
    }

    /* rrpc request */
    memset(topic, 0x0, GATEWAY_TOPIC_LEN_MAX);
    HAL_Snprintf(topic, 
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SYS_RRPC_FMT, 
            pdevice_info->product_key,
            pdevice_info->device_name, 
            "request");
    /* note: rrpc topic      /+ */
    if ((strlen(recv_topic) >= strlen(topic) - 2) && 
      (0 == strncmp(recv_topic, topic, strlen(topic) - 2))) {   
        log_info("gateway rrpc callback");        
        
        if (gateway->gateway_data.rrpc_callback) {
            char message_id[200] = {0};
            if (SUCCESS_RETURN == iotx_parse_rrpc_message_id(recv_topic, message_id, 20)) {
                gateway->gateway_data.rrpc_callback((void*)gateway, 
                                            pdevice_info->product_key, 
                                            pdevice_info->device_name, 
                                            message_id, 
                                            recv_payload);        
            }
        }
        else
            log_info("there is no gateway rrpc callback, please call IOT_RRPC_Register with gateway's device_cloud_id first");
        
        return SUCCESS_RETURN;
    }     
      
    return FAIL_RETURN;
}

        
/* login all subdev */
static void iotx_mqtt_reconnect_callback(iotx_gateway_pt gateway)
{    
    iotx_subdevice_session_pt session = NULL;

    if (NULL == gateway) {
        log_info("param error");
        return;
    }   
    
    log_info("iotx_mqtt_reconnect_callback"); 

    session = gateway->session_list;

    while (session) {
    #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
        HAL_MutexLock(session->lock_status);
    #endif
        session->session_status = IOTX_SUBDEVICE_SEESION_STATUS_INIT;
    #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
        HAL_MutexUnlock(session->lock_status);
    #endif
        if (FAIL_RETURN == IOT_Subdevice_Login(gateway, 
                                session->product_key, 
                                session->device_name, 
                                session->timestamp,
                                session->client_id,
                                session->sign,
                                session->sign_method,
                                session->clean_session)) {
            log_info("reconnect, %s re_login error", session->device_cloud_id);
        }
        session = session->next;
    }
}

#ifndef SUBDEV_VIA_CLOUD_CONN
void iotx_gateway_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_gateway_pt gateway = (iotx_gateway_pt)pcontext;  
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)(msg->msg);
    
    if (gateway == NULL) { 
       log_info("param error"); 
       return; 
    }
    
    log_info("event type %d", msg->event_type);

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
        #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
            HAL_MutexLock(gateway->gateway_data.lock_sync);
        #endif
            if (gateway->gateway_data.sync_status == packet_id) {
                gateway->gateway_data.sync_status = 0;
            #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
                HAL_MutexUnlock(gateway->gateway_data.lock_sync);
            #endif
                return;
            }
        #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
            HAL_MutexUnlock(gateway->gateway_data.lock_sync);
        #endif
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
        #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
            HAL_MutexLock(gateway->gateway_data.lock_sync);
        #endif
            if (gateway->gateway_data.sync_status == packet_id) {
                gateway->gateway_data.sync_status = -1;
            #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
                HAL_MutexUnlock(gateway->gateway_data.lock_sync);
            #endif
                return;
            }        
        #ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
            HAL_MutexUnlock(gateway->gateway_data.lock_sync);
        #endif
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED: {
            char* publish_topic = NULL;
            char* publish_payload = NULL;
            int printf_num = 0;
            char dsltemplate_printf[512] = {0};

            /* printf payload */
            log_info("topic message arrived : topic [%.*s]\n",
                     topic_info->topic_len,
                     topic_info->ptopic); 

            log_info("payload length [%d]", topic_info->payload_len);
            log_info("payload:");
            while(printf_num < topic_info->payload_len) {
                memset(dsltemplate_printf, 0x0, 512);
                if (topic_info->payload_len - printf_num > 500) {
                    strncpy(dsltemplate_printf, &topic_info->payload[printf_num], 500);      
                    printf_num += 500;            
                } else if (topic_info->payload_len - printf_num <= 500){
                    strncpy(dsltemplate_printf, &topic_info->payload[printf_num], topic_info->payload_len - printf_num);  
                    printf_num = topic_info->payload_len;
                }        
                log_info("%s", dsltemplate_printf);
            }

            MALLOC_MEMORY(publish_topic, topic_info->topic_len + 1);
            MALLOC_MEMORY_WITH_FREE(publish_payload, topic_info->payload_len + 1, publish_topic);

            strncpy(publish_topic, topic_info->ptopic, topic_info->topic_len);
            strncpy(publish_payload, topic_info->payload, topic_info->payload_len);

            if (SUCCESS_RETURN == iotx_gateway_recv_publish_callbacks(gateway, publish_topic, publish_payload)
                || SUCCESS_RETURN == iotx_subdevice_recv_rrpc_callback(gateway, publish_topic, publish_payload)) {
                LITE_free(publish_topic);
                LITE_free(publish_payload);
                return;
            }           
            
            LITE_free(publish_topic);
            LITE_free(publish_payload);
        }
            break;

        case IOTX_MQTT_EVENT_RECONNECT:         
            iotx_mqtt_reconnect_callback(pcontext);
            return;
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            return;
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            log_info("publish fail");
            return;
            break;

        default:
            return;
            break;
    }

    if (gateway->event_handler)
        gateway->event_handler(gateway->event_pcontext, pclient, msg);

    return;
}
#endif

/* global message id */
uint32_t global_gateway_cloud_id = 0;

uint32_t IOT_Gateway_Generate_Message_ID()
{
    global_gateway_cloud_id += 1;

    return ((global_gateway_cloud_id == 0 )? global_gateway_cloud_id += 1 : global_gateway_cloud_id);
}


void* IOT_Gateway_Construct(iotx_gateway_param_pt gateway_param)
{
    iotx_gateway_pt gateway = NULL;  
    
#ifdef SUBDEV_VIA_CLOUD_CONN   
    void* handle = NULL;
    iotx_cloud_connection_param_t param = {0};
    iotx_device_info_pt pdevice_info = iotx_device_info_get();
#endif
    
    PARAMETER_NULL_CHECK_WITH_RESULT(gateway_param, NULL);
    PARAMETER_NULL_CHECK_WITH_RESULT(gateway_param->mqtt, NULL);

    if (g_gateway_subdevice_t != NULL) {
        log_err("gateway have been construct");
        return NULL;
    }

    MALLOC_MEMORY_WITH_RESULT(gateway, sizeof(iotx_gateway_t), NULL);
    
#ifndef SUBDEV_VIA_CLOUD_CONN       
    gateway_param->mqtt->handle_event.h_fp = iotx_gateway_event_handle;
    gateway_param->mqtt->handle_event.pcontext = gateway;

    /* construct MQTT client */
    if (NULL == (gateway->mqtt = IOT_MQTT_Construct(gateway_param->mqtt))) {
        log_err("construct MQTT failed");
        LITE_free(gateway);
        return NULL;
    }
#else /* SUBDEV_VIA_CLOUD_CONN */      
    param.device_info = HAL_Malloc(sizeof(iotx_deviceinfo_t));
    if (NULL == param.device_info) {
        log_info("memory error!");   
        LITE_free(gateway);      
        return NULL;
    }
    memset(param.device_info, 0x00, sizeof(iotx_device_info_t));
    strncpy(param.device_info->device_id,    pdevice_info->device_id,   IOTX_DEVICE_ID_LEN);
    strncpy(param.device_info->product_key,  pdevice_info->product_key, IOTX_PRODUCT_KEY_LEN);
    strncpy(param.device_info->device_secret, pdevice_info->device_secret, IOTX_DEVICE_SECRET_LEN);
    strncpy(param.device_info->device_name,  pdevice_info->device_name, IOTX_DEVICE_NAME_LEN);
    
    param.clean_session = gateway_param->mqtt->clean_session;    
    param.keepalive_interval_ms = gateway_param->mqtt->keepalive_interval_ms;
    param.request_timeout_ms = gateway_param->mqtt->request_timeout_ms;
    param.protocol_type = IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_MQTT;
    param.event_handler = _event_handle;
    param.event_pcontext = (void*)gateway;

    handle = IOT_Cloud_Connection_Init(&param);

    if (handle == NULL) {
        LITE_free(gateway);      
        return NULL;
    }

    gateway->mqtt = handle;
#endif /* SUBDEV_VIA_CLOUD_CONN */

#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
    gateway->gateway_data.lock_sync = HAL_MutexCreate();
    gateway->gateway_data.lock_sync_enter = HAL_MutexCreate();
    gateway->gateway_data.lock_login = HAL_MutexCreate();
    gateway->gateway_data.lock_login_enter = HAL_MutexCreate();
    if (NULL == gateway->gateway_data.lock_sync || 
        NULL == gateway->gateway_data.lock_sync_enter || 
        NULL == gateway->gateway_data.lock_login ||
        NULL == gateway->gateway_data.lock_login_enter)
    {
        log_err("create mutex error");
        LITE_free(gateway);
        return NULL;
    }
#endif
    
    /* handle mqtt event for user */
    gateway->event_handler = gateway_param->event_handler;
    gateway->event_pcontext = gateway_param->event_pcontext;

    g_gateway_subdevice_t = gateway;

    /* subscribe default topic */
    if (FAIL_RETURN == iotx_gateway_subscribe_unsubscribe_default(gateway, 1)) {
        log_err("subscribe default topic fialed");
        IOT_Gateway_Destroy((void**)&gateway); 
        return NULL;
    }

    return gateway;
}


int IOT_Subdevice_Login(void* handle, 
        const char* product_key, 
        const char* device_name, 
        const char* timestamp, 
        const char* client_id, 
        const char* sign, 
        iotx_login_sign_method_types_t sign_method_type,
        iotx_login_clean_session_types_t clean_session_type)
{
    uint32_t msg_id = 0;
    char * login_packet = NULL;
    char topic[GATEWAY_TOPIC_LEN_MAX] = {0}; 
    char sign_method[10] = {0};
    char clean_session[10] = {0};
    iotx_subdevice_session_pt session = NULL;
    iotx_device_info_pt pdevice_info = iotx_device_info_get();
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;

    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(product_key, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(device_name, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(timestamp, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(client_id, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(sign, FAIL_RETURN);

    /* check sign method */
    log_info("sign_method_type %d", sign_method_type);
    if (sign_method_type != IOTX_LOGIN_SIGN_METHOD_TYPE_SHA && 
            sign_method_type != IOTX_LOGIN_SIGN_METHOD_TYPE_MD5) {
        log_info("sign method not support");
        return FAIL_RETURN;
    }
            
    if (clean_session_type != IOTX_LOGIN_CLEAN_SESSION_TRUE && 
            clean_session_type != IOTX_LOGIN_CLEAN_SESSION_FALSE) {
        log_info("clean session not support");
        return FAIL_RETURN;
    }

    if (NULL == (session = iotx_subdevice_find_session(gateway,
                                product_key, 
                                device_name))) {
        log_info("there is no seesion, create a new session");
        /* create subdev session */
        if (NULL == (session = iotx_subdevice_add_session(gateway,
                                    product_key, 
                                    device_name, 
                                    NULL, 
                                    sign,
                                    timestamp,
                                    client_id,
                                    sign_method_type,
                                    clean_session_type))) {
            log_err("create session error!");
            return FAIL_RETURN;        
        }    
    } else {
        log_info("device have been login");
        return FAIL_RETURN;
    }

    /* topic */
    HAL_Snprintf(topic, 
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SESSION_COMBINE_FMT, 
            pdevice_info->product_key, 
            pdevice_info->device_name, 
            "login");

    if (sign_method_type == IOTX_LOGIN_SIGN_METHOD_TYPE_SHA) {
        strncpy(sign_method, "hmacsha1", strlen("hmacsha1"));
    } else {
        strncpy(sign_method, "hmacmd5", strlen("hmacmd5"));
    } 

    if (clean_session_type == IOTX_LOGIN_CLEAN_SESSION_TRUE) {
        strncpy(clean_session, "true", strlen("true"));
    } else {
        strncpy(clean_session, "false", strlen("false"));
    }
    
    /* packet */    
    login_packet = iotx_gateway_splice_login_packet(session->product_key,
                        session->device_name,
                        client_id, 
                        timestamp, 
                        sign_method,
                        sign,      
                        clean_session,
                        &msg_id);
            
    
    if (login_packet == NULL) {
        log_err("login packet splice error!");
        iotx_subdevice_remove_session(gateway, session->product_key, session->device_name);
        return FAIL_RETURN;
    }

    /* publish packet */
    if (FAIL_RETURN == iotx_gateway_publish_sync(gateway, 
            IOTX_MQTT_QOS0, 
            topic, 
            login_packet, 
            msg_id, 
            &(gateway->gateway_data.login_reply),
            IOTX_GATEWAY_PUBLISH_LOGIN)) {
        LITE_free(login_packet);
        iotx_subdevice_remove_session(gateway, session->product_key, session->device_name);
        log_err("MQTT Publish error!");
        return FAIL_RETURN;        
    }
            
    LITE_free(login_packet);   
                   
#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
    HAL_MutexLock(session->lock_status);
#endif
    session->session_status = IOTX_SUBDEVICE_SEESION_STATUS_LOGIN;
#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
    HAL_MutexUnlock(session->lock_status);
#endif

    /* subscribe rrpc request */
    if (FAIL_RETURN == iotx_gateway_subscribe_unsubscribe_topic(gateway,
            session->product_key, 
            session->device_name,
            TOPIC_SYS_RRPC_FMT,  
            "request",
            1)) {
        iotx_subdevice_remove_session(gateway, session->product_key, session->device_name);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}


int IOT_Subdevice_Logout(void* handle, 
        const char * product_key, 
        const char * device_name)
{
    uint32_t msg_id = 0;
    char topic[GATEWAY_TOPIC_LEN_MAX] = {0}; 
    iotx_subdevice_session_pt session = NULL;
    char* logout_packet = NULL;
    iotx_device_info_pt pdevice_info = iotx_device_info_get();
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(product_key, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(device_name, FAIL_RETURN);
    
    session = iotx_subdevice_find_session(gateway, product_key, device_name);
    if (NULL == session) {
        log_info("no session, can not logout");
        return FAIL_RETURN;
    }
    
    /* unsubscribe rrpc request */
    if (FAIL_RETURN == iotx_gateway_subscribe_unsubscribe_topic(gateway,
            session->product_key, 
            session->device_name,
            TOPIC_SYS_RRPC_FMT,  
            "request",
            0)){
        return FAIL_RETURN;
    }            

    /* topic */
    HAL_Snprintf(topic, 
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SESSION_COMBINE_FMT, 
            pdevice_info->product_key, 
            pdevice_info->device_name, 
            "logout");

    /* splice logout packet */
    logout_packet = iotx_gateway_splice_logout_packet(session->product_key, 
                            session->device_name, 
                            &msg_id);
    if (logout_packet == NULL) {
        log_err("logout packet splice error!");
        return FAIL_RETURN;
    }

    /* publish logout packet */
    iotx_mqtt_topic_info_t topic_msg;
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));

    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)logout_packet;
    topic_msg.payload_len = strlen(logout_packet);
    topic_msg.packet_id = 0;

    if (FAIL_RETURN == iotx_gateway_publish_sync(gateway, 
            IOTX_MQTT_QOS0, 
            topic, 
            logout_packet, 
            msg_id, 
            &(gateway->gateway_data.logout_reply),
            IOTX_GATEWAY_PUBLISH_LOGIN)) {
        LITE_free(logout_packet);
        log_err("MQTT Publish error!");
        return FAIL_RETURN;        
    }

    LITE_free(logout_packet); 
    
#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
    HAL_MutexLock(session->lock_status);
#endif
    session->session_status = IOTX_SUBDEVICE_SEESION_STATUS_LOGOUT;
#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
    HAL_MutexUnlock(session->lock_status);
#endif

    /* free session */
    iotx_subdevice_remove_session(gateway, session->product_key, session->device_name);

    return SUCCESS_RETURN;
}

int IOT_Gateway_Destroy(void** handle)
{
    iotx_subdevice_session_pt session, pre_session;
    iotx_gateway_pt gateway = NULL;

    PARAMETER_NULL_CHECK_WITH_RESULT(handle, FAIL_RETURN);
    
    gateway  = (iotx_gateway_pt)(*handle);
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    
    /* free session list */
    pre_session = session = gateway->session_list;

    while (session) {
        pre_session = session;
        session = session->next;    
        IOT_Subdevice_Logout(gateway, pre_session->product_key, pre_session->device_name);
    }
    
    /* send unsubscribe packet */
    iotx_gateway_subscribe_unsubscribe_default(gateway, 0);

    /* MQTT disconnect*/
    IOT_MQTT_Destroy(&gateway->mqtt); 
    
#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD  
    HAL_MutexDestroy(gateway->gateway_data.lock_login);
    HAL_MutexDestroy(gateway->gateway_data.lock_login_enter);
    HAL_MutexDestroy(gateway->gateway_data.lock_sync);
    HAL_MutexDestroy(gateway->gateway_data.lock_sync_enter);
#endif
    
    LITE_free(gateway);
    *handle = NULL;
    g_gateway_subdevice_t = NULL;
    
    return SUCCESS_RETURN;
}


int IOT_Gateway_Yield(void* handle, uint32_t timeout)
{
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);

#ifdef SUBDEV_VIA_CLOUD_CONN
    return IOT_Cloud_Connection_Yield(gateway->mqtt, timeout);
#else    
    return IOT_MQTT_Yield(gateway->mqtt, timeout);
#endif
}

int IOT_Gateway_Subscribe(void* handle, 
        const char *topic_filter, 
        int qos, 
        iotx_subdev_event_handle_func_fpt topic_handle_func, 
        void *pcontext)
{
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
#ifdef SUBDEV_VIA_CLOUD_CONN
    iotx_cloud_connection_msg_t msg = {0};
    msg.type = IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_SUBSCRIBE;
    msg.QoS = IOTX_MESSAGE_QOS1;
    msg.URI = (char*)topic_filter;
    msg.URI_length = strlen(topic_filter);
    msg.payload = NULL;
    msg.payload_length = 0;
    msg.response_handler = (iotx_cloud_connection_msg_rsp_func_fpt)topic_handle_func;
    msg.response_pcontext = NULL;
    msg.content_type = IOTX_MESSAGE_CONTENT_TYPE_JSON;
    msg.message_type = IOTX_MESSAGE_CONFIRMABLE;
    return IOT_Cloud_Connection_Send_Message(gateway->mqtt, &msg);
#else    
    return IOT_MQTT_Subscribe(gateway->mqtt, topic_filter, qos, 
            (iotx_mqtt_event_handle_func_fpt)topic_handle_func, pcontext);
#endif    
}


int IOT_Gateway_Unsubscribe(void* handle, 
        const char *topic_filter)
{
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
#ifdef SUBDEV_VIA_CLOUD_CONN
    iotx_cloud_connection_msg_t msg = {0};
    msg.type = IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_UNSUBSCRIBE;
    msg.QoS = IOTX_MESSAGE_QOS1;
    msg.URI = (char*)topic_filter;
    msg.URI_length = strlen(topic_filter);
    msg.payload = NULL;
    msg.payload_length = 0;
    msg.content_type = IOTX_MESSAGE_CONTENT_TYPE_JSON;
    msg.message_type = IOTX_MESSAGE_CONFIRMABLE;
    return IOT_Cloud_Connection_Send_Message(gateway->mqtt, &msg);
#else        
    return IOT_MQTT_Unsubscribe(gateway->mqtt, topic_filter);
#endif    
}


int IOT_Gateway_Publish(void*        handle, 
        const char *topic_name, 
        iotx_mqtt_topic_info_pt topic_msg)
{
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    int printf_num = 0;
    char dsltemplate_printf[512] = {0};
    
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    PARAMETER_NULL_CHECK_WITH_RESULT(topic_msg, FAIL_RETURN);

    log_info("Publish topic_name [%s]", topic_name);
    log_info("Publish payload length [%d]", topic_msg->payload_len);
    log_info("payload:");
    while(printf_num < topic_msg->payload_len) {
        memset(dsltemplate_printf, 0x0, 512);
        if (topic_msg->payload_len - printf_num > 500) {
            strncpy(dsltemplate_printf, &topic_msg->payload[printf_num], 500);      
            printf_num += 500;            
        } else if (topic_msg->payload_len - printf_num <= 500){
            strncpy(dsltemplate_printf, &topic_msg->payload[printf_num], topic_msg->payload_len - printf_num);  
            printf_num = topic_msg->payload_len;
        }        
        log_info("%s", dsltemplate_printf);
    }
    
#ifdef SUBDEV_VIA_CLOUD_CONN
    iotx_cloud_connection_msg_t msg = {0};
    msg.type = IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_PUBLISH;
    msg.QoS = topic_msg->qos;
    msg.URI = (char*)topic_name;
    msg.URI_length = strlen(topic_name);
    msg.payload = (void*)topic_msg->payload;
    msg.payload_length = topic_msg->payload_len;
    msg.content_type = IOTX_MESSAGE_CONTENT_TYPE_JSON;
    msg.message_type = IOTX_MESSAGE_CONFIRMABLE;
    return IOT_Cloud_Connection_Send_Message(gateway->mqtt, &msg);
#else        
    return IOT_MQTT_Publish(gateway->mqtt, topic_name, topic_msg);
#endif    
    
}

int IOT_Gateway_RRPC_Register(void* handle, 
        const char* product_key, 
        const char* device_name, 
        rrpc_request_callback rrpc_callback)
{
    iotx_subdevice_session_pt session = NULL;
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    iotx_device_info_pt pdevice_info = iotx_device_info_get();  
    
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(product_key, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(device_name, FAIL_RETURN);
    PARAMETER_NULL_CHECK_WITH_RESULT(rrpc_callback, FAIL_RETURN);

    /* gateway rrpc */
    if(0 == strncmp(pdevice_info->product_key, product_key, strlen(product_key)) && 
        0 == strncmp(pdevice_info->device_name, device_name, strlen(device_name))) {
        if (gateway->gateway_data.rrpc_callback != NULL) {
            log_info("rrpc_callback have been set");
            return FAIL_RETURN;
        }
    
        gateway->gateway_data.rrpc_callback = rrpc_callback;
    
        return SUCCESS_RETURN;
    }
    
    session = iotx_subdevice_find_session(gateway, product_key, device_name);
    if(NULL == session) {
        log_info("RRPC register fail, can not find device session");
        return FAIL_RETURN;
    }

    /*  rrpc callback can not set twice */
    if (session->rrpc_callback != NULL) {
        log_info("rrpc_callback have been set, can not set again");
        return FAIL_RETURN;
    }

    session->rrpc_callback = rrpc_callback;    

    return SUCCESS_RETURN;
}


int IOT_Gateway_RRPC_Response(void* handle, 
        const char* product_key, 
        const char* device_name,
        const char* message_id, 
        const char* response)
{
#define TOPIC_RRPC_RESPONSE_FMT                   "/sys/%s/%s/rrpc/response/%s"

    int rc = 0;
    char topic[GATEWAY_TOPIC_LEN_MAX] = {0}; 
    iotx_mqtt_topic_info_t topic_msg;
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    iotx_device_info_pt pdevice_info = iotx_device_info_get();
    iotx_subdevice_session_pt session = NULL;    

    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(product_key, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(device_name, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(response, FAIL_RETURN);

    if (0 == strncmp(product_key, pdevice_info->product_key, strlen(product_key)) &&
        0 == strncmp(device_name, pdevice_info->device_name, strlen(device_name))) {
        log_info("gateway RRPC response");
        goto publish_response;
    } else {
        session = gateway->session_list;
        while (session) {
            if (0 == strncmp(product_key, session->product_key, strlen(product_key)) &&
                0 == strncmp(device_name, session->device_name, strlen(device_name))) {
                 log_info("session RRPC response");
                 goto publish_response;
            }
            session = session->next;
        }
        log_info("no session, can not response");
        return FAIL_RETURN;
    }

    publish_response:
    
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    HAL_Snprintf(topic, 
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_RRPC_RESPONSE_FMT, 
            product_key,
            device_name, 
            message_id);

    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)response;
    topic_msg.payload_len = strlen(response);
    topic_msg.packet_id = 0;
    
    rc = IOT_MQTT_Publish(gateway->mqtt, topic, &topic_msg);
    log_info("rc %d", rc);
    return rc;
}


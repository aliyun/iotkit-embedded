#include "iot_import.h"

#include "utils_list.h"
#include "lite-utils.h"
#include "device.h"
#include "iotx_subdev_common.h"


iotx_gateway_t g_gateway_subdevice = {0};
iotx_gateway_pt g_gateway_subdevice_t = &g_gateway_subdevice;


static int iotx_subdevice_common_reply_proc(iotx_gateway_pt gateway, 
        char* payload,
        iotx_gateway_publish_t reply_type)
{
    char* node = NULL;
    iotx_common_reply_data_pt reply_data = NULL;    

    log_info("recv reply");

    if (gateway == NULL || payload == NULL) {
        log_info("param error");
        return FAIL_RETURN;
    }
        
    switch(reply_type) {
        case IOTX_GATEWAY_PUBLISH_REGISTER:
            reply_data = &gateway->gateway_data.register_reply;
            break;
        case IOTX_GATEWAY_PUBLISH_UNREGISTER:
            reply_data = &gateway->gateway_data.unregister_reply;
            break;
        case IOTX_GATEWAY_PUBLISH_LOGIN:
            reply_data = &gateway->gateway_data.login_reply;
            break;
        case IOTX_GATEWAY_PUBLISH_LOGOUT:
            reply_data = &gateway->gateway_data.logout_reply;
            break;
        case IOTX_GATEWAY_PUBLISH_TOPO_ADD:
            reply_data = &gateway->gateway_data.topo_add_reply;
            break;
        case IOTX_GATEWAY_PUBLISH_TOPO_DELETE:
            reply_data = &gateway->gateway_data.topo_delete_reply;
            break;
        case IOTX_GATEWAY_PUBLISH_TOPO_GET:
            reply_data = &gateway->gateway_data.topo_get_reply;
            break;
        default:
            log_info("param error");
            return FAIL_RETURN;
    }

    /* parse result */
    /* parse   id */
    node = LITE_json_value_of("id", payload);
    if (node == NULL) { 
        log_err("get id of json error!");
        return FAIL_RETURN;
    }
    
    if (reply_data->id == atoi(node)) {
        reply_data->id = 0;
    }

    LITE_free(node);
    node = NULL;
    /* parse   code */
    node = LITE_json_value_of("code", payload);
    if (node == NULL) {
        log_err("get code of json error!");
        return FAIL_RETURN;
    }
    
    reply_data->code = atoi(node);
    LITE_free(node);
    node = NULL;

    if (IOTX_GATEWAY_PUBLISH_REGISTER == reply_type) {        
        /* parse   code */
        node = LITE_json_value_of("data", payload);
        if (node == NULL) {
            log_err("register reply: get data of json error!");
            return FAIL_RETURN;
        }
        if (strlen(node) > REPLY_MESSAGE_LEN_MAX) {
            log_err("topo_get reply size is large then REPLY_MESSAGE_LEN_MAX, please modify the REPLY_MESSAGE_LEN_MAX");
            return FAIL_RETURN;
        }
        memset(gateway->gateway_data.register_message, 0x0, REPLY_MESSAGE_LEN_MAX);   
        strncpy(gateway->gateway_data.register_message, node, strlen(node));    
         
        LITE_free(node);
        node = NULL;
    } else if (IOTX_GATEWAY_PUBLISH_TOPO_GET == reply_type) {        
        /* parse   code */
        node = LITE_json_value_of("data", payload);
        if (node == NULL) {
            log_err("topo_get reply: get data of json error!");
            return FAIL_RETURN;
        }
        if (strlen(node) > REPLY_MESSAGE_LEN_MAX) {  
            LITE_free(node);
            log_err("topo_get reply size is large then REPLY_MESSAGE_LEN_MAX, please modify the REPLY_MESSAGE_LEN_MAX");
            return FAIL_RETURN;
        }
        memset(gateway->gateway_data.topo_get_message, 0x0, REPLY_MESSAGE_LEN_MAX);   
        strncpy(gateway->gateway_data.topo_get_message, node, strlen(node));   
        LITE_free(node);
        node = NULL;
    }

    return SUCCESS_RETURN;
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
        iotx_subdevice_common_reply_proc(gateway, recv_payload, IOTX_GATEWAY_PUBLISH_LOGIN);
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
        iotx_subdevice_common_reply_proc(gateway, recv_payload, IOTX_GATEWAY_PUBLISH_LOGOUT);
        return SUCCESS_RETURN;
    }

    /* register_reply */
    HAL_Snprintf(topic,
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SESSION_SUB_FMT, 
            pdevice_info->product_key, 
            pdevice_info->device_name, 
            "register_reply");
    if ((strlen(recv_topic) == strlen(topic)) && 
      (0 == strncmp(recv_topic, topic, strlen(topic)))) {
        if (SUCCESS_RETURN == iotx_subdevice_common_reply_proc(gateway, recv_payload, IOTX_GATEWAY_PUBLISH_REGISTER))
            return SUCCESS_RETURN;
        else
            return FAIL_RETURN;
    }

    /* unregister_reply */
    HAL_Snprintf(topic,
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SESSION_SUB_FMT, 
            pdevice_info->product_key, 
            pdevice_info->device_name, 
            "unregister_reply");
    if ((strlen(recv_topic) == strlen(topic)) && 
      (0 == strncmp(recv_topic, topic, strlen(topic)))) {
        iotx_subdevice_common_reply_proc(gateway, recv_payload, IOTX_GATEWAY_PUBLISH_UNREGISTER);
        return SUCCESS_RETURN;
    }

    /* todo_add_reply */
    HAL_Snprintf(topic,
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SESSION_TOPO_FMT, 
            pdevice_info->product_key, 
            pdevice_info->device_name, 
            "add_reply");
    if ((strlen(recv_topic) == strlen(topic)) && 
      (0 == strncmp(recv_topic, topic, strlen(topic)))) {
        iotx_subdevice_common_reply_proc(gateway, recv_payload, IOTX_GATEWAY_PUBLISH_TOPO_ADD);
        return SUCCESS_RETURN;
    }      
      
    /* todo_delete_reply */
    HAL_Snprintf(topic,
          GATEWAY_TOPIC_LEN_MAX, 
          TOPIC_SESSION_TOPO_FMT, 
          pdevice_info->product_key, 
          pdevice_info->device_name, 
          "delete_reply");
    if ((strlen(recv_topic) == strlen(topic)) && 
    (0 == strncmp(recv_topic, topic, strlen(topic)))) {
      iotx_subdevice_common_reply_proc(gateway, recv_payload, IOTX_GATEWAY_PUBLISH_TOPO_DELETE);
      return SUCCESS_RETURN;
    }

    /* todo_get_reply */
    HAL_Snprintf(topic,
          GATEWAY_TOPIC_LEN_MAX, 
          TOPIC_SESSION_TOPO_FMT, 
          pdevice_info->product_key, 
          pdevice_info->device_name, 
          "get_reply");
    if ((strlen(recv_topic) == strlen(topic)) && 
    (0 == strncmp(recv_topic, topic, strlen(topic)))) {
      iotx_subdevice_common_reply_proc(gateway, recv_payload, IOTX_GATEWAY_PUBLISH_TOPO_GET);
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


/* parse reigster_reply result */
static int iotx_subdevice_parse_register_reply(        char* message,
        const char* product_key, 
        const char* device_name,
        char* device_secret)
{
    char* node = NULL;
    char* data = message;

    /* check parameter */
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(data, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(product_key, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(device_name, FAIL_RETURN);
    PARAMETER_NULL_CHECK_WITH_RESULT(device_secret, FAIL_RETURN);

    /* there is a '[' in data */
    if (data[0] == '[')
        data++;   

    /* product key */
    node = LITE_json_value_of("productKey", data);
    if (node == NULL) { 
        log_err("get id of json error!");
        return FAIL_RETURN;
    }
    if (0 != strncmp(node, product_key, strlen(product_key))) {
        LITE_free(node);
        log_err("productkey error!");
        return FAIL_RETURN;
    }

    /* device name */
    LITE_free(node);
    node = NULL;
    node = LITE_json_value_of("deviceName", data);
    if (node == NULL) { 
        log_err("get id of json error!");
        return FAIL_RETURN;
    }    
    if (0 != strncmp(node, device_name, strlen(device_name))) {
        LITE_free(node);
        log_err("deviceName error!");
        return FAIL_RETURN;
    }
    
    /* device secret */    
    LITE_free(node);
    node = NULL;
    node = LITE_json_value_of("deviceSecret", data);
    if (node == NULL) { 
        log_err("get id of json error!");
        return FAIL_RETURN;
    }   
    strncpy(device_secret, node, strlen(node));   

    LITE_free(node);
    node = NULL;
    
    return SUCCESS_RETURN;
}

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
            
        case IOTX_MQTT_EVENT_DISCONNECT:
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

/* global message id */
uint32_t global_gateway_cloud_id = 0;

uint32_t IOT_Gateway_Generate_Message_ID()
{
    global_gateway_cloud_id += 1;

    return ((global_gateway_cloud_id == 0 )? global_gateway_cloud_id += 1 : global_gateway_cloud_id);
}


void* IOT_Gateway_Construct(iotx_gateway_param_pt gateway_param)
{        
    PARAMETER_NULL_CHECK_WITH_RESULT(gateway_param, NULL);
    PARAMETER_NULL_CHECK_WITH_RESULT(gateway_param->mqtt, NULL);

    if (g_gateway_subdevice_t->is_construct) {
        log_err("gateway have been construct");
        return NULL;
    }

    memset(g_gateway_subdevice_t, 0x0, sizeof(iotx_gateway_t));
        
    gateway_param->mqtt->handle_event.h_fp = iotx_gateway_event_handle;
    gateway_param->mqtt->handle_event.pcontext = g_gateway_subdevice_t;

    /* construct MQTT client */
    if (NULL == (g_gateway_subdevice_t->mqtt = IOT_MQTT_Construct(gateway_param->mqtt))) {
        log_err("construct MQTT failed");
        return NULL;
    }

#ifdef IOT_GATEWAY_SUPPORT_MULTI_THREAD
    g_gateway_subdevice_t->gateway_data.lock_sync = HAL_MutexCreate();
    g_gateway_subdevice_t->gateway_data.lock_sync_enter = HAL_MutexCreate();
    g_gateway_subdevice_t->gateway_data.lock_login = HAL_MutexCreate();
    g_gateway_subdevice_t->gateway_data.lock_login_enter = HAL_MutexCreate();
    if (NULL == g_gateway_subdevice_t->gateway_data.lock_sync || 
        NULL == g_gateway_subdevice_t->gateway_data.lock_sync_enter || 
        NULL == g_gateway_subdevice_t->gateway_data.lock_login ||
        NULL == g_gateway_subdevice_t->gateway_data.lock_login_enter)
    {
        log_err("create mutex error");
        return NULL;
    }
#endif
    
    /* handle mqtt event for user */
    g_gateway_subdevice_t->event_handler = gateway_param->event_handler;
    g_gateway_subdevice_t->event_pcontext = gateway_param->event_pcontext;

    /* subscribe default topic */
    if (FAIL_RETURN == iotx_gateway_subscribe_unsubscribe_default(g_gateway_subdevice_t, 1)) {
        log_err("subscribe default topic fialed");
        IOT_Gateway_Destroy((void**)&g_gateway_subdevice_t); 
        return NULL;
    }

    g_gateway_subdevice_t->is_construct = 1;

    return g_gateway_subdevice_t;
}


/* Register: static and dynamic */    
int IOT_Subdevice_Register(void* handle, 
        iotx_subdev_register_types_t type, 
        const char* product_key, 
        const char* device_name,
        char* timestamp, 
        char* client_id, 
        char* sign,
        iotx_subdev_sign_method_types_t sign_type)
{    
    uint32_t msg_id = 0;
    int rc = 0;
    char* packet = NULL;
    char topic[GATEWAY_TOPIC_LEN_MAX] = {0};
    char device_secret[DEVICE_SECRET_LEN] = {0};
    iotx_device_info_pt pdevice_info = iotx_device_info_get();
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    iotx_subdevice_session_pt session = NULL;

    /* parameter check */
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(product_key, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(device_name, FAIL_RETURN);
    /* check register type */
    if (type != IOTX_SUBDEV_REGISTER_TYPE_DYNAMIC && 
            type != IOTX_SUBDEV_REGISTER_TYPE_STATIC) {
        log_info("register type not support");
        return FAIL_RETURN;
    }

    /* dynamic: get device secret first */
    /* sync wait for response */
    if (IOTX_SUBDEV_REGISTER_TYPE_DYNAMIC == type) {
        if (NULL != timestamp && NULL != client_id && NULL != sign) {
            log_info("parameter error, if dynamic register, timestamp = client_id = sign = NULL");
            return FAIL_RETURN;
        }                

        /* topic */
        HAL_Snprintf(topic, 
                GATEWAY_TOPIC_LEN_MAX, 
                TOPIC_SESSION_SUB_FMT, 
                pdevice_info->product_key, 
                pdevice_info->device_name, 
                "register");
    
        /* packet */    
        packet = iotx_gateway_splice_common_packet(product_key,
                            device_name,
                            "thing.sub.register",
                            &msg_id);
                
        if (packet == NULL) {
            log_err("login packet splice error!");
            return FAIL_RETURN;
        }        
        
        /* publish packet */
        if (FAIL_RETURN == iotx_gateway_publish_sync(gateway, 
                IOTX_MQTT_QOS0, 
                topic, 
                packet, 
                msg_id, 
                &(gateway->gateway_data.register_reply),
                IOTX_GATEWAY_PUBLISH_REGISTER)) {
            LITE_free(packet);
            log_err("MQTT Publish error!");
            return FAIL_RETURN;        
        }
                
        LITE_free(packet);  
                                
        if (FAIL_RETURN == iotx_subdevice_parse_register_reply(gateway->gateway_data.register_message,
                                 product_key, 
                                 device_name, 
                                 device_secret)) {
            log_info("parse register reply error");
            return FAIL_RETURN;
        }        
        log_info("register success, secret %s", device_secret);
        
        /* timestamp */
        MALLOC_MEMORY_WITH_RESULT(timestamp, 20, FAIL_RETURN);
        strncpy(timestamp, "2524608000000", strlen("2524608000000") + 1);
       
        /* client id */
        MALLOC_MEMORY_WITH_FREE_AND_RESULT(client_id, IOT_SUBDEVICE_CLIENT_ID_LEN, timestamp, FAIL_RETURN);
        iotx_subdevice_calc_client_id(client_id, product_key, device_name);     

        /* sign */  
        MALLOC_MEMORY_WITH_FREE_AND_RESULT(sign, 41, timestamp, FAIL_RETURN);    
        if (FAIL_RETURN == (rc = iotx_gateway_calc_sign(product_key,
                                device_name,
                                device_secret,
                                sign, 
                                41,
                                sign_type,
                                client_id,
                                timestamp))) {
            log_err("sign fail");
            goto exit;
        }                

        if (NULL == (session = iotx_subdevice_add_session(gateway,
                                    product_key, 
                                    device_name, 
                                    NULL, 
                                    sign,
                                    timestamp,
                                    client_id,
                                    sign_type,
                                    IOTX_SUBDEV_CLEAN_SESSION_FALSE))) {
            log_err("create session error!");
            rc = FAIL_RETURN;
            goto exit;
        }    
        iotx_subdevice_set_session_status(session, IOTX_SUBDEVICE_SEESION_STATUS_REGISTER);
        iotx_subdevice_set_session_dynamic_register(session);
    } 

    /* check parameter */
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(timestamp, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(client_id, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(sign, FAIL_RETURN);
    
    /* check sign type */
    if (sign_type != IOTX_SUBDEV_SIGN_METHOD_TYPE_SHA && 
            sign_type != IOTX_SUBDEV_SIGN_METHOD_TYPE_MD5) {
        log_info("register type not support");
        rc = FAIL_RETURN;
        goto exit;
    }

    /* topo add */   
    /* topic */
    memset(topic, 0x0, GATEWAY_TOPIC_LEN_MAX);
    HAL_Snprintf(topic,
            GATEWAY_TOPIC_LEN_MAX,
            TOPIC_SESSION_TOPO_FMT,
            pdevice_info->product_key,
            pdevice_info->device_name,
            "add");  
                            
    /* topo packet */
    if (sign_type == IOTX_SUBDEV_SIGN_METHOD_TYPE_SHA) {
        packet = iotx_gateway_splice_topo_add_packet(product_key, 
                        device_name, 
                        client_id,
                        timestamp,  
                        "hmacsha1", 
                        sign, 
                        "thing.topo.add",
                        &msg_id);
    } else if (sign_type == IOTX_SUBDEV_SIGN_METHOD_TYPE_MD5) {
        packet = iotx_gateway_splice_topo_add_packet(product_key, 
                        device_name, 
                        client_id,
                        timestamp,  
                        "hmacmd5", 
                        sign, 
                        "thing.topo.add",
                        &msg_id);
    }
    
    if (packet == NULL) {
        log_err("login packet splice error!");
        rc = FAIL_RETURN;
        goto exit;
    }        
    
    /* publish packet */
    if (FAIL_RETURN == (rc = iotx_gateway_publish_sync(gateway, 
            IOTX_MQTT_QOS0, 
            topic, 
            packet, 
            msg_id, 
            &(gateway->gateway_data.topo_add_reply),
            IOTX_GATEWAY_PUBLISH_TOPO_ADD))) {
        goto exit;
    }

exit: 
    if (packet)
        LITE_free(packet);    
    
    if (IOTX_SUBDEV_REGISTER_TYPE_DYNAMIC == type) {
        LITE_free(timestamp);
        LITE_free(client_id);
        LITE_free(sign);
    }

    return rc;
}

/* unregister: topo delete first, then unregister */
int IOT_Subdevice_Unregister(void* handle,
        const char* product_key, 
        const char* device_name)
{
    uint32_t msg_id = 0;
    iotx_device_info_pt pdevice_info = iotx_device_info_get();
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    char* packet = NULL;
    char topic[GATEWAY_TOPIC_LEN_MAX] = {0};

    /* parameter check */
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(product_key, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(device_name, FAIL_RETURN);    

    /* unregister */
    /* topic */
    HAL_Snprintf(topic, 
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SESSION_SUB_FMT, 
            pdevice_info->product_key, 
            pdevice_info->device_name, 
            "unregister");

    /* splice unregister packet */
    packet = iotx_gateway_splice_common_packet(product_key, 
                            device_name, 
                            "thing.sub.unregister",
                            &msg_id);
    if (packet == NULL) {
        log_err("unregister packet splice error!");
        return FAIL_RETURN;
    }

    if (FAIL_RETURN == iotx_gateway_publish_sync(gateway, 
            IOTX_MQTT_QOS0, 
            topic, 
            packet, 
            msg_id, 
            &(gateway->gateway_data.unregister_reply),
            IOTX_GATEWAY_PUBLISH_UNREGISTER)) {
        LITE_free(packet);
        log_err("MQTT Publish error!");
        return FAIL_RETURN;        
    }

    LITE_free(packet); 

    /* topo delete */
    /* topic */
    HAL_Snprintf(topic, 
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SESSION_TOPO_FMT, 
            pdevice_info->product_key, 
            pdevice_info->device_name, 
            "delete");

    /* splice logout packet */
    packet = iotx_gateway_splice_common_packet(product_key, 
                            device_name, 
                            "thing.topo.delete",
                            &msg_id);
    if (packet == NULL) {
        log_err("topo_delete packet splice error!");
        return FAIL_RETURN;
    }

    if (FAIL_RETURN == iotx_gateway_publish_sync(gateway, 
            IOTX_MQTT_QOS0, 
            topic, 
            packet, 
            msg_id, 
            &(gateway->gateway_data.topo_delete_reply),
            IOTX_GATEWAY_PUBLISH_TOPO_DELETE)) {
        LITE_free(packet);
        log_err("MQTT Publish error!");
        return FAIL_RETURN;        
    }

    LITE_free(packet);            
    
    return SUCCESS_RETURN;
}


int IOT_Subdevice_Login(void* handle, 
        const char* product_key, 
        const char* device_name, 
        const char* timestamp, 
        const char* client_id, 
        const char* sign, 
        iotx_subdev_sign_method_types_t sign_method_type,
        iotx_subdev_clean_session_types_t clean_session_type)
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
    
    /* check sign method */            
    if (clean_session_type != IOTX_SUBDEV_CLEAN_SESSION_TRUE && 
            clean_session_type != IOTX_SUBDEV_CLEAN_SESSION_FALSE) {
        log_info("clean session not support");
        return FAIL_RETURN;
    }

    if (NULL == (session = iotx_subdevice_find_session(gateway,
                                product_key, 
                                device_name))) {
        log_info("there is no seesion, create a new session");
    
        PARAMETER_STRING_NULL_CHECK_WITH_RESULT(sign, FAIL_RETURN);
        PARAMETER_STRING_NULL_CHECK_WITH_RESULT(client_id, FAIL_RETURN);
        PARAMETER_STRING_NULL_CHECK_WITH_RESULT(timestamp, FAIL_RETURN);
        
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
        if (IOTX_SUBDEVICE_SEESION_STATUS_LOGIN == session->session_status) {
            log_info("device have been login");
            return FAIL_RETURN;
        }
    }

    /* topic */
    HAL_Snprintf(topic, 
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SESSION_COMBINE_FMT, 
            pdevice_info->product_key, 
            pdevice_info->device_name, 
            "login");

    if (IOTX_SUBDEVICE_SEESION_STATUS_REGISTER == session->session_status) {
        sign = session->sign;
        sign_method_type = session->sign_method;
        client_id = session->client_id;
        timestamp = session->timestamp;        
    }
    
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(sign, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(client_id, FAIL_RETURN);
    PARAMETER_STRING_NULL_CHECK_WITH_RESULT(timestamp, FAIL_RETURN);
    
    if (sign_method_type == IOTX_SUBDEV_SIGN_METHOD_TYPE_SHA) {
        strncpy(sign_method, "hmacsha1", strlen("hmacsha1"));
    } else {
        strncpy(sign_method, "hmacmd5", strlen("hmacmd5"));
    } 

    if (clean_session_type == IOTX_SUBDEV_CLEAN_SESSION_TRUE) {
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

    if (IOTX_SUBDEVICE_SEESION_STATUS_LOGIN != session->session_status) {
        log_info("status is not login, can not logout");
        iotx_subdevice_remove_session(gateway, product_key, device_name);
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


int IOT_Gateway_Get_TOPO(void* handle, 
        char* get_toop_reply, 
        uint32_t* length)
{
    uint32_t msg_id = 0;
    char topic[GATEWAY_TOPIC_LEN_MAX] = {0}; 
    char* topo_get_packet = NULL;
    iotx_mqtt_topic_info_t topic_msg;
    iotx_device_info_pt pdevice_info = iotx_device_info_get();
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    PARAMETER_NULL_CHECK_WITH_RESULT(get_toop_reply, FAIL_RETURN);
    
    /* topic */
    HAL_Snprintf(topic, 
            GATEWAY_TOPIC_LEN_MAX, 
            TOPIC_SESSION_TOPO_FMT, 
            pdevice_info->product_key, 
            pdevice_info->device_name, 
            "get");

    /* splice topo_get packet */
    topo_get_packet = iotx_gateway_splice_topo_get_packet(&msg_id);
    if (topo_get_packet == NULL) {
        log_err("topo_get packet splice error!");
        return FAIL_RETURN;
    }

    /* publish topo_get packet */
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));

    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)topo_get_packet;
    topic_msg.payload_len = strlen(topo_get_packet);
    topic_msg.packet_id = 0;

    if (FAIL_RETURN == iotx_gateway_publish_sync(gateway, 
            IOTX_MQTT_QOS0, 
            topic, 
            topo_get_packet, 
            msg_id, 
            &(gateway->gateway_data.topo_get_reply),
            IOTX_GATEWAY_PUBLISH_TOPO_GET)) {
        LITE_free(topo_get_packet);
        log_err("MQTT Publish error!");
        return FAIL_RETURN;        
    }

    LITE_free(topo_get_packet); 

    if (*length < strlen(gateway->gateway_data.topo_get_message)) {
        log_err("set memory too small");
        return FAIL_RETURN;
    }

    strncpy(get_toop_reply, gateway->gateway_data.topo_get_message, strlen(gateway->gateway_data.topo_get_message));

    *length = strlen(gateway->gateway_data.topo_get_message);
            
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
    
    *handle = NULL;
    g_gateway_subdevice_t->is_construct = 0;
    
    return SUCCESS_RETURN;
}


int IOT_Gateway_Yield(void* handle, uint32_t timeout)
{
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    
    return IOT_MQTT_Yield(gateway->mqtt, timeout);
}

int IOT_Gateway_Subscribe(void* handle, 
        const char *topic_filter, 
        iotx_mqtt_qos_t qos, 
        iotx_mqtt_event_handle_func_fpt topic_handle_func, 
        void *pcontext)
{
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    
    return IOT_MQTT_Subscribe(gateway->mqtt, topic_filter, qos, topic_handle_func, pcontext);
}


int IOT_Gateway_Unsubscribe(void* handle, 
        const char *topic_filter)
{
    iotx_gateway_pt gateway = (iotx_gateway_pt)handle;
    
    PARAMETER_GATEWAY_CHECK(gateway, FAIL_RETURN);
    
    return IOT_MQTT_Unsubscribe(gateway->mqtt, topic_filter);
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
    
    return IOT_MQTT_Publish(gateway->mqtt, topic_name, topic_msg);
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


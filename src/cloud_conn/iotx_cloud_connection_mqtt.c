#include "iot_import.h"
#include "lite-log.h"
#include "lite-utils.h"
#include "utils_timer.h"
#include "utils_list.h"
#include "lite-system.h"
#include "utils_hmac.h"
#include "iot_export.h"

#include "iotx_cloud_connection_common.h"

typedef struct iotx_cloud_connection_topic_st {   
    void            *next;
    char            *topic;
    int              packet_id;
} iotx_cloud_connection_topic_t, *iotx_cloud_connection_topic_pt;


typedef struct iotx_cloud_connection_mqtt_st {    
    char            *msg_buf;
    char            *msg_readbuf;
    int              list_length;
    iotx_cloud_connection_topic_pt   topic_list;
} iotx_cloud_connection_mqtt_t, *iotx_cloud_connection_mqtt_pt;

static iotx_cloud_connection_mqtt_pt mqtt_pt = NULL;

static int _add_topic(const char* topic, int packet_id)
{
    iotx_cloud_connection_topic_pt new_pt = NULL;  
    
    new_pt = LITE_malloc(sizeof(iotx_cloud_connection_topic_t));
    if (NULL == new_pt) {
        log_info("memory error");
        return FAIL_RETURN;
    }
    memset(new_pt, 0x0, sizeof(iotx_cloud_connection_topic_t));
    
    new_pt->topic = LITE_malloc(strlen(topic) + 1);
    if (NULL == new_pt->topic) {
        log_info("memory error");
        LITE_free(new_pt);
        return FAIL_RETURN;
    }
    memset(new_pt->topic, 0x0, strlen(topic) + 1);
    strncpy(new_pt->topic, topic, strlen(topic));
    
    new_pt->packet_id = packet_id;
        
    new_pt->next = mqtt_pt->topic_list;   

    mqtt_pt->topic_list = new_pt;
    
    mqtt_pt->list_length++;

    return SUCCESS_RETURN;
}

static int _delete_topic(unsigned int packet_id)
{
    iotx_cloud_connection_topic_pt current = NULL;
    iotx_cloud_connection_topic_pt pre = NULL;
    
    if (NULL == mqtt_pt->topic_list) {
        log_info("no list");
        return FAIL_RETURN;
    }

    current = pre = mqtt_pt->topic_list;

    if (current->packet_id == packet_id){ 
        mqtt_pt->topic_list = mqtt_pt->topic_list->next;       
        LITE_free(current->topic);
        LITE_free(current);
        mqtt_pt->list_length--;
        if (mqtt_pt->list_length == 0)
            mqtt_pt->topic_list = NULL;
        return SUCCESS_RETURN;
    }

    current = current->next;

    while (current) {
        if (current->packet_id == packet_id) {
            pre->next = current->next;            
            LITE_free(current->topic);
            LITE_free(current); 
            
            mqtt_pt->list_length--;
            
            if (mqtt_pt->list_length == 0)
                mqtt_pt = NULL;
            return SUCCESS_RETURN;
        }
        pre = current;
        current = current->next;
    }

    return FAIL_RETURN;
}

static char* _find_topic(unsigned int packet_id)
{
    iotx_cloud_connection_topic_pt current = NULL;
    iotx_cloud_connection_topic_pt pre = NULL;
    
    if (NULL == mqtt_pt->topic_list) {
        log_info("no list ");
        return NULL;
    }

    current = pre = mqtt_pt->topic_list;

    while (current) {
        if (current->packet_id == packet_id) {            
            return current->topic;
        }
        pre = current;
        current = current->next;
    }

    return NULL;
}

static void _delete_all()
{
    iotx_cloud_connection_topic_pt current = NULL;
    iotx_cloud_connection_topic_pt next = NULL;
    
    if (NULL == mqtt_pt->topic_list) {
        log_info("no list");
        return;
    }

    current = next = mqtt_pt->topic_list;

    while (current) {  
        next = current->next;          
        LITE_free(current->topic);
        LITE_free(current); 
        current = next;
    }

    mqtt_pt->list_length = 0;
}

static iotx_mqtt_qos_t _QoS (iotx_message_qos_t qos)
{
    switch(qos) {
        case IOTX_MESSAGE_QOS0:
            return IOTX_MQTT_QOS0;
    
        case IOTX_MESSAGE_QOS1:
            return IOTX_MQTT_QOS1;
    
        case IOTX_MESSAGE_QOS2:
            return IOTX_MQTT_QOS2;

        default:
            return IOTX_MQTT_QOS0;        
    }
}


static void iotx_cloud_connection_mqtt_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)pcontext;  

    if (NULL == connection) {
        log_info("error");
        return;
    }

    log_info("event_type %d", msg->event_type);

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            log_info("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT: {
            iotx_cloud_connection_event_msg_t event;
            
            event.event_id = IOTX_CLOUD_CONNECTION_EVENT_DISCONNECT;
            event.msg = NULL;
            
            log_info("MQTT disconnect.");
            
            if (connection->event_handler)
                connection->event_handler(connection, &event);
        }
            break;

        case IOTX_MQTT_EVENT_RECONNECT: {
            iotx_cloud_connection_event_msg_t event;
            
            event.event_id = IOTX_CLOUD_CONNECTION_EVENT_RECONNECT;
            event.msg = NULL;
            
            log_info("MQTT reconnect.");
            
            if (connection->event_handler)
                connection->event_handler(connection, &event);
        }
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS: {
            iotx_cloud_connection_msg_rsp_t msg_rsp;
            
            log_info("subscribe success, packet-id=%u", (unsigned int)packet_id);
        
            memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
            msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_SUBSCRIBE_SUCCESS;
            msg_rsp.URI = _find_topic(packet_id);
            msg_rsp.URI_length = strlen(msg_rsp.URI);
            
            if (connection->resposne_handler)
                connection->resposne_handler(connection, &msg_rsp);    
            
            _delete_topic(packet_id);
        }
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:{
            iotx_cloud_connection_msg_rsp_t msg_rsp;
            
            log_info("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
        
            memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
            msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_SUBSCRIBE_FAIL;
            msg_rsp.URI = _find_topic(packet_id);
            msg_rsp.URI_length = strlen(msg_rsp.URI);
            
            if (connection->resposne_handler)
                connection->resposne_handler(connection, &msg_rsp);     
            
            _delete_topic(packet_id);
        }
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:{
            iotx_cloud_connection_msg_rsp_t msg_rsp;
            
            log_info("subscribe nack, packet-id=%u", (unsigned int)packet_id);
        
            memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
            msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_SUBSCRIBE_FAIL;
            msg_rsp.URI = _find_topic(packet_id);
            msg_rsp.URI_length = strlen(msg_rsp.URI);
            
            if (connection->resposne_handler)
                connection->resposne_handler(connection, &msg_rsp);   
            
            _delete_topic(packet_id);
        }
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:{
            iotx_cloud_connection_msg_rsp_t msg_rsp;
            
            log_info("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
        
            memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
            msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_UNSUBSCRIBE_SUCCESS;
            msg_rsp.URI = _find_topic(packet_id);
            msg_rsp.URI_length = strlen(msg_rsp.URI);
            
            if (connection->resposne_handler)
                connection->resposne_handler(connection, &msg_rsp);   
            
            _delete_topic(packet_id);
        }
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:{
            iotx_cloud_connection_msg_rsp_t msg_rsp;
            
            log_info("unsubscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
        
            memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
            msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_UNSUBSCRIBE_FAIL;
            msg_rsp.URI = _find_topic(packet_id);
            msg_rsp.URI_length = strlen(msg_rsp.URI);
            
            if (connection->resposne_handler)
                connection->resposne_handler(connection, &msg_rsp);  
            
            _delete_topic(packet_id);
        }
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:{
            iotx_cloud_connection_msg_rsp_t msg_rsp;
            
            log_info("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
        
            memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
            msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_UNSUBSCRIBE_FAIL;
            msg_rsp.URI = _find_topic(packet_id);
            msg_rsp.URI_length = strlen(msg_rsp.URI);
            
            if (connection->resposne_handler)
                connection->resposne_handler(connection, &msg_rsp);   
            
            _delete_topic(packet_id);

            LITE_free(msg_rsp.URI);
        }
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:{
            iotx_cloud_connection_msg_rsp_t msg_rsp;
            
            log_info("publish success, packet-id=%u", (unsigned int)packet_id);
        
            memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
            msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_SEND_SUCCESS;
            msg_rsp.URI = _find_topic(packet_id);
            msg_rsp.URI_length = strlen(msg_rsp.URI);
            
            if (connection->resposne_handler)
                connection->resposne_handler(connection, &msg_rsp);   
            
            _delete_topic(packet_id);
        }
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:{
            iotx_cloud_connection_msg_rsp_t msg_rsp;
            
            log_info("publish timeout, packet-id=%u", (unsigned int)packet_id);
        
            memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
            msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_SEND_FAIL;
            msg_rsp.URI = _find_topic(packet_id);
            msg_rsp.URI_length = strlen(msg_rsp.URI);
            
            if (connection->resposne_handler)
                connection->resposne_handler(connection, &msg_rsp);   
            
            _delete_topic(packet_id);
        }
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:{
            iotx_cloud_connection_msg_rsp_t msg_rsp;
            
            log_info("publish nack, packet-id=%u", (unsigned int)packet_id);
        
            memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
            msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_SEND_FAIL;
            msg_rsp.URI = _find_topic(packet_id);
            msg_rsp.URI_length = strlen(msg_rsp.URI);
            
            if (connection->resposne_handler)
                connection->resposne_handler(connection, &msg_rsp);   
            
            _delete_topic(packet_id);
        }
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:{
            iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;
            iotx_cloud_connection_msg_rsp_t msg_rsp;
            
            log_info("topic message arrived: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
        
            memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
            msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_NEW_DATA;
            msg_rsp.URI = LITE_malloc(topic_info->topic_len + 1);
            memset(msg_rsp.URI, 0x0, topic_info->topic_len + 1);     
            strncpy(msg_rsp.URI, topic_info->ptopic, topic_info->topic_len);
            msg_rsp.URI_length = topic_info->topic_len;
            msg_rsp.payload = LITE_malloc(topic_info->payload_len + 1);
            memset(msg_rsp.payload, 0x0, topic_info->payload_len + 1);     
            strncpy(msg_rsp.payload, topic_info->payload, topic_info->payload_len);
            msg_rsp.payload_length = topic_info->payload_len;
            
            if (connection->resposne_handler)
                connection->resposne_handler(connection, &msg_rsp);     

            LITE_free(msg_rsp.URI);
            LITE_free(msg_rsp.payload);
        }
            break;

        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
            log_info("buffer overflow, %s", msg->msg);
            break;

        default:
            log_info("Should NOT arrive here.");
            break;
    }
}


/*  mqtt */
void* iotx_cloud_connection_mqtt_init(void *pparam, void* handle)
{
    iotx_cloud_connection_param_pt param = (iotx_cloud_connection_param_pt)pparam;
    iotx_mqtt_param_t mqtt_param;
    iotx_conn_info_pt pconn_info;
    void *pclient;
    
    if (NULL == param) {
        log_info("parameter error");
        return NULL;
    }

    mqtt_pt = LITE_malloc(sizeof(iotx_cloud_connection_mqtt_t));
    if (NULL == mqtt_pt){
        log_info("not enough memory");
        return NULL;
    }
    memset(mqtt_pt, 0x0, sizeof(iotx_cloud_connection_mqtt_t));

    if (NULL == (mqtt_pt->msg_buf = (char *)LITE_malloc(MQTT_MSGLEN))) {
        log_info("not enough memory");
        LITE_free(mqtt_pt);
        return NULL;
    }
    memset(mqtt_pt->msg_buf, 0x0, MQTT_MSGLEN);

    if (NULL == (mqtt_pt->msg_readbuf = (char *)LITE_malloc(MQTT_MSGLEN))) {
        log_info("not enough memory");
        LITE_free(mqtt_pt->msg_buf);
        LITE_free(mqtt_pt);
        return NULL;
    }
    memset(mqtt_pt->msg_readbuf, 0x0, MQTT_MSGLEN);

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(param->device_info->product_key, param->device_info->device_name, param->device_info->device_secret, (void **)&pconn_info)) {
        log_info("auth error");
        LITE_free(mqtt_pt->msg_buf);
        LITE_free(mqtt_pt->msg_readbuf);
        LITE_free(mqtt_pt);
        return NULL;
    }

    /* Initialize MQTT parameter */
    memset(&mqtt_param, 0x0, sizeof(mqtt_param));

    mqtt_param.port = pconn_info->port;
    mqtt_param.host = pconn_info->host_name;
    mqtt_param.client_id = pconn_info->client_id;
    mqtt_param.username = pconn_info->username;
    mqtt_param.password = pconn_info->password;
    mqtt_param.pub_key = pconn_info->pub_key;

    mqtt_param.request_timeout_ms = param->request_timeout_ms;
    mqtt_param.clean_session = param->clean_session;
    mqtt_param.keepalive_interval_ms = param->keepalive_interval_ms;
    mqtt_param.pread_buf = mqtt_pt->msg_readbuf;
    mqtt_param.read_buf_size = MQTT_MSGLEN;
    mqtt_param.pwrite_buf = mqtt_pt->msg_buf;
    mqtt_param.write_buf_size = MQTT_MSGLEN;

    mqtt_param.handle_event.h_fp = iotx_cloud_connection_mqtt_event_handle;
    mqtt_param.handle_event.pcontext = (void*)handle;


    /* Construct a MQTT client with specify parameter */
    if (NULL == (pclient = IOT_MQTT_Construct(&mqtt_param))) {
        log_info("MQTT construct failed");
        LITE_free(mqtt_pt->msg_buf);
        LITE_free(mqtt_pt->msg_readbuf);
        LITE_free(mqtt_pt);
        return NULL;
    }
    
    return pclient;
}

int iotx_cloud_connection_mqtt_subscribe(void* handle,
                        const char *topic_filter,
                        iotx_message_qos_t qos)
{    
    int rc = 0;
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)handle;  
    
    if (FAIL_RETURN == iotx_cloud_connection_check_function_context(connection))
        return FAIL_RETURN;
    
    rc = IOT_MQTT_Subscribe(connection->function_pt->pcontext, 
                        topic_filter, 
                        _QoS(qos), 
                        iotx_cloud_connection_mqtt_event_handle, 
                        (void*)connection);

    if (rc > 0)
        _add_topic(topic_filter, rc);

    return rc;
}
                        
int iotx_cloud_connection_mqtt_unsubscribe(void* handle,
                        const char *topic_filter)
{
    int rc = 0;
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)handle;  
    
    if (FAIL_RETURN == iotx_cloud_connection_check_function_context(connection))
        return FAIL_RETURN;

    rc = IOT_MQTT_Unsubscribe(connection->function_pt->pcontext, topic_filter);
    
    if (rc > 0)
        _add_topic(topic_filter, rc);
    
    return rc;
}
                        
int iotx_cloud_connection_mqtt_publish(void* handle,
                        iotx_cloud_connection_msg_pt message)
{
    int rc = 0;
    iotx_mqtt_topic_info_t topic_msg; 
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)handle;  
    
    if (FAIL_RETURN == iotx_cloud_connection_check_function_context(connection))
        return FAIL_RETURN;

    if (NULL == message) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));

    topic_msg.dup = 0;
    topic_msg.qos = _QoS(message->QoS);
    topic_msg.retain = 0;
    topic_msg.payload_len = message->payload_length;
    topic_msg.payload = message->payload;
    topic_msg.ptopic = message->URI;
    topic_msg.topic_len = message->URI_length;
    
    rc = IOT_MQTT_Publish(connection->function_pt->pcontext, message->URI, &topic_msg);

    if (IOTX_MESSAGE_QOS0 != topic_msg.qos) {
        if (rc > 0)
            _add_topic(topic_msg.ptopic, rc);
    } else {
        iotx_cloud_connection_msg_rsp_t msg_rsp;  
    
        memset(&msg_rsp, 0x0, sizeof(iotx_cloud_connection_msg_rsp_t));  
        msg_rsp.rsp_type = IOTX_CLOUD_CONNECTION_RESPONSE_SEND_SUCCESS;
        msg_rsp.URI = message->URI;
        msg_rsp.URI_length = message->URI_length;
        
        if (connection->resposne_handler)
            connection->resposne_handler(connection, &msg_rsp);   
    }

    return rc;
}
                        
int iotx_cloud_connection_mqtt_deinit(void* handle)
{
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)handle;  
    
    if (FAIL_RETURN == iotx_cloud_connection_check_function_context(connection))
        return FAIL_RETURN; 

    if (mqtt_pt->list_length != 0)      
        _delete_all();

    if (mqtt_pt->msg_buf)
        LITE_free(mqtt_pt->msg_buf);

    if (mqtt_pt->msg_readbuf)
        LITE_free(mqtt_pt->msg_readbuf);
        
    if (mqtt_pt)
        LITE_free(mqtt_pt);
    
    return IOT_MQTT_Destroy(&(connection->function_pt->pcontext));
}
                        
int iotx_cloud_connection_mqtt_yield(void* handle, int timeout_ms)
{
    iotx_cloud_connection_pt connection = (iotx_cloud_connection_pt)handle;  
    
    if (FAIL_RETURN == iotx_cloud_connection_check_function_context(connection))
        return FAIL_RETURN;

    return IOT_MQTT_Yield(connection->function_pt->pcontext, timeout_ms);
}


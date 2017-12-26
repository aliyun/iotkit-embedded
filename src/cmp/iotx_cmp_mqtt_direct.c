
#ifdef CMP_VIA_MQTT_DIRECT

#include "iot_import.h"

#include "utils_list.h"
#include "lite-utils.h"
#include "device.h"
#include "iot_export.h"
#include "iotx_cmp_common.h"
#include "iot_export_mqtt.h"
#include "iotx_cmp_mqtt_direct.h"

#define MQTT_MSGLEN             (1024 * 4)


typedef struct iotx_cmp_mqtt_direct_topic_st {   
    void            *next;
    char            *topic;
    int              packet_id;
} iotx_cmp_mqtt_direct_topic_t, *iotx_cmp_mqtt_direct_topic_pt;

typedef struct iotx_cmp_mqtt_direct_st {    
    char            *msg_buf;
    char            *msg_readbuf;
    int              list_length;
    iotx_cmp_mqtt_direct_topic_pt   topic_list;
} iotx_cmp_mqtt_direct_t, *iotx_cmp_mqtt_direct_pt;

static iotx_cmp_mqtt_direct_pt mqtt_pt = NULL;

static int _add_topic(const char* topic, int packet_id)
{
    iotx_cmp_mqtt_direct_topic_pt new_pt = NULL;  
    
    new_pt = LITE_malloc(sizeof(iotx_cmp_mqtt_direct_topic_t));
    if (NULL == new_pt) {
        log_info("memory error");
        return FAIL_RETURN;
    }
    memset(new_pt, 0x0, sizeof(iotx_cmp_mqtt_direct_topic_t));
    
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
    iotx_cmp_mqtt_direct_topic_pt current = NULL;
    iotx_cmp_mqtt_direct_topic_pt pre = NULL;
    
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
    iotx_cmp_mqtt_direct_topic_pt current = NULL;
    iotx_cmp_mqtt_direct_topic_pt pre = NULL;
    
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


static int _find_topic_ex(char* URI)
{
    iotx_cmp_mqtt_direct_topic_pt current = NULL;
    iotx_cmp_mqtt_direct_topic_pt pre = NULL;
    
    if (NULL == mqtt_pt->topic_list) {
        log_info("no list ");
        return FAIL_RETURN;
    }

    current = pre = mqtt_pt->topic_list;

    while (current) {
        if (0 == strncmp(current->topic, URI, strlen(URI))) {            
            return current->packet_id;
        }
        pre = current;
        current = current->next;
    }

    return FAIL_RETURN;
}

static void _delete_all()
{
    iotx_cmp_mqtt_direct_topic_pt current = NULL;
    iotx_cmp_mqtt_direct_topic_pt next = NULL;
    
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

static void iotx_cmp_mqtt_direct_event_callback(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)pcontext;  

    if (NULL == cmp_pt) {
        log_info("error");
        return;
    }

    log_info("event_type %d", msg->event_type);

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            log_info("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT: {
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
            iotx_cmp_mqtt_direct_disconnect_handler(cmp_pt);
        #endif
        }
            break;

        case IOTX_MQTT_EVENT_RECONNECT: {
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
            iotx_cmp_mqtt_direct_reconnect_handler(cmp_pt);
        #endif
        }
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS: {   
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

            result_msg->URI = _find_topic(packet_id);
            result_msg->result = 0;
            result_msg->is_register = 1;
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);

            if (FAIL_RETURN == rc) {  
                LITE_free(node->msg);
                LITE_free(node);
            }
        }
        #else
            iotx_cmp_mqtt_direct_register_handler(cmp_pt, _find_topic(packet_id), 0, 1);
            _delete_topic(packet_id);
        #endif  
        }
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:{ 
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

            result_msg->URI = _find_topic(packet_id);
            result_msg->result = -1;
            result_msg->is_register = 1;
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);

            if (FAIL_RETURN == rc) {  
                LITE_free(node->msg);
                LITE_free(node);
            }
        }
        #else
            iotx_cmp_mqtt_direct_register_handler(cmp_pt, _find_topic(packet_id), -1, 1);
            _delete_topic(packet_id);
        #endif  
        }
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:{   
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

            result_msg->URI = _find_topic(packet_id);
            result_msg->result = 0;
            result_msg->is_register = 0;
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);

            if (FAIL_RETURN == rc) {  
                LITE_free(node->msg);
                LITE_free(node);
            }
        }
        #else
            iotx_cmp_mqtt_direct_register_handler(cmp_pt, _find_topic(packet_id), 0, 0);
            _delete_topic(packet_id);
        #endif  
        }
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:{    
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

            result_msg->URI = _find_topic(packet_id);
            result_msg->result = -1;
            result_msg->is_register = 0;
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);

            if (FAIL_RETURN == rc) {  
                LITE_free(node->msg);
                LITE_free(node);
            }
        }
        #else
            iotx_cmp_mqtt_direct_register_handler(cmp_pt, _find_topic(packet_id), -1, 0);
            _delete_topic(packet_id);
        #endif  
        }
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            log_info("error: publish ack");
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:{
        #ifdef CMP_SUPPORT_MULTI_THREAD
        {            
            /* send message to itself thread */
            int rc = 0;
            iotx_mqtt_topic_info_pt mqtt_info = (iotx_mqtt_topic_info_pt)msg->msg;
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

            rsp_msg = node->msg;

            rsp_msg->URI = LITE_malloc(mqtt_info->topic_len + 1);
            if (NULL == rsp_msg->URI) {
                log_info("memory error");
                LITE_free(node->msg);
                LITE_free(node);
                return;
            }
            memset(rsp_msg->URI, 0x0, mqtt_info->topic_len + 1);
            strncpy(rsp_msg->URI, mqtt_info->ptopic, mqtt_info->topic_len);

            rsp_msg->payload = LITE_malloc(mqtt_info->payload_len + 1);
            if (NULL == rsp_msg->payload) {
                log_info("memory error");
                LITE_free(rsp_msg->URI);
                LITE_free(node->msg);
                LITE_free(node);
                return;
            }
            memset(rsp_msg->payload, 0x0, mqtt_info->payload_len + 1);
            memcpy(rsp_msg->payload, mqtt_info->payload, mqtt_info->payload_len);   

            rsp_msg->payload_length = mqtt_info->payload_len;
            
            log_info("URI %s", (char*)rsp_msg->URI);
            log_info("payload %s", (char*)rsp_msg->payload);
            
            rc = iotx_cmp_process_list_push(cmp_pt, node);
            
            log_info("URI %s", (char*)rsp_msg->URI);
            log_info("payload %s", (char*)rsp_msg->payload);

            iotx_cmp_process_response_pt rsp_pt = (iotx_cmp_process_response_pt)node->msg;
            log_info("URI %s", (char*)rsp_pt->URI);
            log_info("payload %s", (char*)rsp_pt->payload);

            if (FAIL_RETURN == rc) {
                LITE_free(rsp_msg->URI);  
                LITE_free(rsp_msg->payload);
                LITE_free(node->msg);
                LITE_free(node);
                break;
            }
        }
        #else
            iotx_mqtt_topic_info_pt mqtt_info = (iotx_mqtt_topic_info_pt)msg->msg;
            iotx_cmp_process_response_t response;
            
            response.URI = LITE_malloc(mqtt_info->topic_len + 1);
            if (NULL == response.URI) {
                return;
            }
            memset(response.URI, 0x0, mqtt_info->topic_len + 1);
            strncpy(response.URI, mqtt_info->ptopic, mqtt_info->topic_len);
            response.payload = (char*)mqtt_info->payload;
            response.payload_length = mqtt_info->payload_len;
            iotx_cmp_mqtt_direct_response_handler(cmp_pt, &response);
            LITE_free(response.URI);
        #endif  
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

int iotx_cmp_mqtt_direct_disconnect_handler(iotx_cmp_conntext_pt cmp_pt)
{
    iotx_cmp_event_msg_t event;

    if (NULL == cmp_pt) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    event.event_id = IOTX_CMP_EVENT_CLOUD_DISCONNECT;
    event.msg = NULL;
    
    log_info("MQTT disconnect.");
    
    if (cmp_pt->event_func)
        cmp_pt->event_func(cmp_pt, &event, cmp_pt->user_data);
    
    return SUCCESS_RETURN;
}

int iotx_cmp_mqtt_direct_reconnect_handler(iotx_cmp_conntext_pt cmp_pt)
{    
    iotx_cmp_event_msg_t event;

    if (NULL == cmp_pt) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    event.event_id = IOTX_CMP_EVENT_CLOUD_RECONNECT;
    event.msg = NULL;
    
    log_info("MQTT reconnect.");
    
    if (cmp_pt->event_func)
        cmp_pt->event_func(cmp_pt, &event, cmp_pt->user_data);
    return SUCCESS_RETURN;

}


int iotx_cmp_mqtt_direct_register_handler(iotx_cmp_conntext_pt cmp_pt, char* URI, int result, int is_register)
{         
    iotx_cmp_event_msg_t event;
    iotx_cmp_event_result_t result_pt = {0};

    if (NULL == cmp_pt) {
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
    
    log_info("subscribe result.");
    
    if (cmp_pt->event_func)
        cmp_pt->event_func(cmp_pt, &event, cmp_pt->user_data);

    
    _delete_topic(_find_topic_ex(result_pt.URI));
    
    LITE_free(result_pt.URI);

    return SUCCESS_RETURN;
}

int iotx_cmp_mqtt_direct_response_handler(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_process_response_pt response)
{
    iotx_cmp_response_info_t msg_rsp;
    
    if (NULL == cmp_pt || NULL == response) {
        log_info("parameter error");
        return FAIL_RETURN;
    }    
    
    log_info("URI %s", (char*)response->URI);
    log_info("payload %s", (char*)response->payload);

    memset(&msg_rsp, 0x0, sizeof(iotx_cmp_response_info_t));  
    msg_rsp.URI = response->URI;
    msg_rsp.payload = response->payload;
    msg_rsp.payload_length = response->payload_length;
    
    if (cmp_pt->response_func)
        cmp_pt->response_func(cmp_pt, &msg_rsp); 

   
    return SUCCESS_RETURN;
}


void* iotx_cmp_mqtt_direct_init(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_init_param_pt pparam)
{
    void *pclient;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;

    if (NULL == cmp_pt || NULL == pparam) {
        log_info("parameter error");
        return NULL;
    }

    mqtt_pt = LITE_malloc(sizeof(iotx_cmp_mqtt_direct_t));
    if (NULL == mqtt_pt){
        log_info("not enough memory");
        mqtt_pt = NULL;
        return NULL;
    }
    memset(mqtt_pt, 0x0, sizeof(iotx_cmp_mqtt_direct_t));

    if (NULL == (mqtt_pt->msg_buf = (char *)LITE_malloc(MQTT_MSGLEN))) {
        LITE_free(mqtt_pt);
        log_info("memory error");
        mqtt_pt = NULL;
        return NULL;
    }
    memset(mqtt_pt->msg_buf, 0x0, MQTT_MSGLEN);

    if (NULL == (mqtt_pt->msg_readbuf = (char *)LITE_malloc(MQTT_MSGLEN))) {
        log_info("memory error");
        LITE_free(mqtt_pt->msg_buf);
        LITE_free(mqtt_pt);
        mqtt_pt = NULL;
        return NULL;
    }
    memset(mqtt_pt->msg_readbuf, 0x0, MQTT_MSGLEN);    

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(pparam->product_key, pparam->device_name, pparam->device_secret, (void **)&pconn_info)) {
        log_info("AUTH request failed!");
        LITE_free(mqtt_pt->msg_buf);
        LITE_free(mqtt_pt->msg_readbuf);
        LITE_free(mqtt_pt);
        mqtt_pt = NULL;
        return NULL;
    }

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = mqtt_pt->msg_readbuf;
    mqtt_params.read_buf_size = MQTT_MSGLEN;
    mqtt_params.pwrite_buf = mqtt_pt->msg_buf;
    mqtt_params.write_buf_size = MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp = iotx_cmp_mqtt_direct_event_callback;
    mqtt_params.handle_event.pcontext = cmp_pt;

    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        log_info("MQTT construct failed!");
        LITE_free(mqtt_pt->msg_buf);
        LITE_free(mqtt_pt->msg_readbuf);
        LITE_free(mqtt_pt);
        mqtt_pt = NULL;
        return NULL;
    }
    
    return pclient;
}

int iotx_cmp_mqtt_direct_register(iotx_cmp_conntext_pt cmp_pt, const char* topic_filter)
{
    int rc = 0;

    if (NULL == cmp_pt || NULL == topic_filter) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    rc = IOT_MQTT_Subscribe(cmp_pt->cloud_connection, 
                        topic_filter, 
                        IOTX_MQTT_QOS1, 
                        iotx_cmp_mqtt_direct_event_callback, 
                        (void*)cmp_pt);
                        
    if (rc > 0)
        _add_topic(topic_filter, rc);

    return rc;
}


int iotx_cmp_mqtt_direct_unregister(iotx_cmp_conntext_pt cmp_pt, const char* topic_filter)
{
    int rc = 0;

    if (NULL == cmp_pt || NULL == topic_filter) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    rc = IOT_MQTT_Unsubscribe(cmp_pt->cloud_connection, topic_filter);
                        
    if (rc > 0)
        _add_topic(topic_filter, rc);

    return rc;
}


int iotx_cmp_mqtt_direct_send(iotx_cmp_conntext_pt cmp_pt, const char* topic_filter, const void* payload, int payload_length)
{
    int rc = 0;
    iotx_mqtt_topic_info_t topic_msg; 
    
    if (NULL == cmp_pt || NULL == topic_filter || NULL == payload) {
        log_info("parameter error");
        return FAIL_RETURN;
    }
    
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));

    topic_msg.dup = 0;
    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.payload_len = payload_length;
    topic_msg.payload = payload;
    topic_msg.ptopic = topic_filter;
    topic_msg.topic_len = strlen(topic_filter);
    
    rc = IOT_MQTT_Publish(cmp_pt->cloud_connection, topic_filter, &topic_msg);

    return rc;
}

int iotx_cmp_mqtt_direct_send_sync(iotx_cmp_conntext_pt cmp_pt, const char* topic_filter, const void* payload, int payload_length)
{
    return FAIL_RETURN;
}


int iotx_cmp_mqtt_direct_yield(iotx_cmp_conntext_pt cmp_pt, int timeout_ms)
{
    if (NULL == cmp_pt) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    return IOT_MQTT_Yield(cmp_pt->cloud_connection, timeout_ms);
}

int iotx_cmp_mqtt_direct_deinit(iotx_cmp_conntext_pt cmp_pt)
{    
    if (NULL == cmp_pt)
        return FAIL_RETURN;

    if (mqtt_pt->list_length != 0)      
        _delete_all();

    if (mqtt_pt->msg_buf)
        LITE_free(mqtt_pt->msg_buf);

    if (mqtt_pt->msg_readbuf)
        LITE_free(mqtt_pt->msg_readbuf);

    if (mqtt_pt)
        LITE_free(mqtt_pt);

    mqtt_pt = NULL;    
    
    return IOT_MQTT_Destroy(&(cmp_pt->cloud_connection));
}

#endif /* CMP_VIA_MQTT_DIRECT */


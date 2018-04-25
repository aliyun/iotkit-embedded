#ifdef CMP_VIA_MQTT_DIRECT

#include "iot_import.h"

#include "utils_list.h"
#include "lite-utils.h"
#include "lite-system.h"
#include "iot_export.h"
#include "iotx_cmp_common.h"
#include "iot_export_mqtt.h"

#include "mqtt_instance.h"
#include "iotx_cmp_mqtt_direct.h"


#ifdef ESP8266
#define MQTT_MSGLEN             (1024)
#else
#define MQTT_MSGLEN             (1024 * 40)
#endif


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

    new_pt = CMP_malloc(sizeof(iotx_cmp_mqtt_direct_topic_t));
    if (NULL == new_pt) {
        CMP_ERR(cmp_log_error_memory);
        return FAIL_RETURN;
    }
    memset(new_pt, 0x0, sizeof(iotx_cmp_mqtt_direct_topic_t));

    new_pt->topic = CMP_malloc(strlen(topic) + 1);
    if (NULL == new_pt->topic) {
        CMP_ERR(cmp_log_error_memory);
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
        CMP_WARNING(cmp_log_warning_no_list);
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

    if (NULL == mqtt_pt || NULL == mqtt_pt->topic_list) {
        CMP_WARNING(cmp_log_warning_no_list);
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
        CMP_WARNING(cmp_log_warning_no_list);
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
        CMP_WARNING(cmp_log_warning_no_list);
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
        CMP_ERR(cmp_log_error_parameter);
        return;
    }

    CMP_INFO(cmp_log_info_event_type, msg->event_type);

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            CMP_WARNING(cmp_log_warning_not_arrived);
            break;

        case IOTX_MQTT_EVENT_DISCONNECT: {
        #ifdef CMP_SUPPORT_MULTI_THREAD
        {
            /* send message to itself thread */
            int rc = 0;
            iotx_cmp_process_list_node_pt node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) return;

            node->type = IOTX_CMP_PROCESS_CLOUD_DISCONNECT;
            node->msg = NULL;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);

            if (FAIL_RETURN == rc) iotx_cmp_free_list_node(node);
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
            iotx_cmp_process_list_node_pt node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) return;

            node->type = IOTX_CMP_PROCESS_CLOUD_RECONNECT;
            node->msg = NULL;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);

            if (FAIL_RETURN == rc) iotx_cmp_free_list_node(node);
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
            node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) return;

            node->type = IOTX_CMP_PROCESS_CLOUD_REGISTER_RESULT;
            node->msg = CMP_malloc(sizeof(iotx_cmp_process_register_result_t));
            if (NULL == node->msg) {
                CMP_ERR(cmp_log_error_memory);
                iotx_cmp_free_list_node(node);
                return;
            }

            result_msg = node->msg;

            result_msg->URI = _find_topic(packet_id);
            result_msg->result = 0;
            result_msg->is_register = 1;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);

            if (FAIL_RETURN == rc) {
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
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

            node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) return;

            node->type = IOTX_CMP_PROCESS_CLOUD_REGISTER_RESULT;
            node->msg = CMP_malloc(sizeof(iotx_cmp_process_register_result_t));
            if (NULL == node->msg) {
                CMP_ERR(cmp_log_error_memory);
                iotx_cmp_free_list_node(node);
                return;
            }

            result_msg = node->msg;

            result_msg->URI = _find_topic(packet_id);
            result_msg->result = -1;
            result_msg->is_register = 1;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);

            if (FAIL_RETURN == rc) {
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
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

            node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) return;

            node->type = IOTX_CMP_PROCESS_CLOUD_UNREGISTER_RESULT;
            node->msg = CMP_malloc(sizeof(iotx_cmp_process_register_result_t));
            if (NULL == node->msg) {
                CMP_ERR(cmp_log_error_memory);
                iotx_cmp_free_list_node(node);
                return;
            }

            result_msg = node->msg;

            result_msg->URI = _find_topic(packet_id);
            result_msg->result = 0;
            result_msg->is_register = 0;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);

            if (FAIL_RETURN == rc) {
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
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

            node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) return;

            node->type = IOTX_CMP_PROCESS_CLOUD_UNREGISTER_RESULT;
            node->msg = CMP_malloc(sizeof(iotx_cmp_process_register_result_t));
            if (NULL == node->msg) {
                CMP_ERR(cmp_log_error_memory);
                iotx_cmp_free_list_node(node);
                return;
            }

            result_msg = node->msg;

            result_msg->URI = _find_topic(packet_id);
            result_msg->result = -1;
            result_msg->is_register = 0;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);

            if (FAIL_RETURN == rc) {
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
            }
        }
        #else
            iotx_cmp_mqtt_direct_register_handler(cmp_pt, _find_topic(packet_id), -1, 0);
            _delete_topic(packet_id);
        #endif
        }
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            break;
        
        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:{
        #ifdef CMP_SUPPORT_MULTI_THREAD
        {
            /* send message to itself thread */
            int rc = 0;
            iotx_mqtt_topic_info_pt mqtt_info = (iotx_mqtt_topic_info_pt)msg->msg;
            iotx_cmp_process_list_node_pt node = NULL;
            iotx_cmp_message_info_pt msg_info = NULL;
        #ifdef CMP_SUPPORT_TOPIC_DISPATCH
            iotx_cmp_mapping_pt mapping = NULL;
        #endif

            node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) return;

            node->type = IOTX_CMP_PROCESS_CLOUD_NEW_DATA;
            node->msg = CMP_malloc(sizeof(iotx_cmp_message_info_t));
            if (NULL == node->msg) {
                CMP_ERR(cmp_log_error_memory);
                iotx_cmp_free_list_node(node);
                return;
            }

			msg_info = node->msg;

            msg_info->URI = CMP_malloc(mqtt_info->topic_len + 1);
            if (NULL == msg_info->URI) {
                CMP_ERR(cmp_log_error_memory);
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
                return;
            }
            if (FAIL_RETURN == iotx_cmp_parse_URI((char*)mqtt_info->ptopic, mqtt_info->topic_len, msg_info->URI, &msg_info->URI_type)) {
                CMP_ERR(cmp_log_error_parse_URI);
                iotx_cmp_free_message_info(msg_info);
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
                return;
            }

        #ifdef CMP_SUPPORT_TOPIC_DISPATCH
            /* find mapping */
            mapping = iotx_cmp_find_mapping(cmp_pt, (char*)mqtt_info->ptopic, mqtt_info->topic_len);
            if (NULL == mapping) {
                CMP_ERR(cmp_log_warning_not_mapping);
                iotx_cmp_free_message_info(msg_info);
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
                return;
            }
            msg_info->message_type = mapping->type;
        #endif

			if (FAIL_RETURN == iotx_cmp_parse_payload((char*)mqtt_info->payload, mqtt_info->payload_len, msg_info)) {
                CMP_ERR(cmp_log_error_parse_payload);
                iotx_cmp_free_message_info(msg_info);
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
				return;
			}

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);

            if (FAIL_RETURN == rc) {
                iotx_cmp_free_message_info(msg_info);
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
                break;
            }
        }
        #else
            iotx_mqtt_topic_info_pt mqtt_info = (iotx_mqtt_topic_info_pt)msg->msg;
            iotx_cmp_message_info_t message_info;
        #ifdef CMP_SUPPORT_TOPIC_DISPATCH
            iotx_cmp_mapping_pt mapping = NULL;
        #endif

            memset(&message_info, 0x0, sizeof(iotx_cmp_message_info_t));

            message_info.URI = CMP_malloc(mqtt_info->topic_len + 1);
            if (NULL == message_info.URI) {
                CMP_ERR(cmp_log_error_memory);
                return;
            }
            memset(message_info.URI, 0x0, mqtt_info->topic_len + 1);
            if (FAIL_RETURN == iotx_cmp_parse_URI((char*)mqtt_info->ptopic, mqtt_info->topic_len, message_info.URI, &message_info.URI_type)) {
                CMP_ERR(cmp_log_error_parse_URI);
                iotx_cmp_free_message_info(&message_info);
                return;
            }
            
        #ifdef CMP_SUPPORT_TOPIC_DISPATCH
            /* find mapping */
            mapping = iotx_cmp_find_mapping(cmp_pt, (char*)mqtt_info->ptopic, mqtt_info->topic_len);
            if (NULL == mapping) {
                CMP_ERR(cmp_log_warning_not_mapping);
                iotx_cmp_free_message_info(&message_info);
                return;
            }
            message_info.message_type = mapping->type;
        #endif

			if (FAIL_RETURN == iotx_cmp_parse_payload((char*)mqtt_info->payload, mqtt_info->payload_len, &message_info)) {
                CMP_ERR(cmp_log_error_parse_payload);
                iotx_cmp_free_message_info(&message_info);
				return;
			}

            iotx_cmp_mqtt_direct_response_handler(cmp_pt, &message_info);

        #endif
        }
            break;

        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
            CMP_WARNING(cmp_log_warning_buffer_overflow, msg->msg);
            break;

        default:
            CMP_WARNING(cmp_log_warning_not_arrived);
            break;
    }
}

int iotx_cmp_mqtt_direct_disconnect_handler(iotx_cmp_conntext_pt cmp_pt)
{
    iotx_cmp_event_msg_t event;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    event.event_id = IOTX_CMP_EVENT_CLOUD_DISCONNECT;
    event.msg = NULL;

    CMP_INFO(cmp_log_info_MQTT_disconnect);

    iotx_cmp_trigger_event_callback(cmp_pt, &event);

    return SUCCESS_RETURN;
}

int iotx_cmp_mqtt_direct_reconnect_handler(iotx_cmp_conntext_pt cmp_pt)
{
    iotx_cmp_event_msg_t event;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    event.event_id = IOTX_CMP_EVENT_CLOUD_RECONNECT;
    event.msg = NULL;

    CMP_INFO(cmp_log_info_MQTT_reconnect);

    iotx_cmp_trigger_event_callback(cmp_pt, &event);

    return SUCCESS_RETURN;

}


int iotx_cmp_mqtt_direct_register_handler(iotx_cmp_conntext_pt cmp_pt, char* URI, int result, int is_register)
{
    iotx_cmp_event_msg_t event;
    iotx_cmp_event_result_t result_pt = {0};

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    result_pt.result = result;
    result_pt.URI = CMP_malloc(CMP_TOPIC_LEN_MAX);
    if (NULL == result_pt.URI) {
        CMP_ERR(cmp_log_error_memory);
        return FAIL_RETURN;
    }
    memset(result_pt.URI, 0x0, CMP_TOPIC_LEN_MAX);

    if (FAIL_RETURN == iotx_cmp_parse_URI(URI, CMP_TOPIC_LEN_MAX, result_pt.URI, &result_pt.URI_type)) {
        CMP_ERR(cmp_log_error_parse_URI);
        LITE_free(result_pt.URI);
        return FAIL_RETURN;
    }

    if (is_register)
        event.event_id = IOTX_CMP_EVENT_REGISTER_RESULT;
    else
        event.event_id = IOTX_CMP_EVENT_UNREGISTER_RESULT;

    event.msg = (void*)&result_pt;

    iotx_cmp_trigger_event_callback(cmp_pt, &event);

    _delete_topic(_find_topic_ex(result_pt.URI));

    LITE_free(result_pt.URI);

    return SUCCESS_RETURN;
}

int iotx_cmp_mqtt_direct_response_handler(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_message_info_pt message_info)
{
    if (NULL == cmp_pt || NULL == message_info) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    CMP_INFO(cmp_log_info_URI, (char*)message_info->URI);

    if (cmp_pt->response_func)
        cmp_pt->response_func(cmp_pt, message_info);

    iotx_cmp_free_message_info(message_info);

    return SUCCESS_RETURN;
}

int iotx_cmp_mqtt_direct_connect(void* handler, void* connectivity_pt)
{
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)handler;    
    iotx_mqtt_param_t mqtt_params;
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();
    void *pclient = NULL;

    if (NULL == cmp_pt || NULL == connectivity) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
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
#ifndef MQTT_ID2_AUTH
    pclient = IOT_MQTT_Construct(&mqtt_params);
#else
    pclient = IOT_MQTT_ConstructSecure(&mqtt_params);
#endif /**< MQTT_ID2_AUTH*/
    if (NULL == pclient) {
        CMP_ERR(cmp_log_error_fail);
        connectivity->is_connected = 0;
        return FAIL_RETURN;
    }

    mqtt_set_instance(pclient);

    connectivity->context = pclient;
    connectivity->is_connected = 1;

    return SUCCESS_RETURN;
}


void* iotx_cmp_mqtt_direct_init(void* handler, iotx_cmp_init_param_pt pparam)
{
    iotx_cmp_connectivity_pt connectivity = NULL;
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)handler;

    if (NULL == cmp_pt || NULL == pparam) {
        CMP_ERR(cmp_log_error_parameter);
        return NULL;
    }

    mqtt_pt = CMP_malloc(sizeof(iotx_cmp_mqtt_direct_t));
    if (NULL == mqtt_pt){
        CMP_ERR(cmp_log_error_memory);
        mqtt_pt = NULL;
        return NULL;
    }
    memset(mqtt_pt, 0x0, sizeof(iotx_cmp_mqtt_direct_t));

    if (NULL == (mqtt_pt->msg_buf = (char *)CMP_malloc(MQTT_MSGLEN))) {
        LITE_free(mqtt_pt);
        CMP_ERR(cmp_log_error_memory);
        mqtt_pt = NULL;
        return NULL;
    }
    memset(mqtt_pt->msg_buf, 0x0, MQTT_MSGLEN);

    if (NULL == (mqtt_pt->msg_readbuf = (char *)CMP_malloc(MQTT_MSGLEN))) {
        CMP_ERR(cmp_log_error_memory);
        LITE_free(mqtt_pt->msg_buf);
        LITE_free(mqtt_pt);
        mqtt_pt = NULL;
        return NULL;
    }
    memset(mqtt_pt->msg_readbuf, 0x0, MQTT_MSGLEN);
    
    connectivity = CMP_malloc(sizeof(iotx_cmp_connectivity_t));
    if (NULL == connectivity) {
        CMP_ERR(cmp_log_error_memory);
        iotx_cmp_mqtt_direct_deinit(connectivity);
        return NULL;
    }
    
    iotx_cmp_mqtt_direct_connect(cmp_pt, connectivity);

    connectivity->init_func = iotx_cmp_mqtt_direct_init;
    connectivity->connect_func = iotx_cmp_mqtt_direct_connect;
    connectivity->register_func = iotx_cmp_mqtt_direct_register;
    connectivity->unregister_func = iotx_cmp_mqtt_direct_unregister;
    connectivity->send_func = iotx_cmp_mqtt_direct_send;
    connectivity->send_sync_func = iotx_cmp_mqtt_direct_send_sync;
    connectivity->yield_func = iotx_cmp_mqtt_direct_yield;
    connectivity->deinit_func = iotx_cmp_mqtt_direct_deinit;

    return connectivity;
}

int iotx_cmp_mqtt_direct_register(void* handler, void* connectivity_pt, const char* topic_filter)
{
    int rc = 0;
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)handler;
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;

    if (NULL == connectivity || NULL == topic_filter) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    rc = IOT_MQTT_Subscribe(connectivity->context,
                        topic_filter,
                        IOTX_MQTT_QOS1,
                        iotx_cmp_mqtt_direct_event_callback,
                        (void*)cmp_pt);

    if (rc > 0)
        _add_topic(topic_filter, rc);

    return rc;
}

int iotx_cmp_mqtt_direct_unregister(void* handler, void* connectivity_pt, const char* topic_filter)
{
    int rc = 0;
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;
    if (NULL == connectivity || NULL == topic_filter) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    rc = IOT_MQTT_Unsubscribe(connectivity->context, topic_filter);

    if (rc > 0)
        _add_topic(topic_filter, rc);

    return rc;
}


static iotx_mqtt_qos_t _to_mqtt_qos(iotx_cmp_message_ack_types_t ack_type)
{
    switch (ack_type) {
        case IOTX_CMP_MESSAGE_NEED_ACK:
            return IOTX_MQTT_QOS1;

        case IOTX_CMP_MESSAGE_NO_ACK:
            return IOTX_MQTT_QOS0;

        default:
            return IOTX_MQTT_QOS0;
    }
}



int iotx_cmp_mqtt_direct_send(void* handler,
                    void* connectivity_pt,
                    const char* topic_filter,
                    iotx_cmp_message_ack_types_t ack_type,
                    const void* payload,
                    int payload_length)
{
    int rc = 0;
    iotx_mqtt_topic_info_t topic_msg;
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;

    if (NULL == connectivity || NULL == topic_filter || NULL == payload) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));

    topic_msg.dup = 0;
    topic_msg.qos = _to_mqtt_qos(ack_type);
    topic_msg.retain = 0;
    topic_msg.payload_len = payload_length;
    topic_msg.payload = payload;
    topic_msg.ptopic = topic_filter;
    topic_msg.topic_len = strlen(topic_filter);

    rc = IOT_MQTT_Publish(connectivity->context, topic_filter, &topic_msg);

    return rc;
}

int iotx_cmp_mqtt_direct_send_sync(void* handler,
                    void* connectivity_pt,
                    const char* topic_filter,
                    iotx_cmp_message_ack_types_t ack_type,
                    const void* payload,
                    int payload_length)
{
    return FAIL_RETURN;
}


int iotx_cmp_mqtt_direct_yield(void* connectivity_pt, int timeout_ms)
{
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;
    if (NULL == connectivity) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    return IOT_MQTT_Yield(connectivity->context, timeout_ms);
}

int iotx_cmp_mqtt_direct_deinit(void* connectivity_pt)
{
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;
    int rc = 0;

    if (NULL == connectivity)
        return FAIL_RETURN;

    rc = IOT_MQTT_Destroy(&(connectivity->context));

    if (mqtt_pt->list_length != 0)
        _delete_all();

    if (mqtt_pt->msg_buf)
        LITE_free(mqtt_pt->msg_buf);

    if (mqtt_pt->msg_readbuf)
        LITE_free(mqtt_pt->msg_readbuf);

    if (mqtt_pt)
        LITE_free(mqtt_pt);

    mqtt_pt = NULL;

    LITE_free(connectivity);

    return rc;
}

#endif /* CMP_VIA_MQTT_DIRECT */


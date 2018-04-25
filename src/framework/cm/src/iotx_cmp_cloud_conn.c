#ifdef CMP_VIA_CLOUN_CONN

#include "iot_import.h"

#include "utils_list.h"
#include "lite-utils.h"
#include "lite-system.h"
#include "iot_export.h"
#include "iotx_cmp_common.h"
#include "iotx_cmp_cloud_conn.h"


static void iotx_cmp_cloud_conn_event_callback(void *pcontext, iotx_cloud_connection_event_msg_pt msg)
{
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)pcontext;

    if (NULL == cmp_pt || NULL == msg) {
        CMP_ERR(cmp_log_error_parameter);
        return;
    }

    CMP_INFO(cmp_log_info_event_id, msg->event_id);

    switch (msg->event_id) {
        case IOTX_CLOUD_CONNECTION_EVENT_DISCONNECT: {
        #ifdef CMP_SUPPORT_MULTI_THREAD
        {
            /* send message to itself thread */
            int rc = 0;
            iotx_cmp_process_list_node_pt node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) {
                CMP_ERR(cmp_log_error_get_node);
                return;
            }

            node->type = IOTX_CMP_PROCESS_CLOUD_DISCONNECT;
            node->msg = NULL;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);
            if (FAIL_RETURN == rc) {
                CMP_ERR(cmp_log_error_push_node);
                iotx_cmp_free_list_node(node);
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
            iotx_cmp_process_list_node_pt node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) {
                CMP_ERR(cmp_log_error_get_node);
                return;
            }
            node->type = IOTX_CMP_PROCESS_CLOUD_RECONNECT;
            node->msg = NULL;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);
            if (FAIL_RETURN == rc) {
                CMP_ERR(cmp_log_error_push_node);
                iotx_cmp_free_list_node(node);
            }
        }
        #else
            iotx_cmp_cloud_conn_reconnect_handler(cmp_pt);
        #endif
        }
            break;

        default:
            CMP_WARNING(cmp_log_warning_not_arrived);
            break;
    }
}


static void iotx_cmp_cloud_conn_response_callback(void *pcontext,
                void *pconnection,
                iotx_cloud_connection_msg_rsp_pt msg)
{
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)pcontext;

    if (NULL == cmp_pt || NULL == pconnection || NULL == msg) {
        CMP_ERR(cmp_log_error_parameter);
        return;
    }

    CMP_INFO(cmp_log_info_rsp_type, msg->rsp_type);

    switch (msg->rsp_type) {
        case IOTX_CLOUD_CONNECTION_RESPONSE_SUBSCRIBE_SUCCESS: {
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
            if (NULL == node) {
                CMP_ERR(cmp_log_error_memory);
                iotx_cmp_free_list_node(node);
                return;
            }

            result_msg = node->msg;
            result_msg->URI = CMP_malloc(strlen(msg->URI) + 1);
            if (NULL == result_msg->URI) {
                LITE_free(node->msg);
                LITE_free(node);
                return;
            }
            memset(result_msg->URI, 0x0, strlen(msg->URI) + 1);
            strncpy(result_msg->URI, (msg->URI), strlen(msg->URI) + 1);
            result_msg->result = 1;
            result_msg->is_register = 1;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);
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

            node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) return;

            node->type = IOTX_CMP_PROCESS_CLOUD_REGISTER_RESULT;
            node->msg = CMP_malloc(sizeof(iotx_cmp_process_register_result_t));
            if (NULL == node) {
                CMP_ERR(cmp_log_error_memory);
                iotx_cmp_free_list_node(node);
                return;
            }

            result_msg = node->msg;

            result_msg->URI = CMP_malloc(strlen(msg->URI) + 1);
            if (NULL == result_msg->URI) {
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
                return;
            }
            memset(result_msg->URI, 0x0, strlen(msg->URI) + 1);
            strncpy(result_msg->URI, (msg->URI), strlen(msg->URI) + 1);

            result_msg->result = 0;
            result_msg->is_register = 1;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);

            if (FAIL_RETURN == rc) {
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
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

            node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) return;

            node->type = IOTX_CMP_PROCESS_CLOUD_UNREGISTER_RESULT;
            node->msg = CMP_malloc(sizeof(iotx_cmp_process_register_result_t));
            if (NULL == node) {
                CMP_ERR(cmp_log_error_memory);
                iotx_cmp_free_list_node(node);
                return;
            }

            result_msg = node->msg;

            result_msg->URI = CMP_malloc(strlen(msg->URI) + 1);
            if (NULL == result_msg->URI) {
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
                return;
            }
            memset(result_msg->URI, 0x0, strlen(msg->URI) + 1);
            strncpy(result_msg->URI, (msg->URI), strlen(msg->URI) + 1);
            result_msg->result = 1;
            result_msg->is_register = 0;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);

            if (FAIL_RETURN == rc) {
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
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

            node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
            if (NULL == node) return;

            node->type = IOTX_CMP_PROCESS_CLOUD_UNREGISTER_RESULT;
            node->msg = CMP_malloc(sizeof(iotx_cmp_process_register_result_t));
            if (NULL == node) {
                CMP_ERR(cmp_log_error_memory);
                iotx_cmp_free_list_node(node);
                return;
            }

            result_msg = node->msg;

            result_msg->URI = CMP_malloc(strlen(msg->URI) + 1);
            if (NULL == result_msg->URI) {
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
                return;
            }
            memset(result_msg->URI, 0x0, strlen(msg->URI) + 1);
            strncpy(result_msg->URI, (msg->URI), strlen(msg->URI) + 1);
            result_msg->result = 0;
            result_msg->is_register = 0;

            rc = iotx_cmp_process_list_push(cmp_pt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);

            if (FAIL_RETURN == rc) {
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
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

            msg_info->URI = CMP_malloc(msg->URI_length + 1);
            if (NULL == msg_info->URI) {
                CMP_ERR(cmp_log_error_memory);
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
                return;
            }

            if (FAIL_RETURN == iotx_cmp_parse_URI((char*)msg->URI, msg->URI_length, msg_info->URI, &msg_info->URI_type)) {
                CMP_ERR(cmp_log_error_parse_URI);
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
                return;
            }
            
        #ifdef CMP_SUPPORT_TOPIC_DISPATCH
            /* find mapping */
            mapping = iotx_cmp_find_mapping(cmp_pt, (char*)msg->URI, msg->URI_length);
            if (NULL == mapping) {
                CMP_WARNING(cmp_log_warning_not_mapping);
                LITE_free(node->msg);
                iotx_cmp_free_list_node(node);
                return;
            }
            msg_info->message_type = mapping->type;
        #endif

			if (FAIL_RETURN == iotx_cmp_parse_payload((char*)msg->payload, msg->payload_length, msg_info)) {
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
            iotx_cmp_message_info_t message_info;
        #ifdef CMP_SUPPORT_TOPIC_DISPATCH
            iotx_cmp_mapping_pt mapping = NULL;
        #endif

            memset(&message_info, 0x0, sizeof(iotx_cmp_message_info_t));

            message_info.URI = CMP_malloc(msg->URI_length + 1);
            if (NULL == message_info.URI) {
                CMP_ERR(cmp_log_error_memory);
                return;
            }
            memset(message_info.URI, 0x0, msg->URI_length + 1);
            if (FAIL_RETURN == iotx_cmp_parse_URI((char*)msg->URI, msg->URI_length, message_info.URI, &message_info.URI_type)) {
                CMP_ERR(cmp_log_error_parse_URI);
                iotx_cmp_free_message_info(&message_info);
                return;
            }
            
        #ifdef CMP_SUPPORT_TOPIC_DISPATCH
            /* find mapping */
            mapping = iotx_cmp_find_mapping(cmp_pt, (char*)msg->URI, msg->URI_length);
            if (NULL == mapping) {
                CMP_WARNING(cmp_log_warning_not_mapping);
                iotx_cmp_free_message_info(&message_info);
                return;
            }
            message_info.message_type = mapping->type;
        #endif

			if (FAIL_RETURN == iotx_cmp_parse_payload((char*)msg->payload, msg->payload_length, &message_info)) {
                CMP_ERR(cmp_log_error_parse_payload);
                iotx_cmp_free_message_info(&message_info);
				return;
			}

            iotx_cmp_cloud_conn_response_handler(cmp_pt, &message_info);
        #endif
        }
            break;

        default:
            CMP_WARNING(cmp_log_warning_not_arrived);
            break;
    }
}


int iotx_cmp_cloud_conn_disconnect_handler(iotx_cmp_conntext_pt cmp_pt)
{
    iotx_cmp_event_msg_t event;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    event.event_id = IOTX_CMP_EVENT_CLOUD_DISCONNECT;
    event.msg = NULL;

    CMP_INFO(cmp_log_info_cloud_disconnect);

    iotx_cmp_trigger_event_callback(cmp_pt, &event);

    return SUCCESS_RETURN;
}

int iotx_cmp_cloud_conn_reconnect_handler(iotx_cmp_conntext_pt cmp_pt)
{
    iotx_cmp_event_msg_t event;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    event.event_id = IOTX_CMP_EVENT_CLOUD_RECONNECT;
    event.msg = NULL;

    CMP_INFO(cmp_log_info_cloud_reconnect);

    iotx_cmp_trigger_event_callback(cmp_pt, &event);

    return SUCCESS_RETURN;
}

int iotx_cmp_cloud_conn_register_handler(iotx_cmp_conntext_pt cmp_pt,
                char* URI,
                int result,
                int is_register)
{
    iotx_cmp_event_msg_t event;
    iotx_cmp_event_result_t result_pt = {0};

    if (NULL == cmp_pt || NULL == URI) {
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

    if (FAIL_RETURN == iotx_cmp_parse_URI((char*)URI, strlen(result_pt.URI), result_pt.URI, &result_pt.URI_type)) {
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

    LITE_free(result_pt.URI);

    return SUCCESS_RETURN;
}


int iotx_cmp_cloud_conn_response_handler(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_message_info_pt message_info)
{
    if (NULL == cmp_pt || NULL == message_info) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    CMP_INFO(cmp_log_info_URI, (char*)message_info->URI);

    if (cmp_pt->response_func)
        cmp_pt->response_func(cmp_pt, message_info);

    /*iotx_cmp_free_message_info(message_info);*/

    return SUCCESS_RETURN;
}


void* iotx_cmp_cloud_conn_init(void* handler, iotx_cmp_init_param_pt pparam)
{
    void* pclient = NULL;
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)handler;
    iotx_cloud_connection_param_t cloud_param = {0};
    iotx_cmp_connectivity_pt connectivity = NULL;

    cloud_param.device_info = HAL_Malloc(sizeof(iotx_deviceinfo_t));
    if (NULL == cloud_param.device_info) {
        CMP_ERR(cmp_log_error_parameter);
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

    pclient = IOT_Cloud_Connection_Init(&cloud_param);
    if (NULL == pclient) {
        CMP_ERR(cmp_log_error_fail);
        iotx_cmp_cloud_conn_deinit(connectivity);
    }

    connectivity = CMP_malloc(sizeof(iotx_cmp_connectivity_t));
    if (NULL == connectivity) {
        CMP_ERR(cmp_log_error_memory);
        iotx_cmp_cloud_conn_deinit(connectivity);
        return NULL;
    }

    connectivity->context = pclient;
    connectivity->init_func = iotx_cmp_cloud_conn_init;
    connectivity->register_func = iotx_cmp_cloud_conn_register;
    connectivity->unregister_func = iotx_cmp_cloud_conn_unregister;
    connectivity->send_func = iotx_cmp_cloud_conn_send;
    connectivity->send_sync_func = iotx_cmp_cloud_conn_send_sync;
    connectivity->yield_func = iotx_cmp_cloud_conn_yield;
    connectivity->deinit_func = iotx_cmp_cloud_conn_deinit;

    return connectivity;
}


int iotx_cmp_cloud_conn_register(void* handler, void* connectivity_pt, const char* topic_filter)
{
    int rc = 0;
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)handler;
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;
    iotx_cloud_connection_msg_t msg = {0};

    if (NULL == connectivity || NULL == topic_filter) {
        CMP_ERR(cmp_log_error_parameter);
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
    rc = IOT_Cloud_Connection_Send_Message(connectivity->context, &msg);

    CMP_INFO(cmp_log_info_result, rc);

    return rc;
}


int iotx_cmp_cloud_conn_unregister(void* handler, void* connectivity_pt, const char* topic_filter)
{
    int rc = 0;
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)handler;
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;
    iotx_cloud_connection_msg_t msg = {0};

    if (NULL == connectivity || NULL == topic_filter) {
        CMP_ERR(cmp_log_error_parameter);
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
    rc = IOT_Cloud_Connection_Send_Message(connectivity->context, &msg);
    CMP_INFO(cmp_log_info_result, rc);
    return rc;
}


static iotx_message_qos_t _to_message_qos(iotx_cmp_message_ack_types_t ack_type)
{
    switch (ack_type) {
        case IOTX_CMP_MESSAGE_NEED_ACK:
            return IOTX_MESSAGE_QOS1;

        case IOTX_CMP_MESSAGE_NO_ACK:
            return IOTX_MESSAGE_QOS0;

        default:
            return IOTX_MESSAGE_QOS0;
    }
}


int iotx_cmp_cloud_conn_send(void* handler,
                    void* connectivity_pt,
                    const char* topic_filter,
                    iotx_cmp_message_ack_types_t ack_type,
                    const void* payload,
                    int payload_length)
{
    int rc = 0;
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)handler;
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;
    iotx_cloud_connection_msg_t msg = {0};

    if (NULL == connectivity || NULL == topic_filter || NULL == payload) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    msg.type = IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_PUBLISH;
    msg.QoS = _to_message_qos(ack_type);
    msg.URI = (char*)topic_filter;
    msg.URI_length = strlen(topic_filter);
    msg.payload = (char*)payload;
    msg.payload_length = payload_length;
    msg.response_handler = iotx_cmp_cloud_conn_response_callback;
    msg.response_pcontext = cmp_pt;
    rc = IOT_Cloud_Connection_Send_Message(connectivity->context, &msg);
    CMP_INFO(cmp_log_info_result, rc);

    return rc;
}


int iotx_cmp_cloud_conn_send_sync(void* handler,
                    void* connectivity_pt,
                    const char* topic_filter,
                    iotx_cmp_message_ack_types_t ack_type,
                    const void* payload,
                    int payload_length)
{
    return FAIL_RETURN;
}

int iotx_cmp_cloud_conn_yield(void* connectivity_pt, int timeout_ms)
{
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;

    if (NULL == connectivity) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    return IOT_Cloud_Connection_Yield(connectivity->context, timeout_ms);
}

int iotx_cmp_cloud_conn_deinit(void* connectivity_pt)
{
    int rc = 0;
    iotx_cmp_connectivity_pt connectivity = (iotx_cmp_connectivity_pt)connectivity_pt;

    if (NULL == connectivity)
        return FAIL_RETURN;

    rc = IOT_Cloud_Connection_Deinit(&(connectivity->context));

    LITE_free(connectivity);

    return rc;
}

#endif


#include "iot_import.h"

#include "utils_list.h"
#include "lite-utils.h"
#include "lite-system.h"
#include "iot_export.h"
#include "iotx_cmp_common.h"
#include "iotx_cmp_mqtt_direct.h"
#include "iotx_cmp_cloud_conn.h"
#include "iotx_cmp_ota.h"



static iotx_cmp_conntext_pt g_cmp_cnt = NULL;

extern void guider_set_domain_type(int domain_type);


/**
 * @brief CMP init
 *        This function initialize the CMP structures, establish network connection
 *        If CMP has been initialized, this function will return success directly.
 *
 * @param pparam, specify the cmp and event handler.
 * @param option, reserve.
 *
 * @return success or fail.
 */
int IOT_CMP_Init(iotx_cmp_init_param_pt pparam, void* option)
{
    iotx_conn_info_pt pconn_info;
    iotx_cmp_event_callback_list_pt event_callback_list = NULL;
    iotx_cmp_event_callback_list_pt event_callback = NULL;
    iotx_cmp_event_callback_node_pt node = NULL;
    iotx_cmp_event_handle_func_fpt user_callback = NULL;
    void* user_data = NULL;

    if (NULL == pparam) return FAIL_RETURN;

    if (NULL == g_cmp_cnt) {
        char product_key[PRODUCT_KEY_LEN + 1] = {0};
#ifdef SUPPORT_PRODUCT_SECRET
        char product_secret[PRODUCT_SECRET_LEN + 1] = {0};
#endif
        char device_name[DEVICE_NAME_LEN + 1] = {0};
        char device_secret[DEVICE_SECRET_LEN + 1] = {0};
        char device_id[DEVICE_ID_LEN + 1] = {0};

        HAL_GetProductKey(product_key);
        if (strlen(product_key) == 0) {
            CMP_ERR(cmp_log_error_pk);
            return FAIL_RETURN;
        }

        HAL_GetDeviceName(device_name);
        if (strlen(device_name) == 0) {
            CMP_ERR(cmp_log_error_dn);
            return FAIL_RETURN;
        }

        HAL_GetDeviceID(device_id);
        if (strlen(device_id) == 0) {
            CMP_ERR(cmp_log_error_di);
            return FAIL_RETURN;
        }

#ifdef SUPPORT_PRODUCT_SECRET
        /* 一型一密 */
        if (IOTX_CMP_DEVICE_SECRET_PRODUCT == pparam->secret_type && 0 >= HAL_GetDeviceSecret(device_secret)) {

            HAL_GetProductSecret(product_secret);
            if (strlen(product_secret) == 0) {
                CMP_ERR(cmp_log_error_secret_1);
                return FAIL_RETURN;
            }

            /* auth */
            if (FAIL_RETURN == iotx_cmp_auth(product_key, device_name, device_id)) {
                CMP_ERR(cmp_log_error_auth);
                return FAIL_RETURN;
            }
        }
#endif /**< SUPPORT_PRODUCT_SECRET*/

        HAL_GetDeviceSecret(device_secret);
        if (strlen(device_secret) == 0) {
            CMP_ERR(cmp_log_error_secret_2);
            return FAIL_RETURN;
        }
		

        /* Device AUTH */
#ifndef MQTT_ID2_AUTH
	if (0 != IOT_SetupConnInfo(product_key, device_name, device_secret, (void **)&pconn_info)) {
#else
	if (0 != IOT_SetupConnInfoSecure(product_key, device_name, device_secret, (void **)&pconn_info)) {
#endif /**< MQTT_ID2_AUTH*/
			CMP_ERR(cmp_log_error_auth);
            return FAIL_RETURN;
        }

        CMP_INFO(cmp_log_info_init);
        g_cmp_cnt = CMP_malloc(sizeof(iotx_cmp_conntext_t));
        if (NULL == g_cmp_cnt) {
            CMP_ERR(cmp_log_error_memory);
            return FAIL_RETURN;
        }
        memset(g_cmp_cnt, 0x0, sizeof(iotx_cmp_conntext_t));

        g_cmp_cnt->response_func = iotx_cmp_response_func;
        guider_set_domain_type(pparam->domain_type);
    }

    user_callback = pparam->event_func;
    user_data = pparam->user_data;

    event_callback = CMP_malloc(sizeof(iotx_cmp_event_callback_list_t));
    if (NULL == event_callback) {
        CMP_ERR(cmp_log_error_memory);
        goto exit;
    }

    node = CMP_malloc(sizeof(iotx_cmp_event_callback_node_t));
    if (NULL == node) {
        CMP_ERR(cmp_log_error_memory);
        goto exit;
    }

    node->event_func = user_callback;
    node->user_data = user_data;
    event_callback->node = node;
    event_callback->next = NULL;

    if(NULL == g_cmp_cnt->event_callback_list) {
        g_cmp_cnt->event_callback_list = event_callback;
        if (FAIL_RETURN == iotx_cmp_add_connectivity_all(g_cmp_cnt, pparam)) goto exit;

        /* create thread */
    #ifdef CMP_SUPPORT_MULTI_THREAD
        g_cmp_cnt->thread_is_stoped = 0;

        g_cmp_cnt->process_cloud_lock = HAL_MutexCreate();
        if (NULL == g_cmp_cnt->process_cloud_lock) goto exit;

    #ifdef CMP_SUPPORT_MULTI_THREAD_VIA_HAL
        if (0 != HAL_ThreadCreate(&g_cmp_cnt->pthread_process, iotx_cmp_cloud_process, NULL, NULL, NULL)) goto exit;

    #else
        if (0 != pthread_create(&g_cmp_cnt->pthread_process, NULL, iotx_cmp_cloud_process, (void*)g_cmp_cnt)) goto exit;
    #endif

    #ifdef CMP_SUPPORT_LOCAL_CONN
        g_cmp_cnt->process_local_lock = HAL_MutexCreate();
        if (NULL == g_cmp_cnt->process_local_lock) goto exit;

    #ifdef CMP_SUPPORT_MULTI_THREAD_VIA_HAL
        if (0 != HAL_ThreadCreate(&g_cmp_cnt->pthread_process, iotx_cmp_local_process, NULL, NULL, NULL)) goto exit;

    #else
        if (0 != pthread_create(&g_cmp_cnt->pthread_process, NULL, iotx_cmp_local_process, (void*)g_cmp_cnt)) goto exit;
    #endif
    #endif

    #endif
    } else {
        event_callback_list = g_cmp_cnt->event_callback_list;
        while(event_callback_list != NULL && event_callback_list->next) {
             event_callback_list = event_callback_list->next;
        }
        event_callback_list->next = event_callback;
    }

    if (g_cmp_cnt->connectivity_list &&
        g_cmp_cnt->connectivity_list->node &&
        g_cmp_cnt->connectivity_list->node->is_connected) {
        iotx_cmp_event_msg_t event_msg = {0};
        event_msg.event_id = IOTX_CMP_EVENT_CLOUD_CONNECTED;
        event_msg.msg = NULL;
        user_callback(g_cmp_cnt, &event_msg, user_data);
    }

    return SUCCESS_RETURN;

exit:
    CMP_ERR(cmp_log_error_fail);
    iotx_cmp_remove_connectivity_all(g_cmp_cnt);

#ifdef SERVICE_OTA_ENABLED
    if (g_cmp_cnt->ota_handler) iotx_cmp_ota_deinit(g_cmp_cnt);
#endif /* SERVICE_OTA_ENABLED */

#ifdef CMP_SUPPORT_MULTI_THREAD
    if (g_cmp_cnt->process_cloud_lock) HAL_MutexDestroy(g_cmp_cnt->process_cloud_lock);
#ifdef CMP_SUPPORT_LOCAL_CONN
    if (g_cmp_cnt->process_local_lock) HAL_MutexDestroy(g_cmp_cnt->process_local_lock);
#endif
#endif

    if (node) LITE_free(node);

    if (event_callback) LITE_free(event_callback);

    if (g_cmp_cnt) LITE_free(g_cmp_cnt);

    g_cmp_cnt = 0;

    return FAIL_RETURN;
}

#ifdef SERVICE_OTA_ENABLED
int IOT_CMP_OTA_Start(char* cur_version, void* option)
{
    if (NULL == g_cmp_cnt) {
        CMP_ERR(cmp_log_error_status);
        return FAIL_RETURN;
    }

    if (NULL == cur_version) CMP_ERR(cmp_log_error_parameter);

    if (g_cmp_cnt->connectivity_list &&
        g_cmp_cnt->connectivity_list->node &&
        0 == g_cmp_cnt->connectivity_list->node->is_connected) {
        CMP_WARNING(cmp_log_warning_cloud_disconnected);
        return FAIL_RETURN;
    }

    /* ota init */
	if (NULL == g_cmp_cnt->ota_handler)
        g_cmp_cnt->ota_handler = iotx_cmp_ota_init(g_cmp_cnt, cur_version);
    if (NULL == g_cmp_cnt->ota_handler) return FAIL_RETURN;

    return SUCCESS_RETURN;
}


int IOT_CMP_OTA_Set_Callback(iotx_cmp_ota_types_t type, void* ota_func, void* user_context, void* option)
{
    void** _ota_func = NULL;
    void** _user_context = NULL;

    if (NULL == g_cmp_cnt) {
        CMP_ERR(cmp_log_error_status);
        return FAIL_RETURN;
    }

    if (NULL == ota_func) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    switch(type) {
    case IOTX_CMP_OTA_TYPE_FOTA:
        _ota_func = (void**)&g_cmp_cnt->fota_func;
        _user_context = (void**)&g_cmp_cnt->fota_user_context;
        break;

    case IOTX_CMP_OTA_TYPE_COTA:
        _ota_func = (void**)&g_cmp_cnt->cota_func;
        _user_context = (void**)&g_cmp_cnt->cota_user_context;
        break;

    default:
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
        break;
    }

    if (*_ota_func) {
        CMP_WARNING(cmp_log_warning_ota_started);
        return FAIL_RETURN;
    }

    if (g_cmp_cnt->connectivity_list &&
        g_cmp_cnt->connectivity_list->node &&
        0 == g_cmp_cnt->connectivity_list->node->is_connected) {
        CMP_WARNING(cmp_log_warning_cloud_disconnected);
        return FAIL_RETURN;
    }

    *_ota_func = ota_func;
    *_user_context = user_context;

    return SUCCESS_RETURN;
}


int IOT_CMP_OTA_Get_Config(const char* configScope, const char* getType, const char* attributeKeys, void* option)
{
    if (NULL == g_cmp_cnt || NULL == g_cmp_cnt->ota_handler) {
        CMP_ERR(cmp_log_error_status);
        return FAIL_RETURN;
    }

    return iotx_cmp_ota_get_config(g_cmp_cnt, configScope, getType, attributeKeys);
}


int IOT_CMP_OTA_Request_Image(const char* version, void* option)
{
    if (NULL == g_cmp_cnt || NULL == g_cmp_cnt->ota_handler) {
        CMP_ERR(cmp_log_error_status);
        return FAIL_RETURN;
    }

    return iotx_cmp_ota_request_image(g_cmp_cnt, version);
}
#endif /* SERVICE_OTA_ENABLED */


/**
 * @brief Register service.
 *        This function used to register some service by different URI, set the URI's callback.
 *        If it is received a request, will callback the register_cb.
 *        If there is no match register_cb (user have not register the service set callback), the request will be discard.
 *
 * @param pparam, register parameter, include URI and register callback.
 * @param option, reserve.
 *
 * @return success or fail.
 *          This API not support one URI register twice, if the URI have been register, it will return fail.
 */
int IOT_CMP_Register(iotx_cmp_register_param_pt pparam, void* option)
{
    char* URI = NULL;
    int rc = 0;
    int length = CMP_TOPIC_LEN_MAX;

    if (NULL == g_cmp_cnt) {
        CMP_ERR(cmp_log_error_status);
        return FAIL_RETURN;
    }

    if (NULL == pparam || NULL == pparam->URI) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    if (pparam->URI_type > IOTX_CMP_URI_UNDEFINE || pparam->URI_type < IOTX_CMP_URI_SYS) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    if (pparam->message_type > IOTX_CMP_MESSAGE_RESPONSE || pparam->message_type < IOTX_CMP_MESSAGE_RAW) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    /* splice URI */
    URI = CMP_malloc(CMP_TOPIC_LEN_MAX);
    if (NULL == URI){
        CMP_ERR(cmp_log_error_memory);
        return FAIL_RETURN;
    }
    memset(URI, 0x0, CMP_TOPIC_LEN_MAX);

    if (FAIL_RETURN == iotx_cmp_splice_URI(URI, &length, pparam->URI, pparam->URI_type)) {
        CMP_ERR(cmp_log_error_splice_URI);
        return FAIL_RETURN;
    }

    if (NULL != iotx_cmp_find_mapping(g_cmp_cnt, URI, length)) {
        CMP_INFO(cmp_log_info_registered);
        LITE_free(URI);
        return FAIL_RETURN;
    }

#ifdef CMP_SUPPORT_MULTI_THREAD
{
    /* send message to itself thread */
    iotx_cmp_process_list_node_pt node = NULL;
    iotx_cmp_process_register_pt msg = NULL;

    node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
    if (NULL == node) {
        CMP_ERR(cmp_log_error_get_node);
        LITE_free(URI);
        return FAIL_RETURN;
    }

    node->type = IOTX_CMP_PROCESS_CLOUD_REGISTER;
    node->msg = CMP_malloc(sizeof(iotx_cmp_process_register_t));
    if (NULL == node->msg) {
        CMP_ERR(cmp_log_error_memory);
        LITE_free(URI);
        iotx_cmp_free_list_node(node);
        return FAIL_RETURN;
    }

    msg = node->msg;
    msg->URI = URI;
    msg->type = pparam->message_type;
    msg->register_func = pparam->register_func;
    msg->user_data = pparam->user_data;
    msg->mail_box = pparam->mail_box;

    rc = iotx_cmp_process_list_push(g_cmp_cnt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);
    if (FAIL_RETURN == rc) {
        CMP_ERR(cmp_log_error_push_node);
        LITE_free(URI);
        LITE_free(node->msg);
        iotx_cmp_free_list_node(node);
    }
}
#else /* CMP_SUPPORT_MULTI_THREAD */
    rc = iotx_cmp_register_service(g_cmp_cnt, URI, pparam->message_type, pparam->register_func, pparam->user_data, pparam->mail_box);
    LITE_free(URI);
#endif /* CMP_SUPPORT_MULTI_THREAD */

    return rc;
}


/**
 * @brief Unregister service.
 *        This function used to unregister some service by different URI
 *
 * @param pparam, unregister parameter, include URI.
 * @param option, reserve.
 *
 * @return success or fail.
 */
int IOT_CMP_Unregister(iotx_cmp_unregister_param_pt pparam, void* option)
{
    char* URI = NULL;
    int rc = 0;
    int length = CMP_TOPIC_LEN_MAX;

    if (NULL == g_cmp_cnt) {
        CMP_ERR(cmp_log_error_status);
        return FAIL_RETURN;
    }

    if (NULL == pparam || NULL == pparam->URI) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    /* splice URI */
    URI = CMP_malloc(CMP_TOPIC_LEN_MAX);
    if (NULL == URI){
        CMP_ERR(cmp_log_error_memory);
        return FAIL_RETURN;
    }
    memset(URI, 0x0, CMP_TOPIC_LEN_MAX);

    if (FAIL_RETURN == iotx_cmp_splice_URI(URI, &length, pparam->URI, pparam->URI_type)) {
        CMP_ERR(cmp_log_error_splice_URI);
        return FAIL_RETURN;
    }

#ifdef CMP_SUPPORT_MULTI_THREAD
{
    /* send message to itself thread */
    iotx_cmp_process_list_node_pt node = NULL;

    node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
    if (NULL == node) {
        CMP_ERR(cmp_log_error_get_node);
        LITE_free(URI);
        return FAIL_RETURN;
    }

    node->type = IOTX_CMP_PROCESS_CLOUD_UNREGISTER;
    node->msg = (void*)URI;

    rc = iotx_cmp_process_list_push(g_cmp_cnt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);
    if (FAIL_RETURN == rc) {
        CMP_ERR(cmp_log_error_push_node);
        LITE_free(URI);
        iotx_cmp_free_list_node(node);
    }
}

#else /* CMP_SUPPORT_MULTI_THREAD */
    rc = iotx_cmp_unregister_service(g_cmp_cnt, URI);
    LITE_free(URI);
#endif /* CMP_SUPPORT_MULTI_THREAD */

    return rc;
}


/**
 * @brief Send data.
 *        This function used to send data to target.
 *        If the target is NULL, the data will broadcast to all the reachable places.
 *        If the target is not NULL, the data will send to target only.
 *        If the target's product_key and device_name is itself, the data will send to cloud.
 *
 * @param target.
 * @param message_info.
 * @param option, reserve.
 *
 * @return success or fail.
 */
int IOT_CMP_Send(iotx_cmp_send_peer_pt target, iotx_cmp_message_info_pt message_info, void* option)
{
    int rc = 0;
    char* URI;
    void* payload;
    int payload_length = 0;

    if (NULL == g_cmp_cnt) {
        CMP_ERR(cmp_log_error_status);
        return FAIL_RETURN;
    }

    if (NULL == message_info || NULL == message_info->URI) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

#ifdef CMP_SUPPORT_MULTI_THREAD
{
    /* send message to itself thread */
    iotx_cmp_process_list_node_pt node = NULL;
    iotx_cmp_process_send_pt msg = NULL;

    node = iotx_cmp_get_list_node(IOTX_CMP_PROCESS_TYPE_CLOUD);
    if (NULL == node) {
        CMP_ERR(cmp_log_error_get_node);
        return FAIL_RETURN;
    }

    node->type = IOTX_CMP_PROCESS_CLOUD_SEND;
    node->msg = CMP_malloc(sizeof(iotx_cmp_process_send_t));
    if (NULL == node->msg) {
        CMP_ERR(cmp_log_error_memory);
        iotx_cmp_free_list_node(node);
        return FAIL_RETURN;
    }

    msg = node->msg;
    if (target) {
        msg->target = CMP_malloc(sizeof(iotx_cmp_send_peer_t));
        memset(msg->target, 0x0, sizeof(iotx_cmp_send_peer_t));
        strncpy(msg->target->device_name, target->device_name, strlen(target->device_name));
        strncpy(msg->target->product_key, target->product_key, strlen(target->product_key));
    } else {
        msg->target = NULL;
    }

    if (FAIL_RETURN == iotx_cmp_parse_message(g_cmp_cnt, message_info, &URI, &payload, &payload_length)) {
        CMP_ERR(cmp_log_error_memory);
        iotx_cmp_free_list_node(node);
        if (msg->target)
            LITE_free(msg->target);
        return FAIL_RETURN;
    }

    msg->ack_type = message_info->ack_type;
    msg->URI = URI;
    msg->payload = payload;
    msg->payload_length = payload_length;

    rc = iotx_cmp_process_list_push(g_cmp_cnt, IOTX_CMP_PROCESS_TYPE_CLOUD, node);
    if (FAIL_RETURN == rc) {
        CMP_ERR(cmp_log_error_push_node);
        if (msg->target) LITE_free(msg->target);
        if (msg->URI) LITE_free(msg->URI);
        if (msg->payload) LITE_free(msg->payload);
        if (node->msg) LITE_free(node->msg);
        iotx_cmp_free_list_node(node);
    }
}
#else /* CMP_SUPPORT_MULTI_THREAD */
    if (FAIL_RETURN == iotx_cmp_parse_message(g_cmp_cnt, message_info, &URI, &payload, &payload_length)) return FAIL_RETURN;

    rc = iotx_cmp_send_data(g_cmp_cnt, target, URI, message_info->ack_type, payload, payload_length);
#endif /* CMP_SUPPORT_MULTI_THREAD */

    return rc;
}


#ifndef CMP_SUPPORT_MULTI_THREAD

/**
 * @brief Send data.
 *        This function used to send data to target and wait for response.
 *        If the target is NULL, the data will broadcast to all the reachable places.
 *        If the target is not NULL, the data will send to target only.
 *        If the target's product_key and device_name is itself, the data will send to cloud.
 *
 * @param target.
 * @param message_info [in/out].
 * @param option, reserve.
 *
 * @return success or fail.
 */
int IOT_CMP_Send_Sync(iotx_cmp_send_peer_pt target, iotx_cmp_message_info_pt message_info, void* option)
{
    if (NULL == g_cmp_cnt){
        CMP_ERR(cmp_log_error_status);
        return FAIL_RETURN;
    }

    CMP_WARNING(cmp_log_warning_not_support);

    return SUCCESS_RETURN;
}

/**
 * @brief Yield.
 *        This function used to yield when want to received data.
 *        This function just need used in CMP_SUPPORT_MULTI_THREAD = n.
 *        If the CMP_SUPPORT_MULTI_THREAD = y, this function is no need.
 *
 * @param target.
 * @param message_info.
 * @param option, reserve.
 *
 * @return success or fail.
 */
int IOT_CMP_Yield(int timeout_ms, void* option)
{
    iotx_cmp_connectivity_list_pt conn_list = NULL;

    if (NULL == g_cmp_cnt){
        CMP_ERR(cmp_log_error_status);
        return FAIL_RETURN;
    }

    conn_list = g_cmp_cnt->connectivity_list;

    while(conn_list) {
        if (conn_list->node && conn_list->node->yield_func) {
            if (0 == conn_list->node->is_connected) {
                conn_list->node->connect_func(g_cmp_cnt, conn_list->node);
                if (1 == conn_list->node->is_connected) {
                    iotx_cmp_event_msg_t event_msg = {0};
                    event_msg.event_id = IOTX_CMP_EVENT_CLOUD_CONNECTED;
                    event_msg.msg = NULL;
                    iotx_cmp_trigger_event_callback(g_cmp_cnt, &event_msg);
                }
            } else {
                conn_list->node->yield_func(conn_list->node, timeout_ms);
            }
        }
        conn_list = conn_list->next;
    }

    return SUCCESS_RETURN;
}
#endif /* CMP_SUPPORT_MULTI_THREAD */

/**
 * @brief deinit
 *
 * @param option, reserve.
 *
 * @return success or fail.
 */
int IOT_CMP_Deinit(void* option)
{
    if (NULL == g_cmp_cnt){
        CMP_ERR(cmp_log_error_status);
        return FAIL_RETURN;
    }

    /* stop thread */
#ifdef CMP_SUPPORT_MULTI_THREAD
    g_cmp_cnt->thread_stop = 1;
    while(!g_cmp_cnt->thread_is_stoped) {
        HAL_SleepMs(10);
    }
#endif

    iotx_cmp_remove_connectivity_all(g_cmp_cnt);

#ifdef SERVICE_OTA_ENABLED
    if (g_cmp_cnt->ota_handler) {
        iotx_cmp_ota_deinit(g_cmp_cnt);
    }
#endif /* SERVICE_OTA_ENABLED */

#ifdef CMP_SUPPORT_MULTI_THREAD
    if (g_cmp_cnt->process_cloud_lock) {
        HAL_MutexDestroy(g_cmp_cnt->process_cloud_lock);
    }
#ifdef CMP_SUPPORT_LOCAL_CONN
    if (g_cmp_cnt->process_local_lock) {
        HAL_MutexDestroy(g_cmp_cnt->process_local_lock);
    }
#endif
#endif

    /* free memory */
#ifdef CMP_SUPPORT_MULTI_THREAD
    iotx_cmp_free_list_node_all(g_cmp_cnt);
#endif
    iotx_cmp_remove_mapping_all(g_cmp_cnt);

    if (g_cmp_cnt->event_callback_list) {
        iotx_cmp_event_callback_list_pt event_callback = g_cmp_cnt->event_callback_list;
        iotx_cmp_event_callback_list_pt pre_event_callback = event_callback;

        while (event_callback) {
            LITE_free(event_callback->node);
            pre_event_callback = event_callback;
            event_callback = event_callback->next;
            LITE_free(pre_event_callback);
        }
    }

    LITE_free(g_cmp_cnt);
    g_cmp_cnt = 0;

    return SUCCESS_RETURN;
}


#ifdef SERVICE_OTA_ENABLED
int IOT_CMP_OTA_Yield(iotx_cmp_ota_pt ota_pt)
{
    if (NULL == g_cmp_cnt) {
        CMP_ERR(cmp_log_error_status);
        return FAIL_RETURN;
    }

    if (NULL == ota_pt || NULL == ota_pt->buffer) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    return iotx_cmp_ota_yield(g_cmp_cnt, ota_pt);
}
#endif /* SERVICE_OTA_ENABLED */



#ifdef UT_TEST
int request_inject(int id, char* uri, char* method, void* parameter, int parameter_len)
{
    iotx_cmp_message_info_t message_info = {0};

    message_info.id = id;
    message_info.code = 0;

    message_info.URI = LITE_calloc(1, strlen(uri) + 1, MEM_MAGIC, "CMP");
    strcpy(message_info.URI, uri);

    message_info.URI_type = IOTX_CMP_URI_UNDEFINE;

    message_info.method = LITE_calloc(1, strlen(method) + 1, MEM_MAGIC, "CMP");
    strcpy(message_info.method, method);

    message_info.parameter = LITE_calloc(1, strlen(parameter) + 1, MEM_MAGIC, "CMP");
    strcpy(message_info.parameter, parameter);

    message_info.parameter_length = parameter_len;
    message_info.message_type = IOTX_CMP_MESSAGE_REQUEST;

    iotx_cmp_mqtt_direct_response_handler(g_cmp_cnt, &message_info);
    return SUCCESS_RETURN;
}

int response_inject(int id, char* uri, int code, void* data, int data_length)
{
    iotx_cmp_message_info_t message_info = {0};

    message_info.id = id;
    message_info.code = code;

    message_info.URI = LITE_calloc(1, strlen(uri) + 1, MEM_MAGIC, "CMP");
    strcpy(message_info.URI, uri);

    message_info.URI_type = IOTX_CMP_URI_UNDEFINE;

    message_info.method = NULL;

    message_info.parameter = LITE_calloc(1, strlen(data) + 1, MEM_MAGIC, "CMP");
    strcpy(message_info.parameter, data);

    message_info.parameter_length = data_length;
    message_info.message_type = IOTX_CMP_MESSAGE_RESPONSE;

    iotx_cmp_mqtt_direct_response_handler(g_cmp_cnt, &message_info);
    return SUCCESS_RETURN;
}

#endif





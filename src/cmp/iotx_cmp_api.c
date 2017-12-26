#include "iot_import.h"

#include "utils_list.h"
#include "lite-utils.h"
#include "device.h"
#include "iot_export.h"
#include "iotx_cmp_common.h"
#include "iotx_cmp_mqtt_direct.h"
#include "iotx_cmp_cloud_conn.h"
#include "iotx_cmp_ota.h"

/* todo */
#define OTA_VERSION             "iotx_ver_1.0.0"


static iotx_cmp_conntext_pt g_cmp_cnt = NULL;

extern void guider_set_domain_type(int domain_type);

/**
 * @brief CMP initial
 *        This function initialize the CMP structures, establish network connection
 *        If CMP have been initial, this function will return success directly.
 *
 * @param cmp_param, specify the cmp and event handler.
 * @param option, reserve.
 *
 * @return success or fail.
 */
int IOT_CMP_Init(iotx_cmp_init_param_pt pparam, void* option)
{    
    iotx_conn_info_pt pconn_info;
    
    if (NULL != g_cmp_cnt) {
        log_info("cmp have been initial");
        return SUCCESS_RETURN;
    }
    
    if (NULL == pparam) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    if (NULL == pparam->product_key || NULL == pparam->device_name || NULL == pparam->device_secret){
        log_info("parameter error");
        return FAIL_RETURN;
    }

    if (0 != IOT_SetupConnInfo(pparam->product_key, pparam->device_name, pparam->device_secret, (void **)&pconn_info)) {
        return FAIL_RETURN;
    }

    g_cmp_cnt = LITE_malloc(sizeof(iotx_cmp_conntext_t));
    if (NULL == g_cmp_cnt) {
        log_info("memory error");
        return FAIL_RETURN;
    }

    memset(g_cmp_cnt, 0x0, sizeof(iotx_cmp_conntext_t));

    g_cmp_cnt->event_func = pparam->event_func;
    g_cmp_cnt->user_data = pparam->user_data;
    g_cmp_cnt->response_func = iotx_cmp_response_func;

    
    guider_set_domain_type(pparam->domain_type);

    /* cloud_connection */
#ifdef CMP_VIA_MQTT_DIRECT
    g_cmp_cnt->cloud_connection = iotx_cmp_mqtt_direct_init(g_cmp_cnt, pparam);
#else
    g_cmp_cnt->cloud_connection = iotx_cmp_cloud_conn_init(g_cmp_cnt, pparam);
#endif
    if (NULL == g_cmp_cnt->cloud_connection) {
        LITE_free(g_cmp_cnt);
        g_cmp_cnt = 0;
        return FAIL_RETURN;
    }

    /* local connection */

    /* ota init */
#ifdef CMP_SUPPORT_OTA    
    g_cmp_cnt->ota_handler = iotx_cmp_ota_init(g_cmp_cnt, OTA_VERSION);
    if (NULL == g_cmp_cnt->ota_handler) {
    #ifdef CMP_VIA_MQTT_DIRECT
        iotx_cmp_mqtt_direct_deinit(g_cmp_cnt);
    #else
        iotx_cmp_cloud_conn_deinit(g_cmp_cnt);
    #endif
        LITE_free(g_cmp_cnt);
        g_cmp_cnt = 0;
        return FAIL_RETURN;
    }
#endif /* CMP_SUPPORT_OTA */

    /* create thread */
#ifdef CMP_SUPPORT_MULTI_THREAD
    g_cmp_cnt->process_lock = HAL_MutexCreate();
    if (NULL == g_cmp_cnt->process_lock) {
        log_info("mutex create error");
        return FAIL_RETURN;
    }

    if (0 != pthread_create(&g_cmp_cnt->pthread_process, NULL, iotx_cmp_process, (void*)g_cmp_cnt)) {
        log_info("pthread_create failed!\n");
        return FAIL_RETURN;
    }
    
    if (0 != pthread_create(&g_cmp_cnt->pthread_yield, NULL, iotx_cmp_yield_process, (void*)g_cmp_cnt)) {
        log_info("pthread_create failed!\n");
        return FAIL_RETURN;
    } 

#endif
    
    return SUCCESS_RETURN;
}


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
 */
int IOT_CMP_Register(iotx_cmp_register_param_pt pparam, void* option)
{        
    char* URI = NULL; 
    int rc = 0;
    int length = CMP_TOPIC_LEN_MAX;

    if (NULL == g_cmp_cnt) {
        log_info("status error!");
        return FAIL_RETURN;
    }

    if (NULL == pparam || NULL == pparam->register_func || NULL == pparam->URI) {
        log_info("parameter error!");
        return FAIL_RETURN;            
    }
    
    /* splice URI */
    URI = LITE_malloc(CMP_TOPIC_LEN_MAX);
    if (NULL == URI){
        log_info("memory error");
        return FAIL_RETURN;
    }
    memset(URI, 0x0, CMP_TOPIC_LEN_MAX);
    
    if (FAIL_RETURN == iotx_cmp_splice_URI(URI, &length, pparam->URI, pparam->URI_type)) {
        log_info("URI splice fail");
        return FAIL_RETURN;
    }    

    log_info("URI %s", URI);

#ifdef CMP_SUPPORT_MULTI_THREAD 
{
    /* send message to itself thread */
    iotx_cmp_process_list_node_pt node = NULL;
    iotx_cmp_process_register_pt msg = NULL;
        
    node = LITE_malloc(sizeof(iotx_cmp_process_list_node_t));
    if (NULL == node) {
        log_info("memory error");
        LITE_free(URI);        
        return FAIL_RETURN;
    }

    memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));

    node->type = IOTX_CMP_PROCESS_REGISTER;
    node->msg = LITE_malloc(sizeof(iotx_cmp_process_register_t));  
    if (NULL == node->msg) {
        log_info("memory error");
        LITE_free(URI);        
        LITE_free(node);
        return FAIL_RETURN;
    }

    msg = node->msg;
    msg->URI = URI;
    msg->register_func = pparam->register_func;
    msg->user_data = pparam->user_data;
    
    rc = iotx_cmp_process_list_push(g_cmp_cnt, node);

    if (FAIL_RETURN == rc) {
        LITE_free(URI);        
        LITE_free(node->msg);  
        LITE_free(node);
    }
}
#else /* CMP_SUPPORT_MULTI_THREAD */
    rc = iotx_cmp_register_service(g_cmp_cnt, URI, pparam->register_func, pparam->user_data);
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
        log_info("status error!");
        return FAIL_RETURN;
    }

    if (NULL == pparam || NULL == pparam->URI) {
        log_info("parameter error!");
        return FAIL_RETURN;            
    }
    
    /* splice URI */
    URI = LITE_malloc(CMP_TOPIC_LEN_MAX);
    if (NULL == URI){
        log_info("memory error");
        return FAIL_RETURN;
    }
    memset(URI, 0x0, CMP_TOPIC_LEN_MAX);
    
    if (FAIL_RETURN == iotx_cmp_splice_URI(URI, &length, pparam->URI, pparam->URI_type)) {
        log_info("URI splice fail");
        return FAIL_RETURN;
    }    

#ifdef CMP_SUPPORT_MULTI_THREAD 
{
    /* send message to itself thread */
    iotx_cmp_process_list_node_pt node = NULL;
        
    node = LITE_malloc(sizeof(iotx_cmp_process_list_node_t));
    if (NULL == node) {
        log_info("memory error");
        LITE_free(URI);        
        return FAIL_RETURN;
    }

    memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));

    node->type = IOTX_CMP_PROCESS_UNREGISTER;
    node->msg = (void*)URI;
    
    rc = iotx_cmp_process_list_push(g_cmp_cnt, node);

    if (FAIL_RETURN == rc) {
        LITE_free(URI);        
        LITE_free(node);
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
    
    if (NULL == g_cmp_cnt) {
        log_info("status error!");
        return FAIL_RETURN;
    }

    if (NULL == target || NULL == message_info) {
        log_info("parameter error!");
        return FAIL_RETURN;            
    }

    if (NULL == message_info->URI) {
        log_info("parameter error!");
        return FAIL_RETURN;            
    }
        
#ifdef CMP_SUPPORT_MULTI_THREAD
{
    /* send message to itself thread */
    iotx_cmp_process_list_node_pt node = NULL;
    iotx_cmp_process_send_pt msg = NULL;
        
    node = LITE_malloc(sizeof(iotx_cmp_process_list_node_t));
    if (NULL == node) {
        log_info("memory error");
        return FAIL_RETURN;
    }

    memset(node, 0x0, sizeof(iotx_cmp_process_list_node_t));

    node->type = IOTX_CMP_PROCESS_SEND;
    node->msg = LITE_malloc(sizeof(iotx_cmp_process_send_t));    
    if (NULL == node->msg) {
        log_info("memory error");
        LITE_free(node);
        return FAIL_RETURN;
    }

    msg = node->msg;
    msg->target = LITE_malloc(sizeof(iotx_cmp_send_peer_t));
    memset(msg->target, 0x0, sizeof(iotx_cmp_send_peer_t));
    strncpy(msg->target->device_name, target->device_name, strlen(target->device_name));
    strncpy(msg->target->product_key, target->product_key, strlen(target->product_key));
    msg->message_info = message_info;
    
    rc = iotx_cmp_process_list_push(g_cmp_cnt, node);

    if (FAIL_RETURN == rc) { 
        LITE_free(node->msg);  
        LITE_free(node);
    }
}
#else /* CMP_SUPPORT_MULTI_THREAD */
    rc = iotx_cmp_send_data(g_cmp_cnt, target, message_info);
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
        log_info("status error!");
        return FAIL_RETURN;
    }
    
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
    if (NULL == g_cmp_cnt){
        log_info("status error!");
        return FAIL_RETURN;
    }

    /* cloud yield */
#ifdef CMP_VIA_MQTT_DIRECT
    iotx_cmp_mqtt_direct_yield(g_cmp_cnt, timeout_ms);
#else
    iotx_cmp_cloud_conn_yield(g_cmp_cnt, timeout_ms);
#endif    

    /* local yield */

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
        log_info("status error!");
        return FAIL_RETURN;
    }

    /* stop thread */
#ifdef CMP_SUPPORT_MULTI_THREAD

#endif

    /* ota */
#ifdef CMP_SUPPORT_OTA    
    iotx_cmp_ota_deinit(g_cmp_cnt, OTA_VERSION);
#endif /* CMP_SUPPORT_OTA */

    /* cloud deinit */
#ifdef CMP_VIA_MQTT_DIRECT
    iotx_cmp_mqtt_direct_deinit(g_cmp_cnt);
#else
    iotx_cmp_cloud_conn_deinit(g_cmp_cnt);
#endif    

    /* local deinit */


    /* free memory */
    iotx_cmp_remove_mapping_all(g_cmp_cnt);

    LITE_free(g_cmp_cnt);
    g_cmp_cnt = 0;
    
    return SUCCESS_RETURN;
}

#ifdef CMP_SUPPORT_OTA

int IOT_CMP_OTA_Yield(iotx_cmp_ota_pt ota_pt)
{
    if (NULL == g_cmp_cnt) {
        log_info("status error!");
        return FAIL_RETURN;
    }

    if (NULL == ota_pt || NULL == ota_pt->buffer) {
        log_info("parameter error!");
        return FAIL_RETURN;
    }
    
#ifdef CMP_SUPPORT_MULTI_THREAD

#else
    return iotx_cmp_ota_yield(g_cmp_cnt->ota_handler, ota_pt);
#endif
}


#endif




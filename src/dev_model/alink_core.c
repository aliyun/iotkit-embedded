/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"
#include "alink_core.h"


#define ALINK_CORE_CONNECT_TIMEOUT              (10000)
#define ALINK_CORE_SUBSCRIBE_TIMEOUT            (5000)

#define ALINK_SUPPORT_FD_NUM                    (1)
#define ALINK_DEFAULT_FD_PROTOCOL               (ALINK_BEARER_MQTT)


typedef struct {
    uint8_t                 is_inited;
    alink_core_status_t     status;
    uint8_t                 fd_num;
    void                   *mutex;

    char                   *product_key;
    char                   *product_secret;
    char                   *device_name;
    char                   *device_secret;

    uint32_t                cm_fd;
    uint32_t                msgid;              /* TODO */

} alink_core_ctx_t;


/***************************************************************
 * local function prototypes
 ***************************************************************/
static int _alink_core_init(iotx_dev_meta_info_t *dev_info);
static int _alink_core_deinit(void);

static void _alink_core_rx_event_handle(int fd, const char *uri, uint32_t uri_len, const char *payload, uint32_t payload_len, void *context);

/***************************************************************
 * local variables
 ***************************************************************/
static linkkit_event_cb_t g_linkkit_event_array[ITE_EVENT_NUM] = { NULL };
static alink_core_ctx_t alink_core_ctx = { 0 };

/*  static uint8_t alink_qos_option = 0x00;  TOOD */


/***************************************************************
 * local functions
 ***************************************************************/
static void _alink_core_lock(void)
{
    if (alink_core_ctx.mutex) {
        HAL_MutexLock(alink_core_ctx.mutex);
    }
}

static void _alink_core_unlock(void)
{
    if (alink_core_ctx.mutex) {
        HAL_MutexUnlock(alink_core_ctx.mutex);
    }
}
extern int alink_msg_list_init(void); /* TODO: test */
static int _alink_core_init(iotx_dev_meta_info_t *dev_info)
{
    int res = FAIL_RETURN;

    if (alink_core_ctx.status > ALINK_CORE_STATUS_DEINIT) {
        return IOTX_CODE_ALREADY_OPENED;
    }
    alink_core_ctx.status = ALINK_CORE_STATUS_INITED;
    alink_core_ctx.msgid = 1;

    alink_core_ctx.mutex = HAL_MutexCreate();
    if (alink_core_ctx.mutex == NULL) {
        _alink_core_deinit();
        return IOTX_CODE_CREATE_MUTEX_FAILED;
    }

    /* store master device info duplication */
    if ((alink_core_ctx.product_key = alink_utils_strdup(dev_info->product_key, strlen(dev_info->product_key))) == NULL) {
        _alink_core_deinit();
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    if ((alink_core_ctx.product_secret = alink_utils_strdup(dev_info->product_secret, strlen(dev_info->product_secret))) == NULL) {
        _alink_core_deinit();
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    if ((alink_core_ctx.device_name = alink_utils_strdup(dev_info->device_name, strlen(dev_info->device_name))) == NULL) {
        _alink_core_deinit();
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    if ((alink_core_ctx.device_secret = alink_utils_strdup(dev_info->device_secret, strlen(dev_info->device_secret))) == NULL) {
        _alink_core_deinit();
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    /* init downstream topic hash table */
    res = alink_uri_hash_table_init();
    if (res < SUCCESS_RETURN) {
        _alink_core_deinit();
        return res;
    }

#ifdef DEVICE_MODEL_GATEWAY
    /* init subdev hash table */        
    res = alink_subdev_mgr_init();
    if (res < SUCCESS_RETURN) {
        _alink_core_deinit();
        return res;
    }

    /* init lite_cjson hook */
    {
        lite_cjson_hooks hooks;
        hooks.malloc_fn = alink_utils_malloc;
        hooks.free_fn = alink_utils_free;
        lite_cjson_init_hooks(&hooks);
    }

    /* use upstream req cache if gw enabled */
    alink_upstream_req_ctx_init();

#if (CONFIG_SDK_THREAD_COST == 1)
    alink_msg_list_init();
#endif    

#endif


    
    return SUCCESS_RETURN;
}

/** TODO: not multi thread safe **/
static int _alink_core_deinit(void)
{
    if (alink_core_ctx.status == ALINK_CORE_STATUS_DEINIT) {
        return FAIL_RETURN;
    }
    alink_core_ctx.status = ALINK_CORE_STATUS_DEINIT;

    if (alink_core_ctx.mutex) {
        HAL_MutexDestroy(alink_core_ctx.mutex);
        alink_core_ctx.mutex = NULL;
    }

    if (alink_core_ctx.product_key) {
        alink_free(alink_core_ctx.product_key);
    }

    if (alink_core_ctx.product_secret) {
        alink_free(alink_core_ctx.product_secret);
    }

    if (alink_core_ctx.device_name) {
        alink_free(alink_core_ctx.device_name);
    }

    if (alink_core_ctx.device_secret) {
        alink_free(alink_core_ctx.device_secret);
    }

    /* downstream topic hash table deinit */
    alink_uri_hash_table_deinit();

#ifdef DEVICE_MODEL_GATEWAY
    /* subdev hash table deinit, TODO */
    alink_subdev_mgr_deinit();
    alink_upstream_req_ctx_deinit();
#if (CONFIG_SDK_THREAD_COST == 0)
#else
    alink_msg_list_deinit();
#endif
#endif

    alink_core_ctx.status = ALINK_CORE_STATUS_DEINIT;

    return SUCCESS_RETURN;
}

void _alink_core_set_status(alink_core_status_t status)
{
    _alink_core_lock();

    alink_core_ctx.status = status;

    _alink_core_unlock();
}

/** subscribe wildcard rsp/# and req/# **/
int _alink_core_register_downstream(iotx_cm_data_handle_cb rx_func)
{
    int res = FAIL_RETURN;
    char uri[IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 9];
    const char *uri_req_fmt = "/%s/%s/req/#";
    const char *uri_rsp_fmt = "/%s/%s/rsp/#";
    iotx_cm_ext_params_t sub_params;

    ALINK_ASSERT_DEBUG(rx_func != NULL);

    sub_params.ack_type = IOTX_CM_MESSAGE_NO_ACK;
    sub_params.sync_mode = IOTX_CM_SYNC;
    sub_params.sync_timeout = ALINK_CORE_SUBSCRIBE_TIMEOUT;
    sub_params.ack_cb = NULL;

    HAL_Snprintf(uri, sizeof(uri), uri_req_fmt, alink_core_ctx.product_key, alink_core_ctx.device_name);
    alink_info("sub topic is %s", uri);
    res = iotx_cm_sub(alink_core_ctx.cm_fd, &sub_params, (const char *)uri, rx_func, NULL);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    HAL_Snprintf(uri, sizeof(uri), uri_rsp_fmt, alink_core_ctx.product_key, alink_core_ctx.device_name);
    alink_info("sub topic is %s", uri);
    return iotx_cm_sub(alink_core_ctx.cm_fd, &sub_params, (const char *)uri, rx_func, NULL);

}

/** core receive event handler, message dispatch and distribut **/
static void _alink_core_rx_event_handle(int fd, const char *uri, uint32_t uri_len, const char *payload, uint32_t payload_len, void *context)
{
    alink_uri_query_t query = {0};
    uint32_t devid = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN] = {0};
    char path[ALINK_URI_PATH_LEN_MAX] = {0};
    alink_downstream_handle_func_t handle_func;

    if (uri == NULL || uri_len == 0 || payload == NULL || payload_len == 0) {
        return;
    }

    /* reslove the uri query */
    alink_format_reslove_uri(uri, uri_len, product_key, device_name, path, &query);

    /* reslove the uri path */
    alink_info("rx data, uri = %.*s", uri_len, uri);
    alink_info("rx data, data = %.*s", payload_len, payload);

    alink_info("pk = %s", product_key);
    alink_info("dn = %s", device_name);
    alink_info("path = %s", path);
    alink_info("query id = %d", query.id);
    alink_info("query format = %c", query.format);
    alink_info("query compress = %c", query.compress);
    alink_info("query code = %d", query.code);
    alink_info("query ack = %c", query.ack);

    if (product_key[0] == '\0' && device_name[0] == '\0') {
        devid = 0;
    }
    else {
#ifdef DEVICE_MODEL_GATEWAY                
        /* get subdev devid */
        alink_subdev_get_devid_by_pkdn(product_key, device_name, &devid);
        if (0 == devid) {
            alink_err("downstream subdev not exist");
            return;
        }
#else
        alink_err("downstream uri error");
        return;
#endif
    }

    handle_func = alink_get_uri_handle_func(path, strlen(path));
    if (handle_func != NULL) {
        alink_info("invoke downstream uri handler");
        handle_func(devid, product_key, device_name, (const uint8_t *)payload, payload_len, &query);
    }
    else {
        alink_info("downstream uri handler no exist");
    }
}

void _alink_core_connected_event_handler(void)
{
    linkkit_connect_success_cb_t handle_func;

    _alink_core_set_status(ALINK_CORE_STATUS_CONNECTED);

    handle_func = alink_get_event_callback(ITE_CONNECT_SUCC);
    if (handle_func) {
        handle_func(ALINK_DEVICE_SELF_ID);
    }
}

void _alink_core_disconnected_event_handler(void)
{
    linkkit_disconnected_cb_t handle_func;

    _alink_core_set_status(ALINK_CORE_STATUS_DISCONNECTED);

    handle_func = alink_get_event_callback(ITE_CONNECT_SUCC);
    if (handle_func) {
        handle_func(ALINK_DEVICE_SELF_ID);
    }
}

void alink_cm_event_handle(int fd, iotx_cm_event_msg_t *event, void *context)
{
    switch (event->type) {
        case IOTX_CM_EVENT_CLOUD_CONNECTED: {
            _alink_core_connected_event_handler();
        }
        break;
        case IOTX_CM_EVENT_CLOUD_CONNECT_FAILED: {
            /* TODO: no impement */
        }
        break;
        case IOTX_CM_EVENT_CLOUD_DISCONNECT: {
            _alink_core_disconnected_event_handler();
        }
        break;
        default:
            break;
    }
}

/***************************************************************
 * global functions
 ***************************************************************/
/** **/
int alink_core_open(iotx_dev_meta_info_t *dev_info)
{
    int res;

    ALINK_ASSERT_DEBUG(dev_info != NULL);
    
    /* init core in open api */
    res = _alink_core_init(dev_info);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    if (alink_core_ctx.fd_num >= ALINK_SUPPORT_FD_NUM) {
        return FAIL_RETURN;
    }
    alink_core_ctx.fd_num++;

    {
        iotx_cm_init_param_t cm_param;
        cm_param.dev_info = dev_info;
        cm_param.region = IOTX_CLOUD_REGION_SHANGHAI;       /* pass region??? */
#if defined(COAP_COMM_ENABLED) && !defined(MQTT_COMM_ENABLED)
        cm_param.protocol_type = IOTX_CM_PROTOCOL_TYPE_COAP;
#else
        cm_param.protocol_type = IOTX_CM_PROTOCOL_TYPE_MQTT;
#endif
        cm_param.handle_event = alink_cm_event_handle;

        alink_core_ctx.cm_fd = iotx_cm_open(&cm_param);
    }

    alink_info("mqtt fd open succeed");
    alink_core_ctx.status = ALINK_CORE_STATUS_OPENED;

    return SUCCESS_RETURN;
}

int alink_core_close(void)
{
    iotx_cm_close(alink_core_ctx.cm_fd);

    _alink_core_deinit();

    return SUCCESS_RETURN;
}

int alink_core_connect_cloud(void)
{
    int res = FAIL_RETURN;

    res = iotx_cm_connect(alink_core_ctx.cm_fd, ALINK_CORE_CONNECT_TIMEOUT);

    if (res < SUCCESS_RETURN) {
        alink_info("connect failed");
        return res;
    }
    alink_info("connect succeed");

    /* TODO: invoke the connected event, event post for awss immediately, indicate success for fail, hehe */

    /*  */
    res = _alink_core_register_downstream(_alink_core_rx_event_handle);
    if (res < SUCCESS_RETURN) {
        alink_info("subscribe failed");
        return res;
    }

    alink_core_ctx.status = ALINK_CORE_STATUS_CONNECTED;
    alink_info("subscribe succeed");
    return SUCCESS_RETURN;
}

/** TODO: cause deadlock **/
alink_core_status_t alink_core_get_status(void)
{
    return alink_core_ctx.status;
}

/** **/
uint32_t alink_core_allocate_msgid(void)
{
    uint32_t msgid;

    _alink_core_lock();
    msgid = alink_core_ctx.msgid++;
    _alink_core_unlock();

    return (msgid & 0x7FFFFFFF);
}

extern int alink_msg_event_list_handler(void);  /* TODO */

/** **/
int alink_core_yield(uint32_t timeout_ms)
{
#if (CONFIG_SDK_THREAD_COST == 1)
    alink_msg_event_list_handler();
    HAL_SleepMs(timeout_ms);
#endif

    return iotx_cm_yield(alink_core_ctx.cm_fd, timeout_ms);
}

/** **/
int alink_core_send_req_msg(char *uri, const uint8_t *payload, uint32_t len)
{
    ALINK_ASSERT_DEBUG(uri != NULL);
    ALINK_ASSERT_DEBUG(payload != NULL);

    alink_info("uri: %s", uri);

    /* return error if status is not connected */
    if (alink_core_get_status() != ALINK_CORE_STATUS_CONNECTED) {
        return IOTX_CODE_STATE_ERROR;
    }

    {
        iotx_cm_ext_params_t pub_param;
        memset(&pub_param, 0, sizeof(iotx_cm_ext_params_t));
        pub_param.ack_type = IOTX_CM_MESSAGE_NO_ACK;
        pub_param.sync_mode = IOTX_CM_ASYNC;
        pub_param.sync_timeout = 0;
        pub_param.ack_cb = NULL;
        return iotx_cm_pub(alink_core_ctx.cm_fd, &pub_param, (const char *)uri, (const char *)payload, len);
    }
}

/** **/
linkkit_event_cb_t alink_get_event_callback(iotx_linkkit_event_type_t event_id)
{
    if (event_id >= ITE_EVENT_NUM) {
        return NULL;
    }

    return g_linkkit_event_array[event_id];
}

/** **/
int alink_set_event_callback(iotx_linkkit_event_type_t event_id, linkkit_event_cb_t callback)
{
    if (event_id >= ITE_EVENT_NUM || callback == NULL) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    g_linkkit_event_array[event_id] = callback;
    return SUCCESS_RETURN;
}




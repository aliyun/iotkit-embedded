/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"
#include "alink_core.h"
#include "infra_defs.h"


#define ALINK_SUPPORT_BEARER_NUM                (1)
#define ALINK_DEFAULT_BEARER_PROTOCOL           (ALINK_BEARER_MQTT)
#define ALINK_DEFAULT_SUB_LEVEL                 "/rsp"



typedef struct {
    uint8_t                 is_inited;
    alink_core_status_t     status;
    uint8_t                 bearer_num;
    void                   *mutex;

    char                   *product_key;
    char                   *product_secret;
    char                   *device_name;
    char                   *device_secret;

    alink_bearer_node_t    *p_activce_bearer;
    uint32_t                msgid;              /* TODO */

} alink_core_ctx_t;


/***************************************************************
 * local function prototypes
 ***************************************************************/
static int _alink_core_init(iotx_dev_meta_info_t *dev_info);
static int _alink_core_deinit(void);
static void _alink_core_rx_event_handle(void *handle, const char *uri, uint32_t uri_len, const char *payload, uint32_t payload_len);


/**
 * local variables
 */
static linkkit_event_cb_t g_linkkit_event_array[ITE_EVENT_NUM] = { NULL };

static alink_core_ctx_t alink_core_ctx = { 0 };


/*  static uint8_t alink_qos_option = 0x00;      /* TOOD */ */





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

int _alink_core_store_dev_meta(char **dest, const char *source)
{
    uint32_t len = strlen(source);

    if (len) {
        if ( (*dest = alink_utils_strdup(source, len)) == NULL ) {
            return FAIL_RETURN;
        }
    }

    return SUCCESS_RETURN;
}

static int _alink_core_init(iotx_dev_meta_info_t *dev_info)
{
    int res = FAIL_RETURN;

    if (strlen(dev_info->product_key) == 0 || strlen(dev_info->device_name) == 0) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    if (alink_core_ctx.status > ALINK_CORE_STATUS_DEINIT) {
        return SUCCESS_RETURN;          /* already init, just return success */
    }
    alink_core_ctx.status = ALINK_CORE_STATUS_INITED;

    alink_core_ctx.mutex = HAL_MutexCreate();
    if (alink_core_ctx.mutex == NULL) {
        _alink_core_deinit();
        return res;
    }

    /* store master device info duplication */
    if ((res = _alink_core_store_dev_meta(&alink_core_ctx.product_key, dev_info->product_key)) < SUCCESS_RETURN) {
        _alink_core_deinit();
        return res;
    }

    if ((res = _alink_core_store_dev_meta(&alink_core_ctx.product_secret, dev_info->product_secret)) < SUCCESS_RETURN) {
        _alink_core_deinit();
        return res;
    }

    if ((res = _alink_core_store_dev_meta(&alink_core_ctx.device_name, dev_info->device_name)) < SUCCESS_RETURN) {
        _alink_core_deinit();
        return res;
    }

    if ((res = _alink_core_store_dev_meta(&alink_core_ctx.device_secret, dev_info->device_secret)) < SUCCESS_RETURN) {
        _alink_core_deinit();
        return res;
    }            

    res = alink_downstream_hash_table_init();
    if (res < SUCCESS_RETURN) {
        _alink_core_deinit();
        return res;
    }

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

    alink_downstream_hash_table_deinit();

    return SUCCESS_RETURN;
}

/**
 * 
 */
int _alink_core_register_downstream(const char *level, alink_bearer_rx_cb_t rx_func)
{
    ALINK_ASSERT_DEBUG(level != NULL);
    ALINK_ASSERT_DEBUG(rx_func != NULL);

#ifdef TEST_MOCK
    const char *uri = "/sys/a1OFrRjV8nz/develop_01/thing/service/property/set";
    int res = alink_bearer_register(alink_core_ctx.p_activce_bearer, uri, rx_func);
#else
    int res = FAIL_RETURN;
    const char *uri_fmt = "/%s/%s%s";
    uint32_t pk_len = strlen(alink_core_ctx.product_key);
    uint32_t dn_len = strlen(alink_core_ctx.device_name);

    char *uri = alink_malloc(strlen(uri_fmt) + pk_len + dn_len);
    if (uri == NULL) {
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    HAL_Snprintf(uri, uri_fmt, alink_core_ctx.product_key, alink_core_ctx.device_name, level);
    res = alink_bearer_register(alink_core_ctx.p_activce_bearer, uri, rx_func);
    alink_free(uri);
#endif
    return res;
}

/**
 * TODO: handle is not used
 */
static void _alink_core_rx_event_handle(void *handle, const char *uri, uint32_t uri_len, const char *payload, uint32_t payload_len)
{   
    if (uri == NULL || uri_len == 0 || payload == NULL || payload_len == 0) {
        return;
    } 

    alink_uri_query_t query = { 0 };
    uint32_t devid = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN] = {0};
    char path[50] = {0};        /* TODO: len? */
    uint8_t is_subdev;

    alink_downstream_handle_func_t handle_func;

    /* reslove the uri query */
    alink_format_reslove_uri(uri, uri_len, product_key, device_name, path, &query, &is_subdev);

    /* reslove the uri path */
    alink_info("rx data, uri = %.*s", uri_len, uri);
    alink_info("rx data, data = %.*s", payload_len, payload);

    alink_info("pk = %s", product_key);
    alink_info("dn = %s", device_name);
    alink_info("path = %s", path);
    alink_info("is_subdev = %d", is_subdev);

    alink_info("query id = %d", query.id);
    alink_info("query format = %c", query.format);
    alink_info("query compress = %c", query.compress);
    alink_info("query code = %d", query.code);
    alink_info("query ack = %c", query.ack);

    if (is_subdev == IOT_FALSE) {
        /* check the master pk, dn? */

        /* clear up the pkdn, is useless for handle function if it indicate a master */
        product_key[0] = '\0';
        device_name[0] = '\0';
        devid = 0;
    }
    else {
#ifdef DEVICE_MODEL_GATEWAY                
        /* get the devid */
        alink_subdev_get_devid_by_pkdn(product_key, device_name, &devid);

        if (0 == devid) {
            alink_err("subdev not exist");
            return;
        }
#else
        return;
#endif
    }

    handle_func = alink_downstream_get_handle_func(path, strlen(path));

    /* TODO: for test!!! */
    if (handle_func != NULL) {
        handle_func(devid, product_key, device_name, (const uint8_t *)payload, payload_len, &query);
    }
    else {
        alink_err("handle func of this path is NULL");
    }
}

/***************************************************************
 * global functions
 ***************************************************************/
/**
 *
 */
int alink_core_open(iotx_dev_meta_info_t *dev_info)
{
    ALINK_ASSERT_DEBUG(dev_info != NULL);

    /* init core in open api */
    int res = _alink_core_init(dev_info);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    if (alink_core_ctx.bearer_num >= ALINK_SUPPORT_BEARER_NUM) {
        return FAIL_RETURN;
    }
    alink_core_ctx.bearer_num++;

    /* add one default bearer protocol */
    alink_core_ctx.p_activce_bearer = alink_bearer_open(ALINK_DEFAULT_BEARER_PROTOCOL, dev_info);
    if (alink_core_ctx.p_activce_bearer == NULL) {
        alink_info("bearer open fail");
        _alink_core_deinit();
        return FAIL_RETURN;
    }

    alink_info("bearer open succeed");
    return SUCCESS_RETURN;
}

int alink_core_close(void)
{
#ifdef DEVICE_MODEL_GATEWAY
    
#endif

    alink_bearer_close();

    _alink_core_deinit();

    return SUCCESS_RETURN;
}

int alink_core_connect_cloud(void)
{
    int res = FAIL_RETURN;

    res = alink_bearer_conect();
    if (res < SUCCESS_RETURN) {
        return res;
        alink_debug("connected fail");
    }

    /* TODO: invoke the connected event, event post for awss immediately, indicate success for fail, hehe */


    alink_debug("connected, start sub");
    _alink_core_register_downstream(ALINK_DEFAULT_SUB_LEVEL, (alink_bearer_rx_cb_t)_alink_core_rx_event_handle);

    return res;
}

uint32_t alink_core_get_msgid(void)
{
    uint32_t msgid;

    _alink_core_lock();
    msgid = alink_core_ctx.msgid++;
    _alink_core_unlock();

    return (msgid & 0x7FFFFFFF);
}

/** TODO **/
int alink_core_unsubscribe_downstream_uri()
{
    int res = FAIL_RETURN;

    return res;
}

/** **/
int alink_core_yield(uint32_t timeout_ms)
{
    return alink_bearer_yield(timeout_ms);
}

/** **/
int alink_core_send_req_msg(char *uri, const uint8_t *payload, uint32_t len)
{
    ALINK_ASSERT_DEBUG(uri != NULL);
    ALINK_ASSERT_DEBUG(payload != NULL);

    alink_info("uri: %s", uri);

    return alink_bearer_send(0, uri, (uint8_t *)payload, len);     /* TODO, parameters type */
}

/** **/
linkkit_event_cb_t alink_get_event_callback(iotx_linkkit_event_type_t event_id)
{
    if (event_id < 0 || event_id >= ITE_EVENT_NUM) {
        return NULL;
    }

    return g_linkkit_event_array[event_id];
}

/** **/
int alink_set_event_callback(iotx_linkkit_event_type_t event_id, linkkit_event_cb_t callback)
{
    if (event_id < 0 || event_id >= ITE_EVENT_NUM || callback == NULL) {
        return IOTX_CODE_PARAMS_INVALID;
    }

    g_linkkit_event_array[event_id] = callback;
    return SUCCESS_RETURN;
}


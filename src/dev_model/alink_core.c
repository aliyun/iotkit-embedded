/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"

#include "alink_core.h"

#include "infra_defs.h"


#define ALINK_SUPPORT_BEARER_NUM                (1)
#define ALINK_DEFAULT_BEARER_PROTOCOL           (ALINK_BEARER_MQTT)


typedef struct {
    uint8_t                 is_inited;
    uint8_t                 bearer_num;
    void                   *mutex;

    alink_bearer_node_t    *p_activce_bearer;
    uint32_t                msgid;              // TODO
} alink_core_ctx_t;

static alink_core_ctx_t alink_core_ctx = {
    .is_inited          = 0,
    .bearer_num         = 0,
    .p_activce_bearer   = NULL,
    .mutex              = NULL
};


/****************
 * 
 *******************/
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





uint32_t alink_core_get_msgid(void)
{
    uint32_t msgid;

    _alink_core_lock();
    msgid = alink_core_ctx.msgid++;
    _alink_core_unlock();

    return (msgid & 0x7FFFFFFF);
}

int alink_core_init(void)
{
    if (alink_core_ctx.is_inited) {
        return FAIL_RETURN;
    }

    alink_core_ctx.is_inited = 1;
    alink_core_ctx.mutex = HAL_MutexCreate();

    return SUCCESS_RETURN;
}

int alink_core_deinit(void) 
{
    if (!alink_core_ctx.is_inited) {
        return FAIL_RETURN;
    }

    alink_core_ctx.is_inited = 0;
    HAL_MutexDestroy(alink_core_ctx.mutex);

    return SUCCESS_RETURN;
}

/**
 * 
 */
int alink_core_open(iotx_dev_meta_info_t *dev_info)
{
    ALINK_ASSERT_DEBUG(dev_info != NULL);

    if (alink_core_ctx.bearer_num >= ALINK_SUPPORT_BEARER_NUM) {
        return FAIL_RETURN;
    }
    alink_core_ctx.bearer_num++;

    /* add one default bearer protocol */
    alink_core_ctx.p_activce_bearer = alink_bearer_open(ALINK_DEFAULT_BEARER_PROTOCOL, dev_info);
    if (alink_core_ctx.p_activce_bearer == NULL) {
        alink_info("bearer open fail");
        return FAIL_RETURN;
    }

    /* TOOD */
    _alink_core_lock();
    _alink_core_unlock();

    alink_info("bearer open succeed");
    return SUCCESS_RETURN;
}

int alink_core_subdev_init()
{
    int res = FAIL_RETURN;

    return res;    
}

int alink_core_connect_cloud(void)
{
    int res = FAIL_RETURN;

    res = alink_bearer_conect();
    if (res == FAIL_RETURN) {
        return res;
    }

    /*  */
    alink_core_subscribe_downstream();

    return res;
}

int alink_core_send_req_msg(char *uri, uint8_t *payload, uint32_t len)
{
    ALINK_ASSERT_DEBUG(uri != NULL);

    int res = FAIL_RETURN;

    /* append query */
    alink_info("uri: %s", uri);

    alink_bearer_send(0, uri, payload, len);

    HAL_Free(uri);

    return res;
}

int alink_core_subdev_connect_cloud()
{
    int res = FAIL_RETURN;

    return res;
}


extern int alink_format_resolve_query(const char *uri, uint32_t uri_len, alink_uri_query_t *query, uint8_t *query_len);

/**
 * 
 */
void _alink_core_rx_event_handle(void *handle, const char *uri, uint32_t uri_len, const char *payload, uint32_t payload_len)
{
    /* reslove the uri query */

    /* reslove layer level, is proxy */

    /* reslove the uri path */
    alink_info("rx data, uri = %.*s", uri_len, uri);
    alink_info("rx data, data = %.*s", payload_len, payload);

    alink_uri_query_t query = {0};
    uint8_t query_len = 0;

    alink_format_resolve_query(uri, uri_len, &query, &query_len);
    
    alink_info("query_len = %d", query_len);        /* not include '/' */
    alink_info("query id = %d", query.id);
    alink_info("query format = %c", query.format);
    alink_info("query compress = %c", query.compress);
    alink_info("query code = %d", query.code);
    alink_info("query ack = %c", query.ack);
}


extern int alink_format_create_hash_table(void);


/**
 * 
 */
int alink_core_subscribe_downstream(void)
{
    int res = FAIL_RETURN;

    const char *uri = "/sys/a1OFrRjV8nz/develop_01/thing/service/property/set";



    // for test
    alink_format_create_hash_table();



    res = alink_bearer_register(alink_core_ctx.p_activce_bearer, uri, (alink_bearer_rx_cb_t)_alink_core_rx_event_handle);

    return res;
}

int alink_core_unsubscribe_downstream_uri()
{
    int res = FAIL_RETURN;

    return res;
}

/**
 * 
 */
int alink_core_yield(uint32_t timeout_ms)
{
    return alink_bearer_yield(timeout_ms);
}

int alink_core_subdev_deinit()
{
    int res = FAIL_RETURN;

    return res;
}

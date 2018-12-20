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

    alink_bearer_conect();

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

int alink_core_subscribe_topic()
{
    int res = FAIL_RETURN;

    return res;
}

int alink_core_unsubscribe_topic()
{
    int res = FAIL_RETURN;

    return res;
}

int alink_core_yield(uint32_t timeout_ms)
{
    int res = FAIL_RETURN;

    res = alink_bearer_yield(timeout_ms);

    return res;
}

int alink_core_subdev_deinit()
{
    int res = FAIL_RETURN;

    return res;
}

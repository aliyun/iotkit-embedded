/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_core.h"



typedef struct {
    void *proto_bearer_p;
    uint16_t supported_protocal;



} alink_procotol_ctx_t;


int32_t alink_core_init(void)
{
    
    /* ota module init */


    
    
    /* add the bearer protocol */
    alink_bearer_open();

    return SUCCESS_RETURN;
}

int32_t alink_core_subdev_init()
{
    int32_t res = FAIL_RETURN;

    return res;    
}

int32_t alink_core_connect_cloud(void)
{
    int32_t res = FAIL_RETURN;

    alink_bearer_conect();

    return res;
}

int32_t alink_core_subdev_connect_cloud()
{
    int32_t res = FAIL_RETURN;

    return res;
}

int32_t alink_core_subscribe_topic()
{
    int32_t res = FAIL_RETURN;

    return res;
}

int32_t alink_core_unsubscribe_topic()
{
    int32_t res = FAIL_RETURN;

    return res;
}

int32_t alink_core_yield()
{
    int32_t res = FAIL_RETURN;

    return res;
}

int32_t alink_core_deinit()
{
    int32_t res = FAIL_RETURN;

    return res;
}

int32_t alink_core_subdev_deinit()
{
    int32_t res = FAIL_RETURN;

    return res;
}

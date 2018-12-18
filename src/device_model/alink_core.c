/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_core.h"



typedef struct {
    void *proto_bearer_p;
    uint16_t supported_protocal;



} alink_procotol_ctx_t;


int alink_core_init(void)
{
    /* ota module init */

    
    
    /* add the bearer protocol */
    alink_bearer_open();

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

int alink_core_send_msg(alink_msg_uri_metadata_t *uri_meta, uint8_t *payload, uint32_t len)
{
    char uri[50] = {0};

    alink_format_assamble_uri(uri_meta, uri, sizeof(uri));
    alink_info("uri: %s", uri);

    alink_bearer_send(0, uri, payload, len);

    return 1;
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

int alink_core_yield()
{
    int res = FAIL_RETURN;

    return res;
}

int alink_core_deinit()
{
    int res = FAIL_RETURN;

    return res;
}

int alink_core_subdev_deinit()
{
    int res = FAIL_RETURN;

    return res;
}

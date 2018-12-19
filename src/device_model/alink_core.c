/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"

#include "alink_core.h"

#include "infra_defs.h"


typedef struct {
    uint8_t current_link_idx;
} alink_core_ctx_t;


static alink_core_ctx_t *alink_core_ctx = NULL;


/**
 * 
 */
int alink_core_open(iotx_dev_meta_info_t *dev_info)
{
    /* ota module init */
    if (alink_core_ctx == NULL) {
        if ((alink_core_ctx = HAL_Malloc(sizeof(alink_core_ctx_t))) == NULL) {
            alink_info("alink core malloc fail");
            return FAIL_RETURN;
        }
    }
    else {
        return FAIL_RETURN;
    }
    
    /* add the bearer protocol */
    alink_core_ctx->current_link_idx = alink_bearer_open(ALINK_BEARER_MQTT, dev_info);
    if (alink_core_ctx->current_link_idx == FAIL_RETURN) {
        alink_info("bearer open fail");
    }
    else {
        alink_info("bearer open success");
    }


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

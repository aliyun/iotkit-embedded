/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"

#include "infra_defs.h"

#if 0
static alink_bearer_mqtt_ctx_t p_alink_bearer_mqtt;
#endif


typedef struct {
    uint8_t     link_num;
    list_head_t bearer_node_list;
} alink_bearer_ctx_t;

//static alink_bearer_ctx_t alink_bearer_ctx;

static alink_bearer_mqtt_ctx_t *p_alink_bearer_mqtt = NULL;

/**
 * return a linkId if open seccessfully
 */
alink_bearer_node_t *alink_bearer_open(alink_bearer_type_t protocol_type, iotx_dev_meta_info_t *dev_info)
{
    /* check if protocal support, TODO */

    if (protocol_type == ALINK_BEARER_MQTT) {
        //alink_bearer_mqtt_ctx_t *p_alink_bearer_mqtt;

        p_alink_bearer_mqtt = HAL_Malloc(sizeof(alink_bearer_mqtt_ctx_t));
        if (NULL == p_alink_bearer_mqtt) {
            alink_info("bearer malloc fail");
            return NULL;
        }

        memset(p_alink_bearer_mqtt, 0, sizeof(alink_bearer_mqtt_ctx_t));
        p_alink_bearer_mqtt->dev_info = dev_info;
        p_alink_bearer_mqtt->region = IOTX_CLOUD_REGION_SHANGHAI;           // TODO

        alink_bearer_mqtt_open(p_alink_bearer_mqtt);                        // TODO            

        return &p_alink_bearer_mqtt->bearer;
    }
    else if (protocol_type == ALINK_BEARER_MQTT) {
        return NULL;
    }
    else {
        return NULL;
    }
}

int alink_bearer_conect(void)
{
    if (p_alink_bearer_mqtt == NULL) {
        alink_info("bearer no exist");
        return FAIL_RETURN;
    }
    alink_bearer_node_t bearer = p_alink_bearer_mqtt->bearer;

    alink_info("bearer connect");

    return bearer.p_api.bearer_connect(&p_alink_bearer_mqtt->bearer, 20000);
}


void alink_bearer_rx_handle(alink_bearer_node_t *p_bearer_ctx, const char *uri, const char *payload, uint32_t payload_len)
{
    (void)p_bearer_ctx;

    








}

/**
 * 
 */
int alink_bearer_register(void *handle, const char *uri, alink_bearer_rx_cb_t cb)
{
    alink_bearer_node_t *bearer = handle;



    return bearer->p_api.bearer_sub(bearer, uri, cb, 0, 0);
}

/**
 * 
 */
int alink_bearer_send(uint8_t link_id, char *uri, uint8_t *payload, uint32_t len)
{
    /* assert */
    (void)link_id;

    if (p_alink_bearer_mqtt == NULL) {
        return FAIL_RETURN;
    }
    alink_bearer_node_t bearer = p_alink_bearer_mqtt->bearer;

    return bearer.p_api.bearer_pub(&p_alink_bearer_mqtt->bearer, uri, payload, len, 0);
}

/**
 * 
 */
int alink_bearer_yield(uint32_t timeout_ms)
{
    alink_bearer_node_t bearer = p_alink_bearer_mqtt->bearer;

    return bearer.p_api.bearer_yield(&p_alink_bearer_mqtt->bearer, timeout_ms);
}

/**
 * 
 */
int alink_bearer_send_proto(uint8_t protocal, char *uri, uint8_t *payload, uint32_t len)
{
    return 1;
}


int alink_bearer_disconnect(void)
{
    return 0;
}

int alink_bearer_close(void)
{
    return 0;
}
/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_BEARER_H__
#define __ALINK_BEARER_H__

#include "infra_types.h"
#include "infra_defs.h"
#include "iotx_alink_internal.h"

typedef enum {
    ALINK_BEARER_MQTT   = 0x01,
    ALINK_BEARER_COAP   = 0x02,
    ALINK_BEARER_HTTP   = 0x04,
    ALINK_BEARER_HTTP2  = 0x08
} alink_bearer_type_t;

typedef struct _alink_bearer_ctx alink_bearer_ctx_t;


typedef void (*alink_bearer_rx_cb_t)(const char *uri, const char *payload, uint32_t payload_len);   // TODO
typedef void (*alink_bearer_event_cb_t)(const char *uri, const char *payload, uint32_t payload_len);

/* copy from CM, TODO */
typedef int (*alink_bearer_connect_func_t)(alink_bearer_ctx_t *p_bearer_ctx, uint32_t timeout);
typedef int (*alink_bearer_close_func_t)(alink_bearer_ctx_t *p_bearer_ctx);
typedef int (*alink_bearer_yield_func_t)(alink_bearer_ctx_t *p_bearer_ctx, uint32_t timeout);
typedef int (*alink_bearer_sub_func_t)(alink_bearer_ctx_t *p_bearer_ctx, const char *uri, alink_bearer_rx_cb_t topic_handle_func, uint8_t qos, uint32_t timeout);
typedef int (*alink_bearer_unsub_func_t)(alink_bearer_ctx_t *p_bearer_ctx, const char *uri);
typedef int (*alink_bearer_pub_func_t)(alink_bearer_ctx_t *p_bearer_ctx, const char *uri, const uint8_t *payload, uint32_t payload_len, uint8_t qos);


typedef struct {
    alink_bearer_connect_func_t bearer_connect;
    alink_bearer_close_func_t   bearer_close;
    alink_bearer_yield_func_t   bearer_yield;
    alink_bearer_sub_func_t     bearer_sub;
    alink_bearer_unsub_func_t   bearer_unsub;
    alink_bearer_pub_func_t     bearer_pub;
} alink_bearer_api_t;


typedef struct {
    alink_bearer_event_cb_t     bearer_event_cb;
} alink_bearer_cb_t;


struct _alink_bearer_ctx {
    alink_bearer_type_t         type;
    alink_bearer_api_t          p_api;
    alink_bearer_cb_t           p_cb;       // TODO
    void                       *p_handle;
    iotx_cloud_region_types_t   region;     // TODO
};



int alink_bearer_open(void);
int alink_bearer_conect(void);
int alink_bearer_send(alink_bearer_type_t bearer, char *uri, uint8_t *payload, uint32_t len);


#endif /* #ifndef __ALINK_BEARER_H__ */
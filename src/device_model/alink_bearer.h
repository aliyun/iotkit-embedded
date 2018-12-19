/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_BEARER_H__
#define __ALINK_BEARER_H__

#include "iotx_alink_internal.h"
#include "infra_list.h"     // TODO
#include "infra_defs.h"     // TODO


typedef enum {
    ALINK_BEARER_MQTT   = 0x01,
    ALINK_BEARER_COAP   = 0x02,
    ALINK_BEARER_HTTP   = 0x04,
    ALINK_BEARER_HTTP2  = 0x08,
} alink_bearer_type_t;


// #ifndef ALINK_BEARER_MQTT
// #define ALINK_BEARER_MQTT   (0)
// #endif

// #ifndef ALINK_BEARER_COAP
// #define ALINK_BEARER_COAP   (0)
// #endif

// #ifndef ALINK_BEARER_HTTP
// #define ALINK_BEARER_HTTP   (0)
// #endif

// #ifndef ALINK_BEARER_HTTP2
// #define ALINK_BEARER_HTTP2  (0)
// #endif

// #define ALINK_SUPPORT_BEARER_ALL    (ALINK_BEARER_MQTT | ALINK_BEARER_COAP | ALINK_BEARER_HTTP | ALINK_BEARER_HTTP2)


typedef struct _alink_bearer_node alink_bearer_node_t;


typedef void (*alink_bearer_rx_cb_t)(const char *uri, const char *payload, uint32_t payload_len);   // TODO
typedef void (*alink_bearer_event_cb_t)(const char *uri, const char *payload, uint32_t payload_len);

/* copy from CM, TODO */
typedef int (*alink_bearer_connect_func_t)(alink_bearer_node_t *p_bearer_ctx, uint32_t timeout);
typedef int (*alink_bearer_close_func_t)(alink_bearer_node_t *p_bearer_ctx);
typedef int (*alink_bearer_yield_func_t)(alink_bearer_node_t *p_bearer_ctx, uint32_t timeout);
typedef int (*alink_bearer_sub_func_t)(alink_bearer_node_t *p_bearer_ctx, const char *uri, alink_bearer_rx_cb_t topic_handle_func, uint8_t qos, uint32_t timeout);
typedef int (*alink_bearer_unsub_func_t)(alink_bearer_node_t *p_bearer_ctx, const char *uri);
typedef int (*alink_bearer_pub_func_t)(alink_bearer_node_t *p_bearer_ctx, const char *uri, const uint8_t *payload, uint32_t payload_len, uint8_t qos);


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
} alink_bearer_callback_t;


struct _alink_bearer_node {
    list_head_t                 bearer_node;
    alink_bearer_type_t         type;
    alink_bearer_api_t          p_api;
    alink_bearer_callback_t     p_cb;       // TODO
    void                       *p_handle;
};


int alink_bearer_open(alink_bearer_type_t bearer_type, iotx_dev_meta_info_t *dev_info);
int alink_bearer_conect(void);
int alink_bearer_send(uint8_t link_id, char *uri, uint8_t *payload, uint32_t len);


#endif /* #ifndef __ALINK_BEARER_H__ */
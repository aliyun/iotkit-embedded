/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_FORMAT__
#define __ALINK_FORMAT__

#include "infra_types.h"

typedef enum {
    ALINK_URI_UP_REQ_PROPERTY_POST = 0,
    ALINK_URI_UP_RSP_PROPERTY_POST,
    ALINK_URI_UP_RSP_PROPERTY_GET,
    ALINK_URI_UP_REQ_EVENT_POST,
    ALINK_URI_UP_RSP_SERVICE_POST,
    ALINK_URI_UP_REQ_RAW_POST,
    ALINK_URI_UP_RSP_RAW_POST,

    ALINK_URI_UP_REQ_SUBDEV_REGISTER_POST,
    ALINK_URI_UP_REQ_SUBDEV_REGISTER_DELETE,
    ALINK_URI_UP_REQ_SUBDEV_TOPO_POST,
    ALINK_URI_UP_MAX
} alink_msg_uri_index_t;



typedef struct {
    char query_key;
    char value;
} alink_uri_query_item_t;

typedef struct {
    uint32_t id;
    uint32_t code;
    char format;
    char compress;
    char ack;
} alink_uri_query_t;



int alink_format_get_upstream_complete_uri(alink_msg_uri_index_t index, const char *query, char **p_uri);
int alink_format_get_upstream_subdev_complete_url(alink_msg_uri_index_t index, const char *pk, const char *dn, const char *uri_query, char **p_uri);


#endif /* #ifndef __ALINK_FORMAT__ */





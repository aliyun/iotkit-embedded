/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_FORMAT__
#define __ALINK_FORMAT__

#include "infra_types.h"

#define ALINK_URI_PATH_LEN_MAX          50

typedef enum {
    ALINK_URI_UP_REQ_PROPERTY_POST = 0,
    ALINK_URI_UP_RSP_PROPERTY_PUT,
    ALINK_URI_UP_RSP_PROPERTY_GET,
    ALINK_URI_UP_REQ_EVENT_POST,
    ALINK_URI_UP_RSP_SERVICE_PUT,
    
    ALINK_URI_UP_REQ_RAW_POST,
    ALINK_URI_UP_RSP_RAW_PUT,

    ALINK_URI_UP_REQ_SUB_REGISTER_POST,
    ALINK_URI_UP_REQ_SUB_REGISTER_DELETE,

    ALINK_URI_UP_REQ_SUB_LOGIN_POST,
    ALINK_URI_UP_REQ_SUB_LOGIN_DELETE,

    ALINK_URI_UP_REQ_THING_TOPO_POST,
    ALINK_URI_UP_REQ_THING_TOPO_DELETE,
    ALINK_URI_UP_REQ_THING_TOPO_GET,

    ALINK_URI_UP_REQ_SUB_LIST_POST,
    ALINK_URI_UP_RSP_SUB_LIST_PUT,

    ALINK_URI_UP_RSP_GW_PERMIT_PUT,
    ALINK_URI_UP_RSP_GW_CONIFG_PUT,

    ALINK_URI_UP_REQ_DEVINFO_POST,
    ALINK_URI_UP_REQ_DEVINFO_GET,
    ALINK_URI_UP_REQ_DEVINFO_DELETE,

    ALINK_URI_UP_MAX
} alink_msg_uri_index_t;

/**
 * uri query data struct define
 */
typedef struct {
    uint32_t id;
    uint32_t code;
    char format;
    char compress;
    char ack;
} alink_uri_query_t;


int alink_format_get_upstream_complete_uri(alink_msg_uri_index_t index, const char *query, char **p_uri);
int alink_format_get_upstream_subdev_complete_url(alink_msg_uri_index_t index, const char *subdev_pk, const char *subdev_dn, const char *uri_query, char **p_uri);

int alink_format_reslove_uri(const char *uri, uint8_t uri_len, char *pk, char *dn, char *path, alink_uri_query_t *query);
int alink_format_assemble_query(alink_uri_query_t *query, char *query_string, uint8_t query_len);

#endif /* #ifndef __ALINK_FORMAT__ */







/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_FORMAT__
#define __ALINK_FORMAT__

#include "infra_types.h"

typedef enum {
    ALINK_MSG_DIST_TYPE_CLOUD,
    ALINK_MSG_DIST_TYPE_DEVICE,
    ALINK_MSG_DIST_TYPE_NUM
} alink_msg_dist_type_t;

typedef struct {
    alink_msg_dist_type_t   type;
    char                   *dest_str;
} alink_msg_dest_t;

typedef enum {
    ALINK_MSG_TYPE_REQ,
    ALINK_MSG_TYPE_RSP,
    ALINK_MSG_TYPE_NUM
} alink_msg_type_t;

typedef enum {
    ALINK_MSG_LAYER_SYS,
    ALINK_MSG_LAYER_EXT,
    ALINK_MSG_LAYER_PROXY,
    ALINK_MSG_LAYER_NUM
} alink_msg_layer_t;

typedef struct {
    alink_msg_dest_t    dist;
    alink_msg_type_t    type;
    alink_msg_layer_t   layer;
    const char          *path;
    char                *query;
} alink_msg_uri_metadata_t;

int alink_format_assamble_uri(alink_msg_uri_metadata_t *uri_meta, char *uri_output, uint32_t len);

#endif /* #ifndef __ALINK_FORMAT__ */
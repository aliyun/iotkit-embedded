/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"


#define _ALINK_EXTEND_STRING_SPLIT_DELIMITER    "?"
#define _ALINK_EXTEND_STRING_AND_DELIMITER      "&"

#define _ALINK_EXTEND_STRING_ACK                "a"     /* shortname of ack */
#define _ALINK_EXTEND_STRING_CODE               "c"     /* shortname of code */
#define _ALINK_EXTEND_STRING_FORMAT             "f"     /* shortname of format */
#define _ALINK_EXTEND_STRING_ID                 "i"     /* shortname of id */
#define _ALINK_EXTEND_STRING_COMPRESSION        "m"     /* shortname of compression */


const char alink_uri_dest[] = "/c/iot";

const char alink_uri_type_req[] = "req";
const char alink_uri_type_rsp[] = "rsp";

const char alink_uri_layer_sys[] = "sys";
const char alink_uri_layer_ext[] = "ext";
const char alink_uri_layer_proxy[] = "proxy";

const char alink_uri_module_thing[] = "thing";
const char alink_uri_module_subdev[] = "subdev";

const char *alink_uri_type[] = { "req", "rsp" };
const char *alink_uri_layer[] = { "sys", "ext", "proxy" };
const char *alink_uri_module[] = { "thing", "subdev"};
const char *alink_uri_method[] = { "post", "put", "get", "delete", "set", "notify" };


const char *alink_uri_fmt = "/%s/%s/%s/%s/%s";

int alink_format_assamble_uri(alink_msg_uri_metadata_t *uri_meta, char *uri_output, uint32_t len)
{
    ALINK_ASSERT_DEBUG(uri_meta != NULL);        // TODO
    ALINK_ASSERT_DEBUG(uri_meta->dist.dest_str != NULL);
    ALINK_ASSERT_DEBUG(uri_meta->path != NULL);
    ALINK_ASSERT_DEBUG(uri_meta->query != NULL);
    ALINK_ASSERT_DEBUG(uri_output != NULL);

    int res;

    


    res = HAL_Snprintf(uri_output, len, alink_uri_fmt, uri_meta->dist.dest_str, 
                                                 alink_uri_type[uri_meta->type], 
                                                 alink_uri_layer[uri_meta->layer],
                                                 uri_meta->path,
                                                 uri_meta->query);

    return res;
}





int _alink_format_append_extend_string()
{
    return 0;
}

int _alink_format_resolve_extend_string()
{
    return 0;
}

int _alink_format_reslove_uri()
{
    return 0;
}

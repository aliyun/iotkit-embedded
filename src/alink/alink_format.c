/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"


#define _ALINK_EXTEND_STRING_DELIMITER          "?"
#define _ALINK_EXTEND_STRING_AND_DELIMITER      "&"

#define _ALINK_EXTEND_STRING_ACK                "a"     /* shortname of ack */
#define _ALINK_EXTEND_STRING_CODE               "c"     /* shortname of code */
#define _ALINK_EXTEND_STRING_FORMAT             "f"     /* shortname of format */
#define _ALINK_EXTEND_STRING_ID                 "i"     /* shortname of id */
#define _ALINK_EXTEND_STRING_COMPRESSION        "m"     /* shortname of compression */


const char alink_topic_dest[] = "/iot/alink";
const char *alink_topic_type[] = { "req", "rsp" };
const char *alink_topic_layer[] = { "sys", "ext", "proxy" };
const char *alink_topic_method[] = { "post", "put", "get", "delete", "set", "notify" };

#if 0

int _alink_format_append_extend_string()
{

}

int _alink_format_resolve_extend_string()
{

}

int _alink_format_reslove_topic()
{

}

#endif

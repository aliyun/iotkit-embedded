/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"

#include "alink_wrapper.h"


#define _ALINK_EXTEND_STRING_SPLIT_DELIMITER    "?"
#define _ALINK_EXTEND_STRING_AND_DELIMITER      "&"

#define _ALINK_EXTEND_STRING_ACK                "a"     /* shortname of ack */
#define _ALINK_EXTEND_STRING_CODE               "c"     /* shortname of code */
#define _ALINK_EXTEND_STRING_FORMAT             "f"     /* shortname of format */
#define _ALINK_EXTEND_STRING_ID                 "i"     /* shortname of id */
#define _ALINK_EXTEND_STRING_COMPRESSION        "m"     /* shortname of compression */


#define ALINK_URI_DIST_CLOUD        0x00
#define ALINK_URI_DIST_APP          0x10
#define ALINK_URI_DIST_DEVICE       0x20

#define ALINK_URI_ACT_REQ           0x00
#define ALINK_URI_ACT_RSP           0x01

#define ALINK_URI_LAYER_SYS         0x00
#define ALINK_URI_LAYER_EXT         0x10
#define ALINK_URI_LAYER_PROXY       0x20

#define ALINK_URI_METHOD_GET        0x00
#define ALINK_URI_METHOD_POST       0x01
#define ALINK_URI_METHOD_PUT        0x02
#define ALINK_URI_METHOD_DELETE     0x03

typedef struct {
    uint8_t     dist_act;
    uint8_t     layer_method;
    const char *path;
} alink_uri_string_map_t;

const char *alink_uri_dist[] = {
    "/c/iot",
    "/app/%s",
    "/%s/%s"
};

const char *alink_uri_act[] = {
    "/req",
    "/rsp"
};

const char *alink_uri_layer[] = {
    "/sys",
    "/ext",
    "/proxy"
};

const char *alink_uri_method[] = {
    "/get",
    "/post",
    "/put",
    "/delete"
};

const alink_uri_string_map_t c_alink_uri_string_map[] = {
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/dt/property"      },  /* "/c/iot/req/sys/dt/property/post" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_RSP),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/thing/property"   },  /* "/c/iot/rsp/sys/thing/property/post" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_RSP),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_GET),       "/thing/property"   },  /* "/c/iot/rsp/sys/thing/property/get" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/dt/property"      },  /* "/c/iot/req/sys/dt/event/post" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_RSP),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/thing/property"   },  /* "/c/iot/rsp/sys/thing/service/post" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/dt/property"      },  /* "/c/iot/req/sys/dt/raw/post" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_RSP),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/thing/property"   },  /* "/c/iot/rsp/sys/thing/raw/post" */

    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/subdev/register"  },  /* "/c/iot/req/sys/subdev/register/post" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_DELETE),    "/subdev/register"  },  /* "/c/iot/req/sys/subdev/register/delete" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/dt/topo"          },  /* "/c/iot/req/sys/dt/topo/post" */
};

/**
 *
 */
const char *alink_msg_uri_short_alias[] = {
    "1",            /* property post request */
    "2",            /* property set response */
    "3",            /* porperty get response */
    "4",            /* event post request */
    "5",            /* service post response */
    "6",            /* raw data upstream request */
    "7",            /* raw data downstream response */
    "8",            /* subdev reigister */
    "9",            /* subdev unregister */
    "10",           /* subdev add topo */
};

typedef int (*alink_downstream_handle_func_t)(const char *query, const char *pk, const char *dn);

typedef struct {
    const char *uri_string;
    alink_downstream_handle_func_t  handle_func;
} alink_downstream_uri_handle_t;

int test(const char *query, const char *pk, const char *dn)
{
    return 0;
}

const alink_downstream_uri_handle_t c_alink_down_uri_handle_map[] = {
    { "/rsp/sys/dt/property/post",          test},
    { "/req/sys/thing/property/post",       test},
    { "/req/sys/thing/property/get",        test},
    { "/rsp/sys/dt/event/post",             test},
    { "/req/sys/thing/service/post",        test},
    { "/rsp/sys/dt/raw/post",               test},
    { "/req/sys/thing/raw/post",            test},

    { "/rsp/sys/subdev/register/post",      test},
    { "/rsp/sys/subdev/register/delete",    test},
    { "/rsp/sys/dt/topo/post",              test},
};




int alink_format_get_upstream_complete_uri(alink_msg_uri_index_t index, const char *uri_query, char **p_uri)
{
    //int res = FAIL_RETURN;
    uint16_t len;

    const char *uri_dist = alink_uri_dist[(c_alink_uri_string_map[index].dist_act >> 4)];
    const char *uri_act = alink_uri_act[(c_alink_uri_string_map[index].dist_act & 0x0F)];
    const char *uri_layer = alink_uri_layer[(c_alink_uri_string_map[index].layer_method >> 4)];
    const char *uri_path = c_alink_uri_string_map[index].path;
    const char *uri_method = alink_uri_method[(c_alink_uri_string_map[index].layer_method & 0x0F)];

    len = strlen(uri_dist) + strlen(uri_act) + strlen(uri_layer) + strlen(uri_path) + strlen(uri_method) + strlen(uri_query) + 1;

    char *uri = HAL_Malloc(len);
    if (uri == NULL) {
        return FAIL_RETURN;
    }

    HAL_Snprintf(uri, len, "%s%s%s%s%s%s", uri_dist, uri_act, uri_layer, uri_path, uri_method, uri_query);

    *p_uri = uri;

    return SUCCESS_RETURN;
}

/**
 * /c/iot/req/proxy/{subdev-productkey}/{subdev-devicename}/sys/dt/property/post
 */
int alink_format_get_upstream_subdev_complete_url(alink_msg_uri_index_t index, const char *pk, const char *dn, const char *uri_query, char **p_uri)
{
    ALINK_ASSERT_DEBUG(index < ALINK_URI_UP_MAX && index >= 0 );
    ALINK_ASSERT_DEBUG(pk != NULL);
    ALINK_ASSERT_DEBUG(dn != NULL);
    ALINK_ASSERT_DEBUG(uri_query != NULL);
    ALINK_ASSERT_DEBUG(p_uri != NULL);

    uint16_t len;

    const char *uri_dist = alink_uri_dist[(c_alink_uri_string_map[index].dist_act >> 4)];
    const char *uri_act = alink_uri_act[(c_alink_uri_string_map[index].dist_act & 0x0F)];
    const char *uri_layer = alink_uri_layer[(c_alink_uri_string_map[index].layer_method >> 4)];
    const char *uri_path = c_alink_uri_string_map[index].path;
    const char *uri_method = alink_uri_method[(c_alink_uri_string_map[index].layer_method & 0x0F)];

    len = strlen(uri_dist) + strlen(uri_act) + strlen(uri_layer) + strlen(uri_path) + strlen(uri_method) 
        + strlen(uri_query) + strlen(pk) + strlen(dn) + strlen(alink_uri_layer[ALINK_URI_LAYER_PROXY >> 4]) + 3;    /* add 2 "/" delimiter */

    char *uri = HAL_Malloc(len);
    if (uri == NULL) {
        return FAIL_RETURN;
    }

    alink_info("query len = %d", strlen(uri_query));
    alink_info("len = %d", len);

    /* e... - -! */
    HAL_Snprintf(uri, len, "%s%s%s/%s/%s%s%s%s%s", uri_dist, uri_act, 
                alink_uri_layer[ALINK_URI_LAYER_PROXY >> 4], pk, dn, uri_layer, uri_path, uri_method, uri_query);

    *p_uri = uri;

    return SUCCESS_RETURN;
}

/**
 * 
 */
const char *alink_format_get_upstream_alias_uri(alink_msg_uri_index_t index)
{
    return alink_msg_uri_short_alias[index];
}


int _alink_format_append_extend_string()
{
    return 0;
}

int alink_format_resolve_query(const char *uri, uint32_t uri_len, alink_uri_query_t *query, uint8_t *query_len)
{
    const char *p = uri + uri_len;
    uint8_t len = 0;
    char temp[30] = {0};        // TODO, malloc

    while (--p != uri) {
        len++;
        if (*p == '?') {
            break;
        }
        else if (*p == '/') {
            return FAIL_RETURN;
        }
    }

    *query_len = len;
    

    if (len >= 30) {      // TODO
        return FAIL_RETURN;
    }

    memcpy(temp, p, len);
    alink_info("query = %s", temp);

    uint8_t i = 0;

    while (i++ < len) {
        switch (temp[i]) {
            case 'f': {
                i += 2;
                query->format = temp[i]; 
            } break;
            case 'i': {
                i += 2;
                query->id = atoi(temp+i);
            } break;
            case 'c': {
                i += 2;
                query->compress = temp[i]; 
            } break;
            case 'a': {
                i += 2;
                query->ack =  temp[i]; 
            }
            case 'r': {
                i += 2;
                query->code = atoi(temp+i);                
            }
            default: continue;
        }
    }

    return 0;
}

int _alink_format_reslove_uri()
{
    return 0;
}

/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"


#define _ALINK_QUERY_SPLIT_DELIMITER    "?"
#define _ALINK_QUERY_AND_DELIMITER      "&"

#define _ALINK_QUERY_KEY_ACK                "a"     /* shortname of ack */
#define _ALINK_QUERY_KEY_CODE               "r"     /* shortname of code */
#define _ALINK_QUERY_KEY_FORMAT             "f"     /* shortname of format */
#define _ALINK_QUERY_KEY_ID                 "i"     /* shortname of id */
#define _ALINK_QUERY_KEY_COMPRESSION        "c"     /* shortname of compression */

#define QUERY_STRING_ID_LEN_MAX         15
#define QUERY_STRING_CODE_LEN_MAX       14
#define QUERY_STRING_LEN_MAX         45


#define ALINK_URI_DIST_CLOUD        0x00
#define ALINK_URI_DIST_APP          0x10
#define ALINK_URI_DIST_DEVICE       0x20

#define ALINK_URI_ACT_REQ           0x00
#define ALINK_URI_ACT_RSP           0x01

#define ALINK_URI_LAYER_SYS         0x00
#define ALINK_URI_LAYER_EXT         0x10

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
};

const char *alink_uri_method[] = {
    "/get",
    "/post",
    "/put",
    "/delete"
};

const alink_uri_string_map_t c_alink_uri_string_map[] = {
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/thing/property"       },  /* "/c/iot/req/sys/thing/property/post" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_RSP),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_PUT),       "/thing/property"       },  /* "/c/iot/rsp/sys/thing/property/put" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_RSP),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_GET),       "/thing/property"       },  /* "/c/iot/rsp/sys/thing/property/get" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/thing/event"          },  /* "/c/iot/req/sys/thing/event/post" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_RSP),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_PUT),       "/thing/service"        },  /* "/c/iot/rsp/sys/thing/service/put" */

    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/thing/raw"            },  /* "/c/iot/req/sys/thing/raw/post" */
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_RSP),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_PUT),       "/thing/raw"            },  /* "/c/iot/rsp/sys/thing/raw/post" */

    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/sub/register"         },
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_DELETE),    "/sub/register"         },

    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/sub/login"            },  
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_DELETE),    "/sub/login"            },  
    
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/thing/topo"           }, 
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_DELETE),    "/thing/topo"           },  
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_GET),       "/thing/topo"           }, 

    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/sub/list"             },  
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_RSP),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_PUT),       "/sub/list"             },  

    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_RSP),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_PUT),       "/gw/permit"            },  
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_RSP),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_PUT),       "/gw/config"            },  

    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_POST),      "/dev/tag"        }, 
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_GET),       "/dev/tag"        }, 
    { (ALINK_URI_DIST_CLOUD | ALINK_URI_ACT_REQ),   (ALINK_URI_LAYER_SYS | ALINK_URI_METHOD_DELETE),    "/dev/tag"        }, 
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

/**********************************
 * local function
 **********************************/


/**********************************
 * global function
 **********************************/
int alink_format_get_upstream_complete_uri(alink_msg_uri_index_t index, const char *uri_query, char **p_uri)
{
    /* int res = FAIL_RETURN; */
    uint16_t len;
    char *uri;

    const char *uri_dist = alink_uri_dist[(c_alink_uri_string_map[index].dist_act >> 4)];
    const char *uri_act = alink_uri_act[(c_alink_uri_string_map[index].dist_act & 0x0F)];
    const char *uri_layer = alink_uri_layer[(c_alink_uri_string_map[index].layer_method >> 4)];
    const char *uri_path = c_alink_uri_string_map[index].path;
    const char *uri_method = alink_uri_method[(c_alink_uri_string_map[index].layer_method & 0x0F)];

    len = strlen(uri_dist) + strlen(uri_act) + strlen(uri_layer) + strlen(uri_path) + strlen(uri_method) + strlen(uri_query) + 1;

    uri = alink_malloc(len);
    if (uri == NULL) {
        return FAIL_RETURN;
    }

    HAL_Snprintf(uri, len, "%s%s%s%s%s%s", uri_dist, uri_act, uri_layer, uri_path, uri_method, uri_query);

    *p_uri = uri;

    return SUCCESS_RETURN;
}

/**
 * /c/iot/req/proxy/{subdev-productkey}/{subdev-devicename}/sys/thing/property/post
 */
int alink_format_get_upstream_subdev_complete_url(alink_msg_uri_index_t index, const char *subdev_pk, const char *subdev_dn, const char *uri_query, char **p_uri)
{
    uint16_t len;
    char *uri;
    const char *uri_dist;
    const char *uri_act;
    const char *uri_layer;
    const char *uri_path;
    const char *uri_method;

    ALINK_ASSERT_DEBUG(index < ALINK_URI_UP_MAX);
    ALINK_ASSERT_DEBUG(subdev_pk != NULL);
    ALINK_ASSERT_DEBUG(subdev_dn != NULL);
    ALINK_ASSERT_DEBUG(uri_query != NULL);
    ALINK_ASSERT_DEBUG(p_uri != NULL);

    uri_dist = alink_uri_dist[(c_alink_uri_string_map[index].dist_act >> 4)];
    uri_act = alink_uri_act[(c_alink_uri_string_map[index].dist_act & 0x0F)];
    uri_layer = alink_uri_layer[(c_alink_uri_string_map[index].layer_method >> 4)];
    uri_path = c_alink_uri_string_map[index].path;
    uri_method = alink_uri_method[(c_alink_uri_string_map[index].layer_method & 0x0F)];

    len = strlen(uri_dist) + strlen(uri_act) + strlen(uri_layer) + strlen(uri_path) + strlen(uri_method) 
        + strlen(uri_query) + strlen(subdev_pk) + strlen(subdev_dn) + strlen(alink_uri_layer[ALINK_URI_LAYER_EXT >> 4]) + 9;    /* add 2 "/" delimiter strlen of "/proxy" */

    uri = alink_malloc(len);
    if (uri == NULL) {
        return FAIL_RETURN;
    }

    alink_info("query len = %d", strlen(uri_query));
    alink_info("len = %d", len);

    /* e... - -! */
    HAL_Snprintf(uri, len, "%s%s%s/proxy/%s/%s%s%s%s%s", uri_dist, uri_act, 
                alink_uri_layer[ALINK_URI_LAYER_EXT >> 4], subdev_pk, subdev_dn, uri_layer, uri_path, uri_method, uri_query);

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

/** assemble query string **/
int alink_format_assemble_query(alink_uri_query_t *query, char *query_string, uint8_t query_len)
{
    char query_id[QUERY_STRING_ID_LEN_MAX] = {0};
    char query_code[QUERY_STRING_CODE_LEN_MAX] = {0};
    char query_format[5] = {0};
    char query_compress[5] = {0};
    char query_ack[5] = {0};
    uint8_t len = 0;

    ALINK_ASSERT_DEBUG(query != NULL);
    ALINK_ASSERT_DEBUG(query_string != NULL);

    /* query_id always exist */
    HAL_Snprintf(query_id, sizeof(query_id), "/?i=%d", query->id);
    len += strlen(query_id) + 1;
    if (query->code != 0) {
        HAL_Snprintf(query_code, sizeof(query_code), "&r=%d", query->code);
        len += strlen(query_code);
    }
    if (query->format != 0) {
        HAL_Snprintf(query_format, sizeof(query_format), "&f=%c", query->format);
        len += strlen(query_format);
    }
    if (query->compress != 0) {
        HAL_Snprintf(query_compress, sizeof(query_compress), "&c=%c", query->compress);
        len += strlen(query_compress);
    }
    if (query->ack != 0) {
        HAL_Snprintf(query_ack, sizeof(query_ack), "&a=%c", query->ack);
        len += strlen(query_ack);
    }

    if (len > query_len) {
        alink_err("query string assemble error");
        return FAIL_RETURN;
    }

    HAL_Snprintf(query_string, query_len, "%s%s%s%s%s", 
                query_id, query_code, query_format, query_compress, query_ack);
    return SUCCESS_RETURN;
}

int alink_format_resolve_query(const char *uri, uint8_t *uri_len, alink_uri_query_t *query)
{
    const char *p = uri + *uri_len;
    uint8_t len = 0;
    uint8_t i = 0;
    char temp[QUERY_STRING_LEN_MAX] = {0};

    while (--p != uri) {
        len++;
        if (*p == '?') {
            break;
        }
        else if (*p == '/') {
            return FAIL_RETURN;
        }
    }
    
    if (len >= QUERY_STRING_LEN_MAX) {
        return FAIL_RETURN;
    }

    *uri_len -= (len+1);    /* query_len not include '/' */
    memcpy(temp, p, len);

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
            } break;
            case 'r': {
                i += 2;
                query->code = atoi(&temp[i]);
            }
            default: continue;
        }
    }

    /* set default value if value absence */
    if (query->ack == '\0') {
        query->ack = 'y';
    }
    if (query->format == '\0') {
        query->format = 'j';
    }

    return 0;
}

/**
 * no include char '/', level start from 1, not support the last level, todo, overflow!!!
 */
int _alink_get_uri_level_value(const char *uri, uint8_t uri_len, uint8_t level, char *value, uint32_t value_len)
{
    uint8_t idx = 0;
    uint8_t cnt = 0;
    char *p1 = NULL;
    char *p2 = NULL;
    uint32_t len;

    ALINK_ASSERT_DEBUG(uri != NULL);
    ALINK_ASSERT_DEBUG(uri_len != 0);
    ALINK_ASSERT_DEBUG(value != NULL);

    for (idx = 0; idx < uri_len; idx++) {
        if (*(uri+idx) == '/' && (idx + 1) < uri_len) {
            cnt++;
            if (cnt == level) {
                p1 = (char *)(uri + idx + 1);
            } 
            else if (cnt == level+1) {
                p2 = (char *)(uri + idx + 1);
            }
        }
    }

    if (!p1 || !p2 ) {
        return FAIL_RETURN;
    }    

    len = p2-p1-1;
    if (len >= value_len) {
        alink_err("get uri level value error");
        return FAIL_RETURN;
    }

    memcpy(value, p1, len); 
    value[p2-p1-1] = 0;

    return SUCCESS_RETURN;
}


/**
 * not include '/'
 */
int _alink_get_uri_level_pointer(const char *uri, uint8_t uri_len, uint8_t level, char **p_value)
{
    uint8_t idx = 0;
    uint8_t cnt = 0;

    ALINK_ASSERT_DEBUG(uri != NULL);
    ALINK_ASSERT_DEBUG(uri_len != 0);
    ALINK_ASSERT_DEBUG(p_value != NULL);

    for (idx = 0; idx < uri_len; idx++) {
        if (*(uri+idx) == '/' && (idx + 1) < uri_len) {
            cnt++;
            if (cnt == level) {
                *p_value = (char *)(uri + idx + 1);

                return SUCCESS_RETURN;
            }
        }
    }

    return FAIL_RETURN;
}

/**
 * TODO, not good!!!
 */
int alink_format_reslove_uri(const char *uri, uint8_t uri_len, char *pk, char *dn, char *path, alink_uri_query_t *query)
{
    char value[20] = {0};
    uint8_t value_len = 0;
    char *p = NULL;
    uint8_t uri_len_temp = uri_len;

    alink_format_resolve_query(uri, &uri_len_temp, query);

    /* check if ext/proxy */
    _alink_get_uri_level_value(uri, uri_len_temp, 5, value, sizeof(value));
    value_len = strlen(value);

    if (value_len == strlen("proxy") && !memcmp(value, "proxy", value_len))
    {
        _alink_get_uri_level_value(uri, uri_len_temp, 6, pk, IOTX_PRODUCT_KEY_LEN);
        _alink_get_uri_level_value(uri, uri_len_temp, 7, dn, IOTX_DEVICE_NAME_LEN);

        _alink_get_uri_level_value(uri, uri_len_temp, 3, path, ALINK_URI_PATH_LEN_MAX);
        memcpy(path+strlen(path), "/", 1);

        _alink_get_uri_level_pointer(uri, uri_len_temp, 8, &p);
        memcpy(path+strlen(path), p, (uri_len_temp - (uint8_t)(p - uri)));
    }
    else {
        /* is not subdev uri, just ignore the pk and dn */
        _alink_get_uri_level_pointer(uri, uri_len_temp, 3, &p);
        memcpy(path, p, (uri_len_temp - (uint8_t)(p - uri)));
    }

    return SUCCESS_RETURN;
}



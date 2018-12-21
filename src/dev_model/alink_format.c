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
} alink_uri_handle_pair_t;

int test(const char *query, const char *pk, const char *dn)
{
    alink_info("%s\r\n", query);
    return 0;
}

const alink_uri_handle_pair_t c_alink_down_uri_handle_map[] = {
    { "/rsp/sys/dt/property/post",  test    },
    { "/req/sys/thing/property/post",   test    },
    { "/req/sys/thing/property/get",    test    },
    { "/rsp/sys/dt/event/post", test    },
    { "/req/sys/thing/service/post",    test    },
    { "/rsp/sys/dt/raw/post",   test    },
    { "/req/sys/thing/raw/post",    test    },

    { "/rsp/sys/subdev/register/post",  test    },
    { "/rsp/sys/subdev/register/delete",    test    },
    { "/rsp/sys/dt/topo/post",  test    },
    { "/rsp/sys/dt/topo/delete",    test    },
    { "/rsp/sys/dt/topo/get",   test    },
    { "/req/sys/subdev/topo/post",  test    },
    { "/rsp/sys/subdev/login/post", test    },
    { "/rsp/sys/subdev/logout/post",    test    },
    { "/rsp/sys/dt/list/post",  test    },
    { "/req/sys/subdev/permit/post",    test    },
    { "/req/sys/subdev/config/post",    test    },
    { "/rsp/sys/dt/deviceinfo/post",    test    },
    { "/rsp/sys/dt/deviceinfo/get", test    },
    { "/rsp/sys/dt/deviceinfo/delete",  test    },
};

/**********************************
 * local function
 **********************************/
#define ALINK_URI_MAX_LEN           50
#define MAX_HASH_TABLE_SIZE         37


typedef struct _hash_node {
    alink_uri_handle_pair_t pair;
    struct _hash_node *next;
} uri_hash_node_t;

uri_hash_node_t *uri_hash_table[MAX_HASH_TABLE_SIZE] = { NULL };


static uint8_t _uri_to_hash(const char *uri)
{
    uint8_t i, nameLen;
    uint32_t sum = 0;

    nameLen = strlen (uri);

    /* Sum the ascii values of the header names */
    for (i = 0; i < nameLen; i++)
    {
        sum += uri[i];
    }

    /* Sum the rest of the length until we get to the maximum length */
    for (; i < ALINK_URI_MAX_LEN; i++)
    {
        sum++;
    }

    sum += nameLen;
    sum = sum % MAX_HASH_TABLE_SIZE;

    if (0 == sum) {
        sum = 1;
    }

    return sum;
}

uri_hash_node_t *_uri_hash_node_malloc(alink_uri_handle_pair_t pair)
{
    uri_hash_node_t *node = HAL_Malloc(sizeof(uri_hash_node_t));
    if (node == NULL) {
        return NULL;
    }

    node->pair = pair;  // TODO
    node->next = NULL;
    
    return node;
}

static int _uri_hash_insert(alink_uri_handle_pair_t pair) {

    uint8_t hash = _uri_to_hash(pair.uri_string);

    uri_hash_node_t *node = HAL_Malloc(sizeof(uri_hash_node_t));
    if (node == NULL) {
        return FAIL_RETURN;
    }
    node->pair = pair;  // TODO
    node->next = NULL;

    if (uri_hash_table[hash] == NULL) {
        uri_hash_table[hash] = node;
    }
    else {
        uri_hash_node_t *search_node = uri_hash_table[hash];
        while (search_node->next) {
            search_node = search_node->next;
        }

        search_node->next = node;
    }

    return SUCCESS_RETURN;
}

static uri_hash_node_t * _uri_hash_search(const char *uri_string)
{
    uint16_t str_len = strlen(uri_string);
    uint8_t hash = _uri_to_hash(uri_string);

    uri_hash_node_t *node = uri_hash_table[hash];

    // TEST
    //uint8_t i = 0;

    while (node) {
        //alink_info("hash search time %d", ++i);
        if (str_len == strlen(node->pair.uri_string) && !memcmp(uri_string, node->pair.uri_string, str_len)) {
            return node;
        }
        else {
            node = node->next;
        }
    }

    return NULL;
}

/* _uri_hash_iterator(); */


int alink_format_handle_uri(const char *uri_string)
{
    uri_hash_node_t *search_node = _uri_hash_search(uri_string);

    search_node->pair.handle_func(search_node->pair.uri_string, "2", "3");

    return 1;
}



/**********************************
 * global function
 **********************************/
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
    uint8_t i = 0;
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
                query->code = atoi(temp+i);     // atoi used
            }
            default: continue;
        }
    }

    return 0;
}

int alink_format_create_hash_table(void)
{
    uint8_t i;
    uint8_t hash_temp;

    for (i=0; i<(sizeof(c_alink_down_uri_handle_map)/sizeof(alink_uri_handle_pair_t)); i++) {
        hash_temp = _uri_to_hash(c_alink_down_uri_handle_map[i].uri_string);
        alink_info("[%d] = %d", i, hash_temp);
        _uri_hash_insert(c_alink_down_uri_handle_map[i]);
    }

    return FAIL_RETURN;
}

int _alink_format_reslove_uri()
{
    return 0;
}

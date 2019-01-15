/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"

/****************************************
 * local macro define
 ****************************************/
/* element number of uri/handler pair map */
#define ALINK_URI_HANDLE_PAIR_NUM           (sizeof(c_alink_down_uri_handle_map)/sizeof(alink_uri_handle_pair_t))
#define HASH_TABLE_SIZE_MAX                 23 /* 13, 17, 19 ,23, 29, 31, 37, 41, 43, 47, 53, 59*/
#define ALINK_URI_MAX_LEN                   50

typedef struct _hash_node *uri_hash_table_t;

typedef struct {
    const char *uri_string;
    alink_downstream_handle_func_t  handle_func;
} alink_uri_handle_pair_t;

typedef struct _hash_node {
    const alink_uri_handle_pair_t *pair;
    struct _hash_node *next;
} uri_hash_node_t;

/****************************************
 * local function prototypes
 ****************************************/
static void _empty_rsp_handle(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

/** all handlers of downstream uri below */
static void alink_downstream_thing_property_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_property_put_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_property_get_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_event_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_service_invoke_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

static void alink_downstream_thing_raw_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_raw_put_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

static void alink_downstream_thing_deviceinfo_post_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_deviceinfo_get_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_deviceinfo_delete_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

#ifdef DEVICE_MODEL_GATEWAY
static void alink_downstream_subdev_register_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_subdev_unregister_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

static void alink_downstream_subdev_login_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_subdev_logout_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

static void alink_downstream_subdev_topo_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_subdev_topo_delete_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_subdev_topo_get_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

static void alink_downstream_subdev_list_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_subdev_list_put_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

static void alink_downstream_subdev_permit_post_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_subdev_config_post_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
#endif


/****************************************
 * local variables define
 ****************************************/
/** uri/handle pair map **/
const alink_uri_handle_pair_t c_alink_down_uri_handle_map[] = {
    { "rsp/sys/thing/property/post",                alink_downstream_thing_property_post_rsp        },
    { "req/sys/thing/property/put",                 alink_downstream_thing_property_put_req         },
    { "req/sys/thing/property/get",                 alink_downstream_thing_property_get_req         },
    { "rsp/sys/thing/event/post",                   alink_downstream_thing_event_post_rsp           },
    { "req/sys/thing/service/put",                  alink_downstream_thing_service_invoke_req       },

    { "rsp/sys/thing/raw/post",                     alink_downstream_thing_raw_post_rsp             },
    { "req/sys/thing/raw/put",                      alink_downstream_thing_raw_put_req              },

    { "rsp/sys/dev/tag/post",                       alink_downstream_thing_deviceinfo_post_rsq      },
    { "rsp/sys/dev/tag/get",                        alink_downstream_thing_deviceinfo_get_rsq       },
    { "rsp/sys/dev/tag/delete",                     alink_downstream_thing_deviceinfo_delete_rsq    },
#ifdef DEVICE_MODEL_GATEWAY
    { "rsp/sys/sub/register/post",                  alink_downstream_subdev_register_post_rsp       },
    { "rsp/sys/sub/register/delete",                alink_downstream_subdev_unregister_post_rsp     },
    { "rsp/sys/sub/login/post",                     alink_downstream_subdev_login_post_rsp          },
    { "rsp/sys/sub/login/delete",                   alink_downstream_subdev_logout_post_rsp         },
    { "rsp/sys/thing/topo/post",                    alink_downstream_subdev_topo_post_rsp           },
    { "rsp/sys/thing/topo/delete",                  alink_downstream_subdev_topo_delete_rsp         },
    { "rsp/sys/thing/topo/get",                     alink_downstream_subdev_topo_get_rsp            },

    { "rsp/sys/sub/list/post",                      alink_downstream_subdev_list_post_rsp           },
    { "req/sys/sub/list/put",                       alink_downstream_subdev_list_put_req            },

    { "req/sys/gw/permit/put",                      alink_downstream_subdev_permit_post_req         },
    { "req/sys/gw/config/put",                      alink_downstream_subdev_config_post_req         },
#endif
};

/** alink protocal uri hash table */
static uri_hash_table_t g_uri_hash_table[HASH_TABLE_SIZE_MAX] = { NULL };

/** alink protocal format key define **/
const char alink_proto_key_params[] = "params";
const char alink_proto_key_property[] = "p";
const char alink_proto_key_id[] = "id";
const char alink_proto_key_subList[] = "subList";
const char alink_proto_key_productKey[] = "pk";
const char alink_proto_key_deviceName[] = "dn";
const char alink_proto_key_deviceSecret[] = "ds";
const char alink_proto_key_code[] = "code";
const char alink_proto_key_data[] = "data";

const char alink_proto_key_timeoutSec[] = "timeoutSec";
const char alink_proto_key_url[] = "url";


#if (CONFIG_SDK_THREAD_COST == 1)

#define ALINK_MSG_LIST_DEINITED                 0x00
#define ALINK_MSG_LIST_INITED                   0x01
#define ALINK_MSG_LIST_NUM_MAX                  50

#define ALINK_EVENT_PROPERTY_PUT_REQ            0x01
#define ALINK_EVENT_PROPERTY_GET_REQ            0x02
#define ALINK_EVENT_EMPTY_RSP                   0x03
#define ALINK_EVENT_SERVICE_PUT_REQ             0x04
#define ALINK_EVENT_RAW_DATA_REQ                0x05

#define ALINK_EVENT_SUB_REGISTER_RSP            0x10
#define ALINK_EVENT_SUB_UNREGISTER_RSP          0x11
#define ALINK_EVENT_SUB_LOGIN_RSP               0x12
#define ALINK_EVENT_SUB_LOGOUT_RSP              0x13

typedef struct {
    char *payload;
    uint32_t len;
} msg_property_put_req_t;

typedef struct {
    char *payload;
    uint32_t len;
} msg_property_get_req_t;

typedef struct {
    char *payload;
    uint32_t len;
} msg_empty_rsp_t;

typedef struct {
    char *id;
    char *payload;
    uint32_t payload_len;
    uint8_t id_len;
} msg_service_put_req_t;

typedef struct {
    uint8_t *data;
    uint32_t len;
} msg_raw_data_req_t;

typedef union {
    msg_property_put_req_t property_put_req;
    msg_property_get_req_t property_get_req;
    msg_empty_rsp_t empty_rsp;
    msg_service_put_req_t service_put_req;
    msg_raw_data_req_t raw_data;
} alink_msg_t;

typedef struct {
    uint32_t devid;
    uint8_t type;
    alink_msg_t msg;
    alink_uri_query_t query;
    list_head_t list;
} alink_msg_event_t;

typedef struct {
    void *mutex;
    uint8_t status;
    uint32_t msg_num;
    list_head_t msg_list;
} alink_downstream_ctx_t;

alink_downstream_ctx_t alink_msg_list_ctx = {0};

static void _alink_msg_list_lock(void)
{
    if (alink_msg_list_ctx.mutex) {
        HAL_MutexLock(alink_msg_list_ctx.mutex);
    }
}

static void _alink_msg_list_unlock(void)
{
    if (alink_msg_list_ctx.mutex) {
        HAL_MutexUnlock(alink_msg_list_ctx.mutex);
    }
}

static void _alink_msg_list_remove(alink_msg_event_t *msg)
{
    void *p_data = NULL;

    ALINK_ASSERT_DEBUG(msg != NULL);

    switch (msg->type) {
        case ALINK_EVENT_PROPERTY_PUT_REQ: {
            p_data = msg->msg.property_put_req.payload;
        } break;

        case ALINK_EVENT_PROPERTY_GET_REQ: {
            /* implement next versin */
        } break;

        case ALINK_EVENT_EMPTY_RSP: {
            p_data = msg->msg.empty_rsp.payload;
        } break;

        case ALINK_EVENT_SERVICE_PUT_REQ: {
            alink_free(msg->msg.service_put_req.id);
            p_data = msg->msg.service_put_req.payload;
        } break;

        case ALINK_EVENT_RAW_DATA_REQ: {
            p_data = msg->msg.raw_data.data;
        }
        default: break;
    }

    if (p_data != NULL) {
        alink_free(p_data);
    }
    alink_free(msg);
}

int alink_msg_list_init(void)
{
    alink_msg_list_ctx.mutex = HAL_MutexCreate();
    if (alink_msg_list_ctx.mutex == NULL) {
        return IOTX_CODE_CREATE_MUTEX_FAILED;
    }

    alink_msg_list_ctx.status = ALINK_MSG_LIST_INITED;
    alink_msg_list_ctx.msg_num = 0;
    INIT_LIST_HEAD(&alink_msg_list_ctx.msg_list);
    return SUCCESS_RETURN;
}

void alink_msg_list_deinit(void)
{
    alink_msg_event_t *node, *next;

    _alink_msg_list_lock();

    list_for_each_entry_safe(node, next, &alink_msg_list_ctx.msg_list, list, alink_msg_event_t) {
        list_del(&node->list);
        /* release node case by case */
        _alink_msg_list_remove(node);
    }

    alink_msg_list_ctx.status = ALINK_MSG_LIST_DEINITED;
    alink_msg_list_ctx.msg_num = 0;
    INIT_LIST_HEAD(&alink_msg_list_ctx.msg_list);

    _alink_msg_list_unlock();

    if (alink_msg_list_ctx.mutex) {
        HAL_MutexDestroy(alink_msg_list_ctx.mutex);
    }
}

int alink_msg_list_insert(alink_msg_event_t *msg_type)
{
    if (alink_msg_list_ctx.status == ALINK_MSG_LIST_DEINITED) {
        return FAIL_RETURN;
    }

    _alink_msg_list_lock();

    if (alink_msg_list_ctx.msg_num >= ALINK_MSG_LIST_NUM_MAX) {
        /* msg event lost, TODO: stats lost rate */
        _alink_msg_list_unlock();
        return FAIL_RETURN;
    }

    list_add_tail(&msg_type->list, &alink_msg_list_ctx.msg_list);
    alink_msg_list_ctx.msg_num++;

    alink_info("insert successfully, %d", msg_type->type);


    _alink_msg_list_unlock();

    return SUCCESS_RETURN;
}

int alink_msg_list_next(alink_msg_event_t **msg)
{
    alink_msg_event_t *node;

    if (alink_msg_list_ctx.status == ALINK_MSG_LIST_DEINITED) {
        return FAIL_RETURN;
    }

    _alink_msg_list_lock();

    if (list_empty(&alink_msg_list_ctx.msg_list)) {
        _alink_msg_list_unlock();
        return FAIL_RETURN;
    }

    node = list_first_entry(&alink_msg_list_ctx.msg_list, alink_msg_event_t, list);
    /* delete this node as it will be consumed */
    list_del(&node->list);
    alink_msg_list_ctx.msg_num--;

    *msg = node;
    _alink_msg_list_unlock();
    return SUCCESS_RETURN;
}

uint32_t alink_msg_list_get_num(void)
{
    uint32_t num = 0;

    _alink_msg_list_lock();
    num = alink_msg_list_ctx.msg_num;
    _alink_msg_list_unlock();

    return num;
}

int alink_msg_event_list_handler(void)
{
    int res;
    alink_msg_event_t *msg;

    while (1) {
        res = alink_msg_list_next(&msg);
        if (res < SUCCESS_RETURN) {
            return FAIL_RETURN;
        }

        switch (msg->type) {
            case ALINK_EVENT_PROPERTY_PUT_REQ: {
                linkkit_property_set_cb_t handle_func;
                msg_property_put_req_t *msg_data = &(msg->msg.property_put_req);

                /* invoke the user callback funciton */
                handle_func = (linkkit_property_set_cb_t)alink_get_event_callback(ITE_PROPERTY_SET);
                if (handle_func != NULL) {
                    res = handle_func(msg->devid, (const char *)msg_data->payload, msg_data->len);
                }

                /* send response if ack is y */
                if (msg->query.ack == 'y') {
                    char pk[IOTX_PRODUCT_KEY_LEN] = {0};
                    char dn[IOTX_DEVICE_NAME_LEN] = {0};

#ifdef DEVICE_MODEL_GATEWAY
                    /* quite embarrassed, i have to get pk,dn again */
                    if (msg->devid != 0) {
                        alink_subdev_get_pkdn_by_devid(msg->devid, pk, dn);
                    }
#endif
                    msg->query.code = (res == SUCCESS_RETURN) ? ALINK_ERROR_CODE_200: ALINK_ERROR_CODE_400;

                    alink_upstream_thing_property_set_rsp(pk, dn, &msg->query);
                }

                /* release the msg */
                alink_free(msg->msg.property_put_req.payload);
                alink_free(msg);
            } break;

            case ALINK_EVENT_PROPERTY_GET_REQ: {
                /* implement next versin */
            } break;

            /* empty response event */
            case ALINK_EVENT_EMPTY_RSP: {
                linkkit_report_reply_cb_t handle_func;
                msg_empty_rsp_t *msg_data = &(msg->msg.empty_rsp);

                handle_func = (linkkit_report_reply_cb_t)alink_get_event_callback(ITE_REPORT_REPLY);
                if (handle_func != NULL) {
                    handle_func(msg->devid, msg->query.id, msg->query.code, msg_data->payload, msg_data->len);
                }

                alink_free(msg_data->payload);
                alink_free(msg);
            } break;

            case ALINK_EVENT_SERVICE_PUT_REQ: {
                char *rsp_data = NULL;
                uint32_t rsp_len;
                linkkit_service_request_cb_t handle_func;
                /* just invoke the user callback funciton */
                handle_func = (linkkit_service_request_cb_t)alink_get_event_callback(ITE_SERVICE_REQUEST);
                if (handle_func != NULL) {
                    res = handle_func(msg->devid, msg->msg.service_put_req.id, msg->msg.service_put_req.id_len,
                                        msg->msg.service_put_req.payload, msg->msg.service_put_req.payload_len, &rsp_data, &rsp_len);
                }

                alink_debug("propery get user rsp = %.*s", rsp_len, rsp_data);
                msg->query.code = (res == SUCCESS_RETURN) ? ALINK_ERROR_CODE_200: ALINK_ERROR_CODE_400;

                /* send upstream response if ack is need */
                if (msg->query.ack == 'y') {
                    char pk[IOTX_PRODUCT_KEY_LEN] = {0};
                    char dn[IOTX_DEVICE_NAME_LEN] = {0};

#ifdef DEVICE_MODEL_GATEWAY
                    /* quite embarrassed, i have to get pk,dn again */
                    if (msg->devid != 0) {
                        alink_subdev_get_pkdn_by_devid(msg->devid, pk, dn);
                    }
#endif
                    alink_upstream_thing_service_invoke_rsp(pk, dn, msg->msg.service_put_req.id, rsp_data, rsp_len, &msg->query);
                }
                if (rsp_data != NULL) {
                    alink_free(rsp_data);
                }
                alink_free(msg->msg.service_put_req.id);
                alink_free(msg->msg.service_put_req.payload);
                alink_free(msg);
            } break;

            case ALINK_EVENT_RAW_DATA_REQ: {
                /* just invoke the user callback funciton */
                linkkit_rawdata_rx_cb_t handle_func;
                handle_func = (linkkit_rawdata_rx_cb_t)alink_get_event_callback(ITE_RAWDATA_ARRIVED);
                if (handle_func != NULL) {
                    handle_func(msg->devid, msg->msg.raw_data.data, msg->msg.raw_data.len);
                }

                /* release the msg */
                if (msg->msg.raw_data.len > 0) {
                    alink_free(msg->msg.raw_data.data);
                }
                alink_free(msg);
            } break;

            default: {
                alink_err("unknown msg event recv");
                /* release the msg, memory leak may happen!!! */
                alink_free(msg);
            } break;
        }
    }

    return SUCCESS_RETURN;
}
#endif

/***************************************************************
 * uri handler hash table management
 ***************************************************************/
static uint8_t _uri_to_hash(const char *uri, uint32_t uri_len)
{
    uint32_t i;
    uint32_t sum = 0;

    for (i = 0; i < uri_len; i++)
    {
        sum += uri[i];
    }

    for (; i < ALINK_URI_MAX_LEN; i++)
    {
        sum++;
    }

    sum += uri_len;
    sum = sum % HASH_TABLE_SIZE_MAX;

    return sum;
}

static int _uri_hash_insert(const alink_uri_handle_pair_t *pair, uri_hash_table_t *table)
{
    uint8_t hash = _uri_to_hash(pair->uri_string, strlen(pair->uri_string));
    uri_hash_node_t *node, *search_node;

    node = alink_malloc(sizeof(uri_hash_node_t));
    if (node == NULL) {
        return FAIL_RETURN;
    }
    node->pair = pair;
    node->next = NULL;

    if (table[hash] == NULL) {
        table[hash] = node;
    }
    else {
        search_node = table[hash];
        while (search_node->next) {
            search_node = search_node->next;
        }
        search_node->next = node;
    }

    return SUCCESS_RETURN;
}

#if UTILS_HASH_TABLE_ITERATOR_ENABLE
void _uri_hash_iterator(uri_hash_table_t *table)
{
    uri_hash_node_t *node;
    uint8_t idx;

    ALINK_ASSERT_DEBUG(table != NULL);

    for (idx = 0; idx < HASH_TABLE_SIZE_MAX; idx++) {
        if (table[idx] == NULL ) {
            alink_debug("hTable[%d] = NULL", idx);
            continue;
        }
        else {
            node = table[idx];
            alink_debug("hTable[%d] = %s", idx, node->pair->uri_string);

            while (node->next) {
                node = node->next;
                alink_debug("hTable[%d] = %s *", idx, node->pair->uri_string);
            }
        }
    }
}
#endif

int alink_uri_hash_table_init(void)
{
    uint8_t i;
    int res = FAIL_RETURN;

    for (i = 0; i < ALINK_URI_HANDLE_PAIR_NUM; i++) {
        res = _uri_hash_insert(&c_alink_down_uri_handle_map[i], g_uri_hash_table);
        if (res < SUCCESS_RETURN) {
            return res;
        }
    }

#if UTILS_HASH_TABLE_ITERATOR_ENABLE
    alink_debug("print hash table");
    _uri_hash_iterator(g_uri_hash_table);
#endif

    return SUCCESS_RETURN;
}

alink_downstream_handle_func_t alink_get_uri_handle_func(const char *uri_string, uint8_t uri_len)
{
    uint8_t hash;
    uri_hash_node_t *node;

    hash = _uri_to_hash(uri_string, uri_len);
    node = g_uri_hash_table[hash];

    while (node) {
        if (uri_len == strlen(node->pair->uri_string) && !memcmp(uri_string, node->pair->uri_string, uri_len)) {
            return node->pair->handle_func;
        }
        else {
            node = node->next;
        }
    }

    return NULL;
}

void alink_uri_hash_table_deinit(void)
{
    uri_hash_node_t *node, *temp;
    uint8_t idx;

    for (idx = 0; idx < HASH_TABLE_SIZE_MAX; idx++) {
        if (g_uri_hash_table[idx] == NULL) {
            continue;
        }

        node = g_uri_hash_table[idx];
        g_uri_hash_table[idx] = NULL;
        temp = node->next;
        alink_free(node);

        while (temp) {
            node = temp;
            temp = temp->next;
            alink_free(node);
        }
    }
}

/***************************************************************
 * device model management downstream message
 ***************************************************************/
static void _empty_rsp_handle(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    /* ignore the payload, just checkout the return code */
#if (CONFIG_SDK_THREAD_COST == 0)
    {
        linkkit_report_reply_cb_t handle_func;
        /* just invoke the user callback funciton */
        handle_func = (linkkit_report_reply_cb_t)alink_get_event_callback(ITE_REPORT_REPLY);
        if (handle_func != NULL) {
            handle_func(devid, query->id, query->code, (const char *)payload, len);
        }
    }
#else
    {
        alink_msg_event_t *msg_data = alink_malloc(sizeof(alink_msg_event_t));
        if (msg_data == NULL) {
            alink_warning("malloc failed");
            return;
        }

        msg_data->devid = devid;
        msg_data->type = ALINK_EVENT_EMPTY_RSP;
        memcpy(&msg_data->query, query, sizeof(alink_uri_query_t));
        msg_data->msg.empty_rsp.payload = alink_malloc(len);
        if (msg_data->msg.empty_rsp.payload == NULL) {
            alink_free(msg_data);
            return;
        }
        memcpy(msg_data->msg.empty_rsp.payload, payload, len);
        msg_data->msg.empty_rsp.len = len;

        alink_msg_list_insert(msg_data);
    }
#endif
}

static void alink_downstream_thing_property_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    alink_info("property post rsp recv");
    _empty_rsp_handle(devid, pk, dn, payload, len, query);
}

static void alink_downstream_thing_property_put_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;
    lite_cjson_t root, item;
    char *req_data;

    alink_info("property put req recv");

    res = alink_utils_json_parse((const char *)payload, len, cJSON_Object, &root);
    if (res < SUCCESS_RETURN) {
        return;
    }
    res = alink_utils_json_object_item(&root, alink_proto_key_property, sizeof(alink_proto_key_property)-1, cJSON_Object, &item);
    if (res < SUCCESS_RETURN) {
        return;
    }

    req_data = alink_utils_strdup(item.value, item.value_length);
    if (req_data == NULL) {
        return;
    }

    alink_debug("property set req data = %s", req_data);

#if (CONFIG_SDK_THREAD_COST == 0)
    {
        linkkit_property_set_cb_t handle_func;
        /* just invoke the user callback funciton */
        handle_func = (linkkit_property_set_cb_t)alink_get_event_callback(ITE_PROPERTY_SET);
        if (handle_func != NULL) {
            res = handle_func(devid, (const char *)req_data, item.value_length);
        }
    }
    alink_free(req_data);

    /* just return if ack need is no */
    if (query->ack == 'y') {
        query->code = (res == SUCCESS_RETURN) ? ALINK_ERROR_CODE_200: ALINK_ERROR_CODE_400;

        /* send upstream response */
        alink_upstream_thing_property_set_rsp(pk, dn, query);
    }
#else
    {
        alink_msg_event_t *msg_data = alink_malloc(sizeof(alink_msg_event_t));
        if (msg_data == NULL) {
            alink_warning("malloc failed");
            return;
        }

        msg_data->devid = devid;
        msg_data->type = ALINK_EVENT_PROPERTY_PUT_REQ;
        memcpy(&msg_data->query, query, sizeof(alink_uri_query_t));
        msg_data->msg.property_put_req.payload = req_data;
        msg_data->msg.property_put_req.len = item.value_length;

        alink_msg_list_insert(msg_data);
    }
#endif
}

static void alink_downstream_thing_property_get_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;
    lite_cjson_t root, item;
    char *req_data;

    alink_info("propery get req recv");

    res = alink_utils_json_parse((const char *)payload, len, cJSON_Object, &root);
    if (res < SUCCESS_RETURN) {
        return;
    }
    res = alink_utils_json_object_item(&root, alink_proto_key_params, sizeof(alink_proto_key_params)-1, cJSON_Array, &item);
    if (res < SUCCESS_RETURN) {
        return;
    }

    req_data = alink_utils_strdup(item.value, item.value_length);
    if (req_data == NULL) {
        return;
    }

    alink_debug("property get req data = %s", req_data);

#if (CONFIG_SDK_THREAD_COST == 0)
    /* just invoke the user callback funciton */
    {
        char *rsp_data = NULL;
        uint32_t rsp_len;
        linkkit_property_get_cb_t handle_func;
        handle_func = (linkkit_property_get_cb_t)alink_get_event_callback(ITE_PROPERTY_GET);
        if (handle_func != NULL) {
            res = handle_func(devid, req_data, item.value_length, &rsp_data, &rsp_len);
        }

        alink_debug("propery get user rsp = %.*s", rsp_len, rsp_data);

        /* send upstream response */
        alink_upstream_thing_property_get_rsp(pk, dn, (res == SUCCESS_RETURN) ? ALINK_ERROR_CODE_200: ALINK_ERROR_CODE_400, rsp_data, rsp_len, query);
        alink_free(rsp_data);
    }
#else
#endif
    alink_free(req_data);
}

static void alink_downstream_thing_event_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    alink_info("event post rsp recv");
    _empty_rsp_handle(devid, pk, dn, payload, len, query);
}

static void alink_downstream_thing_service_invoke_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;
    lite_cjson_t root, item_id, item_params;
    char *service_id;
    char *service_params;

    alink_info("service req recv");

    res = alink_utils_json_parse((const char *)payload, len, cJSON_Object, &root);
    if (res < SUCCESS_RETURN) {
        return;
    }
    res = alink_utils_json_object_item(&root, alink_proto_key_id, sizeof(alink_proto_key_id)-1, cJSON_String, &item_id);
    if (res < SUCCESS_RETURN) {
        return;
    }
    res = alink_utils_json_object_item(&root, alink_proto_key_params, sizeof(alink_proto_key_params)-1, cJSON_Object, &item_params);
    if (res < SUCCESS_RETURN) {
        return;
    }

    service_id = alink_utils_strdup(item_id.value, item_id.value_length);
    if (service_id == NULL) {
        alink_err("memery not enough");
        return;
    }

    service_params = alink_utils_strdup(item_params.value, item_params.value_length);
    if (service_id == NULL) {
        alink_err("memery not enough");
        alink_free(service_id);
        return;
    }

    alink_debug("service id = %s", service_id);
    alink_debug("service params = %s", service_params);

#if (CONFIG_SDK_THREAD_COST == 0)
    {
        char *rsp_data = NULL;
        uint32_t rsp_len;
        linkkit_service_request_cb_t handle_func;
        /* just invoke the user callback funciton */
        handle_func = (linkkit_service_request_cb_t)alink_get_event_callback(ITE_SERVICE_REQUEST);
        if (handle_func != NULL) {
            res = handle_func(devid, service_id, item_id.value_length, service_params, item_params.value_length, &rsp_data, &rsp_len);
        }

        alink_debug("propery get user rsp = %.*s", rsp_len, rsp_data);
        query->code = (res == SUCCESS_RETURN) ? ALINK_ERROR_CODE_200: ALINK_ERROR_CODE_400;
        /* send upstream response if ack is need */
        if (query->ack == 'y') {
            alink_upstream_thing_service_invoke_rsp(pk, dn, service_id, rsp_data, rsp_len, query);
        }
        alink_free(rsp_data);
        alink_free(service_id);
        alink_free(service_params);
    }
#else
    {
        alink_msg_event_t *msg_data = alink_malloc(sizeof(alink_msg_event_t));
        if (msg_data == NULL) {
            alink_warning("malloc failed");
            return;
        }

        msg_data->devid = devid;
        msg_data->type = ALINK_EVENT_SERVICE_PUT_REQ;
        memcpy(&msg_data->query, query, sizeof(alink_uri_query_t));

        msg_data->msg.service_put_req.payload_len = item_params.value_length;
        msg_data->msg.service_put_req.id_len = item_id.value_length;
        msg_data->msg.service_put_req.id = service_id;
        msg_data->msg.service_put_req.payload = service_params;

        alink_msg_list_insert(msg_data);
    }
#endif
}

/***************************************************************
 * device model management raw data mode downstream message
 ***************************************************************/
static void alink_downstream_thing_raw_put_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    alink_info("raw data recv");

#if (CONFIG_SDK_THREAD_COST == 0)
    {
        /* just invoke the user callback funciton */
        linkkit_rawdata_rx_cb_t handle_func;
        handle_func = (linkkit_rawdata_rx_cb_t)alink_get_event_callback(ITE_RAWDATA_ARRIVED);
        if (handle_func != NULL) {
            handle_func(devid, (query->code == 400)? NULL: payload, (query->code == 400)? 0: len);  /* return NULL if return code is error */
        }
        /* TODO: if ack == 'n' */
    }
#else
    {
        alink_msg_event_t *msg_data = alink_malloc(sizeof(alink_msg_event_t));
        if (msg_data == NULL) {
            alink_warning("malloc failed");
            return;
        }

        msg_data->devid = devid;
        msg_data->type = ALINK_EVENT_RAW_DATA_REQ;
        memcpy(&msg_data->query, query, sizeof(alink_uri_query_t));
        if (query->code == 400) {
            msg_data->msg.raw_data.len = 0;
            msg_data->msg.raw_data.data = NULL;
        }
        else {
            msg_data->msg.raw_data.len = len;
            msg_data->msg.raw_data.data = alink_malloc(len);
            if (msg_data->msg.raw_data.data == NULL) {
                alink_free(msg_data);
                return;
            }
            memcpy(msg_data->msg.raw_data.data, payload, len);
        }

        alink_msg_list_insert(msg_data);
    }
#endif
}

static void alink_downstream_thing_raw_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    if (query->code != 200) {
        query->code = 400;
    }
    alink_downstream_thing_raw_put_req(devid, pk, dn, payload, len, query);
}

#ifdef DEVICE_MODEL_GATEWAY
/***************************************************************
 * subdevice management downstream message
 ***************************************************************/
#if (CONFIG_SDK_THREAD_COST == 0)

#else
static void _alink_downstream_subdev_rsp_notify(uint32_t msgid, uint32_t return_code)
{
    alink_req_cache_node_t *node;

    if (alink_upstream_req_cache_search(msgid, &node) == SUCCESS_RETURN) {
        alink_info("get msg in cache");
        node->result = return_code;
        if (node->semaphore) {
            HAL_SemaphorePost(node->semaphore);
        }
    }
}
#endif

static void alink_downstream_subdev_register_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = 0, idx = 0;
    uint32_t subdev_id = 0;
    lite_cjson_t root, array, object, item_temp;
    char product_key[IOTX_PRODUCT_KEY_LEN] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN] = {0};
    char device_secret[IOTX_DEVICE_SECRET_LEN] = {0};

    alink_info("register rsp recv");

#if (CONFIG_SDK_THREAD_COST == 0)
    if (query->code != 200) {
        /* just delete the req cache, TODO: invoke user cb? miss ds information? */
        alink_upstream_req_cache_delete_by_msgid(query->id);
        return;
    }
#else
    if (query->code != 200) {
        _alink_downstream_subdev_rsp_notify(query->id, query->code);
        return;
    }
#endif

    /* register failed, just return */
    if (query->code != 200) {
        alink_info("cloud return %d", query->code);
        return;
    }

    res = alink_utils_json_parse((const char *)payload, len, cJSON_Object, &root);
    if (res < SUCCESS_RETURN) {
        return;
    }

    res = alink_utils_json_object_item(&root, alink_proto_key_subList, sizeof(alink_proto_key_subList)-1,
                                        cJSON_Array, &array);
    if (res < SUCCESS_RETURN) {
        return;
    }

    for (idx = 0; idx < array.size; idx++) {
        subdev_id = 0;
        memset(product_key, 0, IOTX_PRODUCT_KEY_LEN);
        memset(device_name, 0, IOTX_DEVICE_NAME_LEN);
        memset(device_secret, 0, IOTX_DEVICE_SECRET_LEN);
        memset(&item_temp, 0, sizeof(lite_cjson_t));

        /* get array item */
        res = lite_cjson_array_item(&array, idx, &object);
        if (res < SUCCESS_RETURN || !lite_cjson_is_object(&object)) {
            continue;
        }

        /* get product key */
        res = alink_utils_json_object_item(&object, alink_proto_key_productKey, sizeof(alink_proto_key_productKey)-1, cJSON_String, &item_temp);
        if (res < SUCCESS_RETURN) {
            continue;
        }
        memcpy(product_key, item_temp.value, item_temp.value_length);
        memset(&item_temp, 0, sizeof(lite_cjson_t));

        /* get device name */
        res = alink_utils_json_object_item(&object, alink_proto_key_deviceName, sizeof(alink_proto_key_deviceName)-1, cJSON_String, &item_temp);
        if (res < SUCCESS_RETURN) {
            continue;
        }
        memcpy(device_name, item_temp.value, item_temp.value_length);
        memset(&item_temp, 0, sizeof(lite_cjson_t));

        /* get device secret */
        res = alink_utils_json_object_item(&object, alink_proto_key_deviceSecret, sizeof(alink_proto_key_deviceSecret)-1, cJSON_String, &item_temp);
        if (res < SUCCESS_RETURN) {
            continue;
        }

        /* ignore the iotId */

        memcpy(device_secret, item_temp.value, item_temp.value_length);
        memset(&item_temp, 0, sizeof(lite_cjson_t));

        alink_info("register rsp, idx = %d", idx);
        alink_info("register rsp, pk = %s", product_key);
        alink_info("register rsp, dn = %s", device_name);
        alink_info("register rsp, ds = %s", device_secret);

        /* get subdev id */
        res = alink_subdev_get_devid_by_pkdn(product_key, device_name, &subdev_id);
        if (res != SUCCESS_RETURN) {
            continue;
        }

        alink_info("register rsp, devid = %d", subdev_id);

        /* update sebdev secret */
        res = alink_subdev_update_device_secret(subdev_id, device_secret);
        if (res < SUCCESS_RETURN) {
            continue;
        }

        /* update subdev status */
        alink_subdev_update_status(subdev_id, ALINK_SUBDEV_STATUS_REGISTERED);

        #if (CONFIG_SDK_THREAD_COST == 0)
        {
            /* just invoke the user callback funciton */
            linkkit_connect_success_cb_t handle_func;
            handle_func = (linkkit_connect_success_cb_t)alink_get_event_callback(ITE_CONNECT_SUCC);
            if (handle_func != NULL) {
                handle_func(subdev_id);
            }
        }
        #endif /* #if (CONFIG_SDK_THREAD_COST == 0) */
    }

#if (CONFIG_SDK_THREAD_COST == 1)
    _alink_downstream_subdev_rsp_notify(query->id, query->code);
#endif /* #if (CONFIG_SDK_THREAD_COST == 1) */
}

static void alink_downstream_subdev_unregister_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    alink_info("unregister rsp recv");

    /* ignore the payload, just checkout the return code */
    alink_debug("unregister rsp payload = %.*s", len, payload);

#if (CONFIG_SDK_THREAD_COST == 0)
    if (query->code == 200) {
        alink_req_cache_node_t *req_msg;
        uint32_t *devid_array;
        uint8_t i;
        if (alink_upstream_req_cache_search(query->id, &req_msg) < SUCCESS_RETURN) {
            return;
        }

        devid_array = req_msg->msg_data.subdev_id_list.subdev_id;
        for (i=0; i<MASS_SUBDEV_REQ_NUM_MAX; i++) {
            if (devid_array[i] == 0) {
                break;
            }

            alink_debug("devid %d update status", devid_array[i]);
            alink_subdev_update_status(devid_array[i], ALINK_SUBDEV_STATUS_OPENED);
        }

        alink_upstream_req_cache_delete_by_msgid(query->id);
        /* TODO: user interface isn't provided */
    }
    else {
        /* just delete the req cache, TODO: user interface isn't provided */
        alink_upstream_req_cache_delete_by_msgid(query->id);
    }
#else
    _alink_downstream_subdev_rsp_notify(query->id, query->code);
#endif
}

static void alink_downstream_subdev_login_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    alink_info("login rsp recv");

    /* ignore the payload, just checkout the return code */
    alink_debug("login rsp payload = %.*s", len, payload);

#if (CONFIG_SDK_THREAD_COST == 0)
    if (query->code == 200) {
        alink_req_cache_node_t *req_msg;
        uint32_t *devid_array;
        uint8_t i;
        if (alink_upstream_req_cache_search(query->id, &req_msg) < SUCCESS_RETURN) {
            return;
        }

        devid_array = req_msg->msg_data.subdev_id_list.subdev_id;
        for (i=0; i<MASS_SUBDEV_REQ_NUM_MAX; i++) {
            if (devid_array[i] == 0) {
                break;
            }

            alink_debug("devid %d update status", devid_array[i]);
            alink_subdev_update_status(devid_array[i], ALINK_SUBDEV_STATUS_ONLINE);

            /* invoke device inited evnet callback funciton, this is the only choice - -! */
            {
                linkkit_inited_cb_t handle_func;
                handle_func = (linkkit_inited_cb_t)alink_get_event_callback(ITE_INITIALIZE_COMPLETED);
                if (handle_func != NULL) {
                    handle_func(devid_array[i]);
                }
            }
        }

        alink_upstream_req_cache_delete_by_msgid(query->id);
    }
    else {
        /* just delete the req cache, TODO: user interface isn't provided */
        alink_upstream_req_cache_delete_by_msgid(query->id);
    }
#else
    _alink_downstream_subdev_rsp_notify(query->id, query->code);
#endif
}

static void alink_downstream_subdev_logout_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    alink_info("login rsp recv");

    /* ignore the payload, just checkout the return code */
    alink_debug("login rsp payload = %.*s", len, payload);

#if (CONFIG_SDK_THREAD_COST == 0)
    if (query->code == 200) {
        alink_req_cache_node_t *req_msg;
        uint32_t *devid_array;
        uint8_t i;
        if (alink_upstream_req_cache_search(query->id, &req_msg) < SUCCESS_RETURN) {
            return;
        }

        devid_array = req_msg->msg_data.subdev_id_list.subdev_id;
        for (i=0; i<MASS_SUBDEV_REQ_NUM_MAX; i++) {
            if (devid_array[i] == 0) {
                break;
            }

            alink_debug("devid %d update status", devid_array[i]);
            alink_subdev_update_status(devid_array[i], ALINK_SUBDEV_STATUS_ONLINE);

            /* invoke device disconnected evnet callback funciton, this is the only choice - -! */
            {
                linkkit_disconnected_cb_t handle_func;
                handle_func = (linkkit_disconnected_cb_t)alink_get_event_callback(ITE_DISCONNECTED);
                if (handle_func != NULL) {
                    handle_func(devid_array[i]);
                }
            }
        }

        alink_upstream_req_cache_delete_by_msgid(query->id);
    }
    else {
        /* just delete the req cache, TODO: user interface isn't provided */
        alink_upstream_req_cache_delete_by_msgid(query->id);
    }
#else
    _alink_downstream_subdev_rsp_notify(query->id, query->code);
#endif

}

static void alink_downstream_subdev_topo_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    /* not implement now */
}

static void alink_downstream_subdev_topo_delete_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    /* not implement now */
}

static void alink_downstream_subdev_topo_get_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    /* not implement now */
}

static void alink_downstream_subdev_list_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    /* not implement now */
}

static void alink_downstream_subdev_list_put_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    /* not implement now */
}

static void alink_downstream_subdev_permit_post_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;
    lite_cjson_t root, item_pk, item_timeout;
    char *productKey;
    uint32_t timeoutSec;

    alink_info("permit req recv");

    res = alink_utils_json_parse((const char *)payload, len, cJSON_Object, &root);
    if (res < SUCCESS_RETURN) {
        return;
    }
    res = alink_utils_json_object_item(&root, alink_proto_key_productKey, sizeof(alink_proto_key_productKey)-1, cJSON_String, &item_pk);
    if (res < SUCCESS_RETURN) {
        return;
    }
    res = alink_utils_json_object_item(&root, alink_proto_key_timeoutSec, sizeof(alink_proto_key_timeoutSec)-1, cJSON_Number, &item_timeout);
    if (res < SUCCESS_RETURN) {
        return;
    }

    productKey = alink_utils_strdup(item_pk.value, item_pk.value_length);
    if (productKey == NULL) {
        alink_err("memery not enough");
        return;
    }
    timeoutSec = item_timeout.value_int;

    alink_debug("pk = %s", productKey);
    alink_debug("timeout = %d", timeoutSec);

#if (CONFIG_SDK_THREAD_COST == 0)
    {
        linkkit_permit_join_cb_t handle_func;
        /* just invoke the user callback funciton */
        handle_func = (linkkit_permit_join_cb_t)alink_get_event_callback(ITE_PERMIT_JOIN);
        if (handle_func != NULL) {
            res = handle_func(productKey, timeoutSec);
        }

        /* send upstream response */
        alink_upstream_gw_permit_put_rsp(pk, dn, (res == SUCCESS_RETURN) ? ALINK_ERROR_CODE_200: ALINK_ERROR_CODE_400, query);
    }
    alink_free(productKey);
#else
#endif
}

static void alink_downstream_subdev_config_post_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{

}

#endif

/***************************************************************
 * thing device information management downstream message
 ***************************************************************/

static void alink_downstream_thing_deviceinfo_post_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    _empty_rsp_handle(devid, pk, dn, payload, len, query);
}

static void alink_downstream_thing_deviceinfo_get_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{

}

static void alink_downstream_thing_deviceinfo_delete_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    _empty_rsp_handle(devid, pk, dn, payload, len, query);
}



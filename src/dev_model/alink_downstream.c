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

/****************************************
 * local function prototypes
 ****************************************/
static void alink_downstream_thing_property_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_property_set_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_property_get_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_event_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_service_invoke_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

static void alink_downstream_thing_raw_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_raw_put_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

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

static void alink_downstream_thing_deviceinfo_post_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_deviceinfo_get_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_deviceinfo_delete_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);

/****************************************
 * local variables define
 ****************************************/
/** uri/handle pair map **/
const alink_uri_handle_pair_t c_alink_down_uri_handle_map[] = {
    { "rsp/sys/thing/property/post",                alink_downstream_thing_property_post_rsp        },
    { "req/sys/thing/property/put",                 alink_downstream_thing_property_set_req         },
    { "req/sys/thing/property/get",                 alink_downstream_thing_property_get_req         },
    { "rsp/sys/thing/event/post",                   alink_downstream_thing_event_post_rsp           },
    { "req/sys/thing/service/put",                  alink_downstream_thing_service_invoke_req       },

    { "rsp/sys/thing/raw/post",                     alink_downstream_thing_raw_post_rsp             },
    { "req/sys/thing/raw/put",                      alink_downstream_thing_raw_put_req              },

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

    { "rsp/sys/thing/devinfo/post",                 alink_downstream_thing_deviceinfo_post_rsq      },
    { "rsp/sys/thing/devinfo/get",                  alink_downstream_thing_deviceinfo_get_rsq       },
    { "rsp/sys/thing/devinfo/delete",               alink_downstream_thing_deviceinfo_delete_rsq    },
};

/** alink protocal uri hash table */
static uri_hash_table_t uri_hash_table[HASH_TABLE_SIZE_MAX] = { NULL };

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




static uint8_t _uri_to_hash(const char *uri, uint8_t uri_len)
{
    uint8_t i;
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

int utils_uri_hash_init(const alink_uri_handle_pair_t *uri_handle_pair, uint8_t pair_num, uri_hash_table_t *table)
{
    uint8_t i;
    int res = FAIL_RETURN;

    for (i = 0; i < pair_num; i++) {
        res = _uri_hash_insert(&uri_handle_pair[i], table);
        if (res < SUCCESS_RETURN) {
            return res;
        }
    }

#if UTILS_HASH_TABLE_ITERATOR_ENABLE
    alink_debug("print hash table");
    _uri_hash_iterator(table);
#endif

    return SUCCESS_RETURN;
}

uri_hash_node_t *utils_uri_hash_search(const char *uri_string, uint8_t uri_len, uri_hash_table_t *table)
{
    uint8_t hash;
    uri_hash_node_t *node;

    hash = _uri_to_hash(uri_string, uri_len);
    node = table[hash];

    while (node) {
        if (uri_len == strlen(node->pair->uri_string) && !memcmp(uri_string, node->pair->uri_string, uri_len)) {
            return node;
        }
        else {
            node = node->next;
        }
    }

    return NULL;
}

void utils_uri_hash_destroy(uri_hash_table_t *table)
{
    uri_hash_node_t *node, *temp;
    uint8_t idx;

    ALINK_ASSERT_DEBUG(table != NULL);

    for (idx = 0; idx < HASH_TABLE_SIZE_MAX; idx++) {
        if (table[idx] == NULL) {
            continue;
        }

        node = table[idx];
        table[idx] = NULL;
        temp = node->next;
        alink_free(node);

        while (temp) {
            node = temp;
            temp = temp->next;
            alink_free(node);
        }
    }
}

/**
 * 
 */
int alink_downstream_hash_table_init(void)
{
    return utils_uri_hash_init(c_alink_down_uri_handle_map, ALINK_URI_HANDLE_PAIR_NUM, uri_hash_table);
}

void alink_downstream_hash_table_deinit(void)
{
    utils_uri_hash_destroy(uri_hash_table);
}

alink_downstream_handle_func_t alink_downstream_get_handle_func(const char *uri_string, uint8_t uri_len)
{
    uri_hash_node_t *search_node = utils_uri_hash_search(uri_string, uri_len, uri_hash_table);

    if (search_node == NULL) {
        return NULL;
    }

    return search_node->pair->handle_func;
}

/*  TODO */
int alink_downstream_invoke_mock(const char *uri_string, const uint8_t payload, uint32_t payload_len)
{
    alink_downstream_handle_func_t p_handle_func;
    alink_uri_query_t query = { 0 };

    p_handle_func = alink_downstream_get_handle_func(uri_string, strlen(uri_string));

    if (p_handle_func != NULL) {
        p_handle_func(0, "1", "2", (uint8_t *)"abc", 3, &query);
    } 
    else {
        alink_info("handle func doesn't exit");
    }

    return 1;
}


/***************************************************************
 * device model management downstream message
 ***************************************************************/
/** **/
static void alink_downstream_thing_property_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;
    lite_cjson_t root;

    alink_info("empty rsp recv");
    
    res = alink_utils_json_parse((const char *)payload, len, cJSON_Object, &root);
    if (res < SUCCESS_RETURN) {
        return;
    }

    alink_debug("alink response = %.*s", root.value_length, root.value);

#if (CONFIG_SDK_THREAD_COST == 0)
    {
        linkkit_report_reply_cb_t handle_func;
        /* just invoke the user callback funciton */
        handle_func = (linkkit_report_reply_cb_t)alink_get_event_callback(ITE_REPORT_REPLY);
        if (handle_func != NULL) {
            res = handle_func(devid, query->id, query->code, NULL, 0);
        }
    }
#else
#endif
}

/** **/
static void alink_downstream_thing_property_set_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;
    lite_cjson_t root, item;
    char *req_data;

    alink_info("property set req recv");
    /* TODO: parameter check??? */
    
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

    alink_debug("property get req data = %s", req_data);    

#if (CONFIG_SDK_THREAD_COST == 0)
    {
        linkkit_property_set_cb_t handle_func;
        /* just invoke the user callback funciton */
        handle_func = (linkkit_property_set_cb_t)alink_get_event_callback(ITE_PROPERTY_SET);
        if (handle_func != NULL) {
            res = handle_func(devid, (const char *)req_data, len);
        }
    }
#else
#endif
    alink_free(req_data);

    /* just return if ack need is no */
    if (query->ack == 'y') {
        query->ack = '\0';
        query->code = (res == SUCCESS_RETURN) ? ALINK_ERROR_CODE_200: ALINK_ERROR_CODE_400;

        /* send upstream response */
        alink_upstream_thing_property_set_rsp(pk, dn, query);
    }
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
            res = handle_func(devid, req_data, item.value_length, &rsp_data, (int *)&rsp_len);
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
    alink_downstream_thing_property_post_rsp(devid, pk, dn, payload, len, query);
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
            res = handle_func(devid, service_id, item_id.value_length, service_params, item_params.value_length, &rsp_data, (int *)&rsp_len);
        }

        alink_debug("propery get user rsp = %.*s", rsp_len, rsp_data);
        query->code = (res == SUCCESS_RETURN) ? ALINK_ERROR_CODE_200: ALINK_ERROR_CODE_400;

        /* send upstream response */
        alink_upstream_thing_service_invoke_rsp(pk, dn, service_id, rsp_data, rsp_len, query);
        alink_free(rsp_data);
    }
#else


#endif
    alink_free(service_id);
    alink_free(service_params);
}

/***************************************************************
 * device model management raw data mode downstream message
 ***************************************************************/
static void alink_downstream_thing_raw_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    /* TODO: parameter check??? */
    
#if (CONFIG_SDK_THREAD_COST == 0)
    /* just invoke the user callback funciton */
    linkkit_rawdata_rx_cb_t handle_func;
    alink_info("raw data recv");
    handle_func = (linkkit_rawdata_rx_cb_t)alink_get_event_callback(ITE_RAWDATA_ARRIVED);
    if (handle_func != NULL) {
        handle_func(devid, payload, len);
    }
#else
#endif
}

static void alink_downstream_thing_raw_put_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    /* it's just rawdata, don't care about it's a req or rsp */
    alink_downstream_thing_raw_post_rsp(devid, pk, dn, payload, len, query);
}

#ifdef DEVICE_MODEL_GATEWAY
/***************************************************************
 * subdevice management downstream message
 ***************************************************************/
static void alink_downstream_subdev_register_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = 0, idx = 0;
    uint32_t subdev_id = 0;
    lite_cjson_t root, array, object, item_temp;
    char product_key[IOTX_PRODUCT_KEY_LEN] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN] = {0};
    char device_secret[IOTX_DEVICE_SECRET_LEN] = {0};

    alink_info("register rsp recv");

    if (query->code != 200) {   /* TODO: register error, send to api  */
        alink_info("return code error");
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
    }

/*
    alink_upstream_req_node_t *node;
    alink_info("register rsp recv");

    if (devid != 0) {
        return;
    }


    alink_upstream_req_list_search(query->id, &node);
    if (node == NULL) {
        alink_info("reg rsp corresponding req no exist");
        return;
    }

    alink_info("rsp devid = %d", node->devid);

    alink_upstream_req_list_delete_by_node(node);
*/

#if (CONFIG_SDK_THREAD_COST == 0)

#else

#endif
}

static void alink_downstream_subdev_unregister_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;
    lite_cjson_t root;

    alink_info("unregister rsp recv");
    
    res = alink_utils_json_parse((const char *)payload, len, cJSON_Object, &root);
    if (res < SUCCESS_RETURN) {
        return;
    }

    alink_debug("alink response = %.*s", root.value_length, root.value);

#if (CONFIG_SDK_THREAD_COST == 0)
    {
        /* get unregister req list */
    }
#else
#endif
}

static void alink_downstream_subdev_login_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = 0, idx = 0;
    uint32_t subdev_id = 0;
    lite_cjson_t root, array, object, item_data, item_temp;
    int code = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN] = {0};

    alink_info("login rsp recv");

    if (query->code != 200) {   /* TODO: register error, send to api  */
        alink_info("return code error");
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
        code = 0;
        memset(product_key, 0, IOTX_PRODUCT_KEY_LEN);
        memset(device_name, 0, IOTX_DEVICE_NAME_LEN);
        memset(&item_temp, 0, sizeof(lite_cjson_t));

        /* get array item */
        res = lite_cjson_array_item(&array, idx, &object);
        if (res < SUCCESS_RETURN || !lite_cjson_is_object(&object)) {
            continue;
        }

        /* get code */
        res = alink_utils_json_object_item(&object, alink_proto_key_code, sizeof(alink_proto_key_code)-1, cJSON_Number, &item_temp);
        if (res < SUCCESS_RETURN) {
            continue;
        }
        code = item_temp.value_int;
        if (code != 200) {
            continue;
        }

        /* get data object */
        res = alink_utils_json_object_item(&object, alink_proto_key_data, sizeof(alink_proto_key_data)-1, cJSON_Object, &item_data);
        if (res < SUCCESS_RETURN) {
            continue;
        }

        /* get product key */
        res = alink_utils_json_object_item(&item_data, alink_proto_key_productKey, sizeof(alink_proto_key_productKey)-1, cJSON_String, &item_temp);
        if (res < SUCCESS_RETURN) {
            continue;
        }
        memcpy(product_key, item_temp.value, item_temp.value_length);
        memset(&item_temp, 0, sizeof(lite_cjson_t));

        /* get device name */
        res = alink_utils_json_object_item(&item_data, alink_proto_key_deviceName, sizeof(alink_proto_key_deviceName)-1, cJSON_String, &item_temp);
        if (res < SUCCESS_RETURN) {
            continue;
        }
        memcpy(device_name, item_temp.value, item_temp.value_length);
        memset(&item_temp, 0, sizeof(lite_cjson_t));

        alink_info("register rsp, idx = %d", idx);
        alink_info("register rsp, pk = %s", product_key);
        alink_info("register rsp, dn = %s", device_name);
        
        /* get subdev id */
        res = alink_subdev_get_devid_by_pkdn(product_key, device_name, &subdev_id);
        if (res != SUCCESS_RETURN) {
            continue;
        }

        alink_info("register rsp, devid = %d", subdev_id);

        /* update subdev status */
        alink_subdev_update_status(subdev_id, ALINK_SUBDEV_STATUS_ONLINE);

#if (CONFIG_SDK_THREAD_COST == 0)
        /* invoke user callback function */
        {
            linkkit_inited_cb_t handle_func;
            handle_func = (linkkit_inited_cb_t)alink_get_event_callback(ITE_INITIALIZE_COMPLETED);
            if (handle_func != NULL) {
                handle_func(subdev_id);
            }
        }
#else
        alink_upstream_req_node_t *node;
        if (devid != 0) {
            return;
        }

        alink_upstream_req_list_search(query->id, &node);
        if (node == NULL) {
            alink_info("reg rsp corresponding req no exist");
            return;
        }

        alink_info("rsp devid = %d", node->devid);

        alink_upstream_req_list_delete_by_node(node);
#endif
    }
}

static void alink_downstream_subdev_logout_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    

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
#else
#endif
    alink_free(productKey);
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
    alink_downstream_thing_property_post_rsp(devid, pk, dn, payload, len, query);
}

static void alink_downstream_thing_deviceinfo_get_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    


    
}

static void alink_downstream_thing_deviceinfo_delete_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    alink_downstream_thing_property_post_rsp(devid, pk, dn, payload, len, query);
}



/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"


/*  */
#define ALINK_URI_HANDLE_PAIRE_NUM          (sizeof(c_alink_down_uri_handle_map)/sizeof(alink_uri_handle_pair_t))

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

static void alink_downstream_thing_deviceinfo_post_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_deviceinfo_get_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static void alink_downstream_thing_deviceinfo_delete_rsq(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query);


/****************************************
 * local variables define
 ****************************************/
/** not begin with char '/' **/
const alink_uri_handle_pair_t c_alink_down_uri_handle_map[] = {
    { "rsp/sys/thing/property/post",                alink_downstream_thing_property_post_rsp        },
    { "req/sys/thing/property/put",                 alink_downstream_thing_property_set_req         },
    { "req/sys/thing/property/get",                 alink_downstream_thing_property_get_req         },
    { "rsp/sys/thing/event/post",                   alink_downstream_thing_event_post_rsp           },
    { "req/sys/thing/service/put",                  alink_downstream_thing_service_invoke_req       },

    { "rsp/sys/thing/raw/post",                     alink_downstream_thing_raw_post_rsp             },
    { "req/sys/thing/raw/put",                      alink_downstream_thing_raw_put_req              },

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
    
    { "rsp/sys/thing/devinfo/post",                 alink_downstream_thing_deviceinfo_post_rsq      },
    { "rsp/sys/thing/devinfo/get",                  alink_downstream_thing_deviceinfo_get_rsq       },
    { "rsp/sys/thing/devinfo/delete",               alink_downstream_thing_deviceinfo_delete_rsq    },
};

/*  */
static uri_hash_table_t uri_hash_table[HASH_TABLE_SIZE_MAX] = { NULL };


const char alink_proto_key_params[] = "params";
const char alink_proto_key_data[] = "data";
const char alink_proto_key_code[] = "code";
const char alink_proto_key_serviceId[] = "serviceId";
const char alink_proto_key_productKey[] = "productKey";
const char alink_proto_key_deviceName[] = "deviceName";
const char alink_proto_key_timeoutSec[] = "timeoutSec";
const char alink_proto_key_url[] = "url";


/**
 * 
 */
int alink_downstream_hash_table_init(void)
{
    return utils_uri_hash_init(c_alink_down_uri_handle_map, ALINK_URI_HANDLE_PAIRE_NUM, uri_hash_table);
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
int alink_downstream_invoke_mock(const char *uri_string)
{
    alink_downstream_handle_func_t p_handle_func;

    p_handle_func = alink_downstream_get_handle_func(uri_string, strlen(uri_string));

    alink_uri_query_t query = { 0 };

    if (p_handle_func != NULL) {
        p_handle_func(0, "1", "2", (uint8_t *)"abc", 3, &query);
    } 
    else {
        alink_info("handle func doesn't exit");
    }

    return 1;
}




/**
 * device model management downstream message
 **/

/**
 * 
 */
static void alink_downstream_thing_property_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    alink_info("property/event/devinfo post rsp recv");
    
    int res = FAIL_RETURN;

    lite_cjson_t root, item;

    res = alink_utils_json_parse((const char *)payload, len, cJSON_Object, &root);
    if (res < SUCCESS_RETURN) {
        return;
    }
    res = alink_utils_json_object_item(&root, alink_proto_key_code, sizeof(alink_proto_key_code)-1, cJSON_Number, &item);
    if (res < SUCCESS_RETURN) {
        return;
    }

    alink_debug("alink response code = %d", item.value_int);

#if (CONFIG_SDK_THREAD_COST == 0)
    /* just invoke the user callback funciton */
    linkkit_report_reply_cb_t handle_func;
    handle_func = (linkkit_report_reply_cb_t)alink_get_event_callback(ITE_REPORT_REPLY);
    if (handle_func != NULL) {
        res = handle_func(devid, query->id, item.value_int, NULL, 0);
    }
#else
#endif
}

static void alink_downstream_thing_property_set_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    alink_info("property set req recv");
    /* TODO: parameter check??? */

    int res = FAIL_RETURN;
    
#if (CONFIG_SDK_THREAD_COST == 0)
    /* just invoke the user callback funciton */
    linkkit_property_set_cb_t handle_func;
    handle_func = (linkkit_property_set_cb_t)alink_get_event_callback(ITE_PROPERTY_SET);
    if (handle_func != NULL) {
        res = handle_func(devid, (const char *)payload, len);
    }
#else
#endif

    /* just return if ack need is no */
    if (query->ack == 'y') {
        query->ack = '\0';

        /* send upstream response */
        alink_upstream_thing_property_set_rsp(pk, dn, (res == SUCCESS_RETURN) ? ALINK_ERROR_CODE_200: ALINK_ERROR_CODE_400, query);
    }
}

static void alink_downstream_thing_property_get_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    alink_info("propery get req recv");

    int res = FAIL_RETURN;

    lite_cjson_t root, item;

    res = alink_utils_json_parse((const char *)payload, len, cJSON_Object, &root);
    if (res < SUCCESS_RETURN) {
        return;
    }
    res = alink_utils_json_object_item(&root, alink_proto_key_params, sizeof(alink_proto_key_params)-1, cJSON_Array, &item);
    if (res < SUCCESS_RETURN) {
        return;
    }

    char *req_data = alink_utils_strdup(item.value, item.value_length);
    if (req_data == NULL) {
        alink_err("memery not enough");
        return;
    }

    alink_debug("property get req data = %s", req_data);
    
#if (CONFIG_SDK_THREAD_COST == 0)
    /* just invoke the user callback funciton */
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
    alink_info("service req recv");

    int res = FAIL_RETURN;    

    lite_cjson_t root, item_id, item_params;

    res = alink_utils_json_parse((const char *)payload, len, cJSON_Object, &root);
    if (res < SUCCESS_RETURN) {
        return;
    }
    res = alink_utils_json_object_item(&root, alink_proto_key_serviceId, sizeof(alink_proto_key_serviceId)-1, cJSON_String, &item_id);
    if (res < SUCCESS_RETURN) {
        return;
    }
    res = alink_utils_json_object_item(&root, alink_proto_key_params, sizeof(alink_proto_key_params)-1, cJSON_Object, &item_params);
    if (res < SUCCESS_RETURN) {
        return;
    }

    char *service_id = alink_utils_strdup(item_id.value, item_id.value_length);
    if (service_id == NULL) {
        alink_err("memery not enough");
        return;
    }

    char *service_params = alink_utils_strdup(item_params.value, item_params.value_length);
    if (service_id == NULL) {
        alink_err("memery not enough");
        alink_free(service_id);
        return;
    }

    alink_debug("service id = %s", service_id);
    alink_debug("service params = %s", service_params);

#if (CONFIG_SDK_THREAD_COST == 0)
    /* just invoke the user callback funciton */
    char *rsp_data = NULL;
    uint32_t rsp_len;
    linkkit_service_request_cb_t handle_func;

    handle_func = (linkkit_service_request_cb_t)alink_get_event_callback(ITE_SERVICE_REQUEST);
    if (handle_func != NULL) {
        res = handle_func(devid, service_id, item_id.value_length, service_params, item_params.value_length, &rsp_data, (int *)&rsp_len);
    }

    alink_debug("propery get user rsp = %.*s", rsp_len, rsp_data);

    /* send upstream response */
    alink_upstream_thing_service_invoke_rsp(pk, dn, (res == SUCCESS_RETURN) ? ALINK_ERROR_CODE_200: ALINK_ERROR_CODE_400, rsp_data, rsp_len, query);
    alink_free(rsp_data);
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
    alink_info("raw data recv");
    /* TODO: parameter check??? */
    
#if (CONFIG_SDK_THREAD_COST == 0)
    /* just invoke the user callback funciton */
    linkkit_rawdata_rx_cb_t handle_func;
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

/***************************************************************
 * subdevice management downstream message
 ***************************************************************/

static void alink_downstream_subdev_register_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
      
}

static void alink_downstream_subdev_unregister_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
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



static void alink_downstream_subdev_login_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    
}

static void alink_downstream_subdev_logout_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    
}

static void alink_downstream_subdev_list_post_rsp(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    
}

static void alink_downstream_subdev_list_put_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    
}




static void alink_downstream_subdev_permit_post_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    alink_info("permit req recv");

    int res = FAIL_RETURN;    

    lite_cjson_t root, item_pk, item_timeout;

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

    char *productKey = alink_utils_strdup(item_pk.value, item_pk.value_length);
    if (productKey == NULL) {
        alink_err("memery not enough");
        return;
    }
    uint32_t timeoutSec = item_timeout.value_int;

    alink_debug("pk = %s", productKey);
    alink_debug("timeout = %d", timeoutSec);

#if (CONFIG_SDK_THREAD_COST == 0)
    /* just invoke the user callback funciton */

    linkkit_permit_join_cb_t handle_func;

    handle_func = (linkkit_permit_join_cb_t)alink_get_event_callback(ITE_PERMIT_JOIN);
    if (handle_func != NULL) {
        res = handle_func(productKey, timeoutSec);
    }

    /* send upstream response */
    alink_upstream_gw_permit_put_rsp(pk, dn, (res == SUCCESS_RETURN) ? ALINK_ERROR_CODE_200: ALINK_ERROR_CODE_400, query);
#else
#endif
    alink_free(productKey);
}

static void alink_downstream_subdev_config_post_req(uint32_t devid, const char *pk, const char *dn, const uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    

}

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



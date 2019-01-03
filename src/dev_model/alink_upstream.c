/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"

typedef struct {
    uint8_t                 is_subdev;
    const char             *pk;
    const char             *dn;

    uint8_t                *user_data;
    uint32_t                payload_len;
} up_stream_msg_meta_t;

typedef struct {
    alink_dev_type_t    dev_type;   
    const char          *pk;        /* only valid when dev_type is subdev */
    const char          *dn;        /* only valid when dev_type is subdev */
} alink_dev_info_t;


/*  TODO, add query params!!! */
static int _alink_upstream_send_request_msg(alink_msg_uri_index_t idx, const char *pk, const char *dn, const uint8_t *payload, uint32_t len, 
                                            alink_uri_query_t *query)
{
    int res = FAIL_RETURN;

    char *uri;
    uint32_t msgid = 0;
    char uri_query[45] = {0};

    /* append msgid if parameters query is NULL */
    if (query == NULL) {
        msgid = alink_core_get_msgid();
        HAL_Snprintf(uri_query, sizeof(uri_query), "/?i=%d", msgid);
    }
    else {
        /* setup id if both id and return code are 0 */
        if (0 == query->id && query->code == 0) {
            query->id = alink_core_get_msgid();
        } 

        alink_format_assemble_query(query, uri_query, sizeof(uri_query));
    }

    if (query == NULL || query->format != 'b') {
        lite_cjson_t lite;
        memset(&lite, 0, sizeof(lite_cjson_t));
        res = lite_cjson_parse((char *)payload, len, &lite);
        if (res < SUCCESS_RETURN) {
            alink_err("wrong json format: %.*s", len, payload);
            return FAIL_RETURN;
        }
    }

    if (*pk == '\0' && *dn == '\0') {
        res = alink_format_get_upstream_complete_uri(idx, uri_query, &uri);
    }
    else {
        /* get subdev status first, TODO */

        res = alink_format_get_upstream_subdev_complete_url(idx, pk, dn, uri_query, &uri);
    }

    if (res < 0) {
        return res;
    }

    res = alink_core_send_req_msg(uri, payload, len);
    alink_free(uri);

    alink_debug("core send return = %d", res);

    if (res == SUCCESS_RETURN) {
        res = msgid;
    }

    return res;
}

/***************************************************************
 * device model management upstream message
 ***************************************************************/
const char *c_property_post_fmt = "{\"p\":%.*s}";
int alink_upstream_thing_property_post_req(const char *pk, const char *dn, const char *user_data, uint32_t data_len)
{
    int res = FAIL_RETURN;

    uint32_t len = strlen(c_property_post_fmt) + data_len;
    char *payload = alink_malloc(len);
    if (payload == NULL) {
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }
    
    memset(payload, 0, len);
    HAL_Snprintf(payload, len, c_property_post_fmt, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_PROPERTY_POST, pk, dn, (uint8_t *)payload, strlen(payload), NULL);
    alink_free(payload);

    return res;
}

const char *c_event_post_fmt = "{\"id\":\"%.*s\",\"params\":%.*s}";
/**
 * thing model post event to cloud 
 */
int alink_upstream_thing_event_post_req(const char *pk, const char *dn, const char *event_id, uint8_t id_len, const char *user_data, uint32_t data_len)
{
    int res = FAIL_RETURN;

    /* alink payload format */
    uint32_t len = strlen(c_event_post_fmt) + id_len + data_len;

    char *payload = alink_malloc(len);
    if (payload == NULL) {
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    memset(payload, 0, len);
    HAL_Snprintf(payload, len, c_event_post_fmt, id_len, event_id, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_EVENT_POST, pk, dn, (uint8_t *)payload, strlen(payload), NULL);
    alink_free(payload);

    return res;
}

static int _alink_upstream_empty_rsp(alink_msg_uri_index_t idx, const char *pk, const char *dn, alink_uri_query_t *query)
{
    char payload[3] = "{}";
    
    return _alink_upstream_send_request_msg(idx, pk, dn, (uint8_t *)payload, strlen(payload), query);
}

int alink_upstream_thing_property_set_rsp(const char *pk, const char *dn, alink_uri_query_t *query)      /* TODO */
{
    return _alink_upstream_empty_rsp(ALINK_URI_UP_RSP_PROPERTY_PUT, pk, dn, query);
}

const char *c_property_get_rsp_fmt = "{\"code\":%d,\"params\":%.*s}";
int alink_upstream_thing_property_get_rsp(const char *pk, const char *dn, uint32_t code, const char *user_data, uint32_t data_len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;

    /* alink payload format */
    uint32_t len = strlen(c_property_get_rsp_fmt) + 10 + data_len;

    char *payload = alink_malloc(len);
    if (payload == NULL) {
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    memset(payload, 0, len);
    HAL_Snprintf(payload, len, c_property_get_rsp_fmt, code, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_RSP_PROPERTY_GET, pk, dn, (uint8_t *)payload, strlen(payload), query);
    alink_free(payload);

    return res;
}

/** TODO: same with property get rsp */
const char *c_service_invoke_rsp_fmt = "{\"id\":\"%s\",\"params\":%.*s}";
int alink_upstream_thing_service_invoke_rsp(const char  *pk, const char *dn, const char *service_id, const char *user_data, uint32_t data_len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;

    /* alink payload format */
    uint32_t len = strlen(c_service_invoke_rsp_fmt) + strlen(service_id) + data_len;

    char *payload = alink_malloc(len);
    if (payload == NULL) {
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    memset(payload, 0, len);
    HAL_Snprintf(payload, len, c_service_invoke_rsp_fmt, service_id, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_RSP_SERVICE_PUT, pk, dn, (uint8_t *)payload, strlen(payload), query);
    alink_free(payload);

    return res;
}

/***************************************************************
 * device model management raw data mode upstream message
 ***************************************************************/
int alink_upstream_thing_raw_post_req(const char *pk, const char *dn, const uint8_t *user_data, uint32_t data_len)
{
    return _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_RAW_POST, pk, dn, user_data, data_len, NULL);
}

#if 0 /** this function is useless, as the script doesn't care about which topic the raw data come from **/
int alink_upstream_thing_raw_post_rsp(const char *pk, const char *dn, const uint8_t *user_data, uint32_t data_len)
{
    return 0;
}
#endif

#ifdef DEVICE_MODEL_GATEWAY
/***************************************************************
 * subdevice management upstream message
 ***************************************************************/
char *_alink_upstream_assamble_pkdn_pair_payload(alink_subdev_pkdn_list_t *pkdn_list)
{
    uint8_t idx = 0;
    uint8_t pair_num = pkdn_list->subdev_num;
    pkdn_pair_t *pair_list = pkdn_list->subdev_pkdn;
    lite_cjson_item_t *lite_root = NULL, *lite_object = NULL, *lite_array = NULL, *lite_array_item = NULL;
    char *payload = NULL;

    lite_root = lite_cjson_create_object();
    if (lite_root == NULL) {
        return NULL;
    }

    lite_object = lite_cjson_create_object();
    if (lite_object == NULL) {
        lite_cjson_delete(lite_root);
        return NULL;
    }
    
    lite_array = lite_cjson_create_array();
    if (lite_array == NULL) {
        lite_cjson_delete(lite_root);
        return NULL;
    }

    lite_cjson_add_item_to_object(lite_root, "params", lite_object);
    lite_cjson_add_item_to_object(lite_object, "subList", lite_array);

    for (idx = 0; idx < pair_num; idx++) {
        /* check parameters */
        if (pair_list[idx].pk == NULL || pair_list[idx].dn == NULL) {
            lite_cjson_delete(lite_root);
            return NULL;
        }

        lite_array_item = lite_cjson_create_object();
        if (lite_array_item == NULL) {
            lite_cjson_delete(lite_root);
            return NULL;
        }

        lite_cjson_add_string_to_object(lite_array_item, "productKey", pair_list[idx].pk);
        lite_cjson_add_string_to_object(lite_array_item, "deviceName", pair_list[idx].dn);
        lite_cjson_add_item_to_array(lite_array, lite_array_item);
    }

    payload = lite_cjson_print_unformatted(lite_root);
    lite_cjson_delete(lite_root);

    return payload;
}

const char *c_login_sign_source_fmt = "clientId%sdeviceName%sproductKey%stimestamp%s";   /* clientId is pk.dn */
char *_alink_upstream_assamble_auth_list_payload(alink_subdev_triple_list_t *triple_list)
{
    uint8_t idx = 0;
    uint8_t triple_num = triple_list->subdev_num;
    triple_meta_t *triple_array = triple_list->subdev_triple;
    lite_cjson_item_t *lite_root = NULL, *lite_object = NULL, *lite_array = NULL, *lite_array_item = NULL;
    char *payload = NULL;

    lite_root = lite_cjson_create_object();
    if (lite_root == NULL) {
        return NULL;
    }

    lite_object = lite_cjson_create_object();
    if (lite_object == NULL) {
        lite_cjson_delete(lite_root);
        return NULL;
    }
    
    lite_array = lite_cjson_create_array();
    if (lite_array == NULL) {
        lite_cjson_delete(lite_root);
        return NULL;
    }

    lite_cjson_add_item_to_object(lite_root, "params", lite_object);
    lite_cjson_add_item_to_object(lite_object, "subList", lite_array);

    for (idx = 0; idx < triple_num; idx++) {
        char timestamp[20] = {0};
        char clientid[IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 2];
        char sign[32] = {0};
        char sign_string[65] = {0};
        const char *pk = triple_array[idx].pk;
        const char *dn = triple_array[idx].dn;
        const char *ds = triple_array[idx].ds;
        char *sign_source;
        uint16_t sign_source_len;

        /* check parameters */
        if (pk == NULL || dn == NULL || ds == NULL) {
            lite_cjson_delete(lite_root);
            return NULL;
        }

        lite_array_item = lite_cjson_create_object();
        if (lite_array_item == NULL) {
            lite_cjson_delete(lite_root);
            return NULL;
        }

        HAL_Snprintf(clientid, sizeof(clientid), "%s.%s", pk, dn);
        HAL_Snprintf(timestamp, sizeof(timestamp), "%llu", HAL_UptimeMs());
        sign_source_len = strlen(c_login_sign_source_fmt) + strlen(clientid) + strlen(pk) + strlen(dn) + strlen(timestamp);
        sign_source = alink_malloc(sign_source_len);
        HAL_Snprintf(sign_source, sign_source_len, c_login_sign_source_fmt, clientid, pk, dn, timestamp);
        utils_hmac_sha256((const uint8_t *)sign_source, strlen(sign_source), (const uint8_t *)ds, strlen(ds), (uint8_t *)sign);
        infra_hex2str((uint8_t *)sign, 32, sign_string);
        alink_info("%s", sign_source);
        alink_free(sign_source);

        lite_cjson_add_string_to_object(lite_array_item, "ci", clientid);
        lite_cjson_add_string_to_object(lite_array_item, "dn", dn);
        lite_cjson_add_string_to_object(lite_array_item, "pk", pk);
        lite_cjson_add_string_to_object(lite_array_item, "ts", timestamp);
        lite_cjson_add_string_to_object(lite_array_item, "sm", "hmacSha256");
        lite_cjson_add_string_to_object(lite_array_item, "sn", sign_string);

        lite_cjson_add_item_to_array(lite_array, lite_array_item);
    }

    payload = lite_cjson_print_unformatted(lite_root);
    lite_cjson_delete(lite_root);

    return payload;
}

int alink_upstream_subdev_register_post_req(alink_subdev_pkdn_list_t *pair_list)
{
    int res = FAIL_RETURN;
    char *payload = NULL;

    payload = _alink_upstream_assamble_pkdn_pair_payload(pair_list);
    if (payload == NULL) {
        return res;
    }
    alink_info("subdev reg_post payload = %s", payload);

    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_SUB_REGISTER_POST, "\0", "\0", (uint8_t *)payload, strlen(payload), NULL);
    alink_free(payload);

    return res;
}

int alink_upstream_subdev_register_delete_req(alink_subdev_pkdn_list_t *pair_list)
{
    int res = FAIL_RETURN;
    char *payload = NULL;

    payload = _alink_upstream_assamble_pkdn_pair_payload(pair_list);
    if (payload == NULL) {
        return res;
    }
    alink_info("subdev reg_delete payload = %s", payload);

    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_SUB_REGISTER_DELETE, "\0", "\0", (uint8_t *)payload, strlen(payload), NULL);
    alink_free(payload);

    return res;
}

int alink_upstream_subdev_login_post_req(alink_subdev_triple_list_t *triple_list)
{
    int res = FAIL_RETURN;
    char *payload = NULL;

    payload = _alink_upstream_assamble_auth_list_payload(triple_list);
    if (payload == NULL) {
        return res;
    }
    alink_info("subdev login_post payload = %s", payload);

    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_SUB_LOGIN_POST, "\0", "\0", (uint8_t *)payload, strlen(payload), NULL);
    alink_free(payload);

    return 0;
}

int alink_upstream_subdev_login_delete_req(alink_subdev_pkdn_list_t *pair_list)
{
    int res = FAIL_RETURN;
    char *payload = NULL;

    payload = _alink_upstream_assamble_pkdn_pair_payload(pair_list);
    if (payload == NULL) {
        return res;
    }
    alink_info("subdev login_delete payload = %s", payload);

    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_SUB_LOGIN_DELETE, "\0", "\0", (uint8_t *)payload, strlen(payload), NULL);
    alink_free(payload);

    return res;
}

#if 0 /** all topo relatived funcitons are not implement **/
int alink_upstream_thing_topo_post()
{

}

int alink_upstream_thing_topo_delete()
{

}

int alink_upstream_thing_topo_get()
{

}

int alink_upstream_subdev_list_post_req(void)
{
    return 0;
}

int alink_upstream_subdev_list_put_rsp(void)
{
    return 0;
}
#endif

int alink_upstream_gw_permit_put_rsp(const char *pk, const char *dn, uint32_t code, alink_uri_query_t *query)
{
    return _alink_upstream_empty_rsp(ALINK_URI_UP_RSP_GW_PERMIT_PUT, pk, dn, query);
}

int alink_upstream_gw_config_put_rsp(const char *pk, const char *dn, uint32_t code, alink_uri_query_t *query)
{
    return _alink_upstream_empty_rsp(ALINK_URI_UP_RSP_GW_CONIFG_PUT, pk, dn, query);
}
#endif

/***************************************************************
 * thing device information management upstream message
 ***************************************************************/
const char *c_deviceinfo_alink_fmt = "{\"params\":%.*s}";
int alink_upstream_thing_deviceinfo_post_req(const char *pk, const char *dn, const char *user_data, uint32_t data_len)
{
    int res = FAIL_RETURN;

    uint32_t len = strlen(c_deviceinfo_alink_fmt) + data_len;
    char *payload = alink_malloc(len);
    if (payload == NULL) {
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    memset(payload, 0, len);
    HAL_Snprintf(payload, len, c_deviceinfo_alink_fmt, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_DEVINFO_POST, pk, dn, (uint8_t *)payload, strlen(payload), NULL);
    alink_free(payload);

    return res;
}

int alink_upstream_thing_deviceinfo_get_req(const char *pk, const char *dn)
{
    char payload[] = "{}";
    return _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_DEVINFO_POST, pk, dn, (uint8_t *)payload, strlen(payload), NULL);
}

int alink_upstream_thing_deviceinfo_delete_req(const char *pk, const char *dn, const char *user_data, uint32_t data_len)
{
    int res = FAIL_RETURN;

    uint32_t len = strlen(c_deviceinfo_alink_fmt) + data_len;
    char *payload = alink_malloc(len);
    if (payload == NULL) {
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    memset(payload, 0, len);
    HAL_Snprintf(payload, len, c_deviceinfo_alink_fmt, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_DEVINFO_DELETE, pk, dn, (uint8_t *)payload, strlen(payload), NULL);
    alink_free(payload);

    return res;
}




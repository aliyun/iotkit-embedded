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
    char uri_query[20] = {0};

    /* setup query string if parameters query is NULL */
    if (query == NULL) {
        msgid = alink_core_get_msgid();
        HAL_Snprintf(uri_query, sizeof(uri_query), "/?i=%d", msgid);
    }
    else {
        HAL_Snprintf(uri_query, sizeof(uri_query), "/?i=%d", query->id);

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
int alink_upstream_thing_property_post_req(const char *pk, const char *dn, const char *user_data, uint32_t data_len)
{
    return _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_PROPERTY_POST, pk, dn, (uint8_t *)user_data, data_len, NULL);
}

const char *c_event_post_fmt = "{\"eventId\":\"%.*s\",\"params\":%.*s}";
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

    HAL_Snprintf(payload, len, c_event_post_fmt, id_len, event_id, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_EVENT_POST, pk, dn, (uint8_t *)payload, strlen(payload), NULL);
    alink_free(payload);

    return res;
}

const char *c_simple_rsp_fmt = "{\"code\":%d}";
static int _alink_upstream_thing_status_code_rsp(alink_msg_uri_index_t idx, const char *pk, const char *dn, uint32_t code, alink_uri_query_t *query)
{
    char payload[20] = {0};
    
    HAL_Snprintf(payload, sizeof(payload), c_simple_rsp_fmt, code);
    return _alink_upstream_send_request_msg(idx, pk, dn, (uint8_t *)payload, strlen(payload), query);
}

int alink_upstream_thing_property_set_rsp(const char *pk, const char *dn, uint32_t code, alink_uri_query_t *query)      /* TODO */
{
    return _alink_upstream_thing_status_code_rsp(ALINK_URI_UP_RSP_PROPERTY_PUT, pk, dn, code, query);
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

    HAL_Snprintf(payload, len, c_property_get_rsp_fmt, code, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_RSP_PROPERTY_GET, pk, dn, (uint8_t *)payload, strlen(payload), query);
    alink_free(payload);

    return res;
}

/** TODO: same with property get rsp */
int alink_upstream_thing_service_invoke_rsp(const char  *pk, const char *dn, uint32_t code, const char *user_data, uint32_t data_len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;

    /* alink payload format */
    uint32_t len = strlen(c_property_get_rsp_fmt) + 10 + data_len;

    char *payload = alink_malloc(len);
    if (payload == NULL) {
        return IOTX_CODE_MEMORY_NOT_ENOUGH;
    }

    HAL_Snprintf(payload, len, c_property_get_rsp_fmt, code, data_len, user_data);
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

#if 0 /** this function is useless, as the script doesn't care about the topic of the raw data **/
int alink_upstream_thing_raw_post_rsp(const char *pk, const char *dn, const uint8_t *user_data, uint32_t data_len)
{
    return 0;
}
#endif

/***************************************************************
 * subdevice management upstream message
 ***************************************************************/

int alink_upstream_subdev_register_post_req(void)
{
    return 0;
}

int alink_upstream_subdev_register_delete_req(void)
{
    return 0;
}

int alink_upstream_subdev_login_post_req(void)
{
    return 0;
}

int alink_upstream_subdev_login_delete_req(void)
{
    return 0;
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
#endif


int alink_upstream_subdev_list_post_req(void)
{
    return 0;
}

int alink_upstream_subdev_list_put_rsp(void)
{
    return 0;
}

int alink_upstream_gw_permit_put_rsp(const char *pk, const char *dn, uint32_t code, alink_uri_query_t *query)
{
    return _alink_upstream_thing_status_code_rsp(ALINK_URI_UP_RSP_GW_PERMIT_PUT, pk, dn, code, query);
}

int alink_upstream_gw_config_put_rsp(const char *pk, const char *dn, uint32_t code, alink_uri_query_t *query)
{
    return _alink_upstream_thing_status_code_rsp(ALINK_URI_UP_RSP_GW_CONIFG_PUT, pk, dn, code, query);
}

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

    HAL_Snprintf(payload, len, c_deviceinfo_alink_fmt, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_DEVINFO_DELETE, pk, dn, (uint8_t *)payload, strlen(payload), NULL);
    alink_free(payload);

    return res;
}




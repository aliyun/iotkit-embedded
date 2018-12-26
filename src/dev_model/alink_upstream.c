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



// TODO, add query params!!!
static int _alink_upstream_send_request_msg(alink_msg_uri_index_t idx, const char *pk, const char *dn, const uint8_t *payload, uint32_t len)
{
    int res = FAIL_RETURN;

    char *uri;
    uint32_t msgid = alink_core_get_msgid();
    char uri_query[20] = {0};

    /* setup query string */
    HAL_Snprintf(uri_query, sizeof(uri_query), "/?i=%d", msgid);

    /* parse the payload if it's jsom format, TODO */
    if (1) {
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
        /* get subdev status first */


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

int alink_upstream_send_response_msg(alink_msg_uri_index_t idx, const char *pk, const char *dn, uint8_t *payload, uint32_t len)
{
    int res = FAIL_RETURN;

    return res;
}




/***************************************************************
 * device model management upstream message
 ***************************************************************/
int alink_upstream_thing_property_post_req(const char *pk, const char *dn, const char *user_data, uint32_t data_len)
{
    return _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_PROPERTY_POST, pk, dn, (uint8_t *)user_data, data_len);
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
        return ALINK_CODE_MEMORY_NOT_ENOUGH;
    }

    HAL_Snprintf(payload, len, c_event_post_fmt, id_len, event_id, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_EVENT_POST, pk, dn, (uint8_t *)payload, strlen(payload));
    alink_free(payload);

    return res;
}

const char *c_simple_rsp_fmt = "{\"code\":%d}";
int alink_upstream_thing_property_set_rsp(const char *pk, const char *dn, uint32_t code)
{
    char payload[20] = {0};
    
    HAL_Snprintf(payload, sizeof(payload), c_simple_rsp_fmt, code);
    return _alink_upstream_send_request_msg(ALINK_URI_UP_RSP_PROPERTY_PUT, pk, dn, (uint8_t *)payload, strlen(payload));
}


const char *c_property_get_rsp_fmt = "{\"code\":%d,\"params\":%.*s}";
int alink_upstream_thing_property_get_rsp(const char *pk, const char *dn, uint32_t code, const char *user_data, uint32_t data_len)
{
    int res = FAIL_RETURN;

    /* alink payload format */
    uint32_t len = strlen(c_property_get_rsp_fmt) + 10 + data_len;

    char *payload = alink_malloc(len);
    if (payload == NULL) {
        return ALINK_CODE_MEMORY_NOT_ENOUGH;
    }

    HAL_Snprintf(payload, len, c_property_get_rsp_fmt, code, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_EVENT_POST, pk, dn, (uint8_t *)payload, strlen(payload));
    alink_free(payload);

    return res;
}

int alink_upstream_thing_service_invoke_rsp(int8_t *pk, int8_t *dn, uint8_t *payload, uint32_t len)
{
    int res = FAIL_RETURN;


    return res;
}

/***************************************************************
 * device model management raw data mode upstream message
 ***************************************************************/
int alink_upstream_thing_raw_post_req(const char *pk, const char *dn, const uint8_t *user_data, uint32_t data_len)
{
    return _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_RAW_POST, pk, dn, user_data, data_len);
}

int alink_upstream_thing_raw_post_rsp(const char *pk, const char *dn, const uint8_t *user_data, uint32_t data_len)
{
    return _alink_upstream_send_request_msg(ALINK_URI_UP_RSP_RAW_PUT, pk, dn, user_data, data_len);
}

#if 0
/***************************************************************
 * subdevice management upstream message
 ***************************************************************/

int alink_upstream_subdev_register_post()
{

}

int alink_upstream_subdev_unregister_post()
{

}

int alink_upstream_subdev_topo_post()
{

}

int alink_upstream_subdev_topo_delete()
{

}

int alink_upstream_subdev_topo_get()
{

}

int alink_upstream_subdev_topo_notify_rsp()
{

}

int alink_upstream_subdev_login_post()
{

}

int alink_upstream_subdev_logout_post()
{

}

int alink_upstream_subdev_list_post()
{

}

int alink_upstream_subdev_permit_post_rsp()
{

}

int alink_upstream_subdev_config_post_rsp()
{

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
        return ALINK_CODE_MEMORY_NOT_ENOUGH;
    }

    HAL_Snprintf(payload, len, c_deviceinfo_alink_fmt, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_DEVINFO_POST, pk, dn, (uint8_t *)payload, strlen(payload));
    alink_free(payload);

    return res;
}

int alink_upstream_thing_deviceinfo_get_req(const char *pk, const char *dn)
{
    char payload[] = "{}";
    return _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_DEVINFO_POST, pk, dn, (uint8_t *)payload, strlen(payload));
}


int alink_upstream_thing_deviceinfo_delete_req(const char *pk, const char *dn, const char *user_data, uint32_t data_len)
{
    int res = FAIL_RETURN;

    uint32_t len = strlen(c_deviceinfo_alink_fmt) + data_len;
    char *payload = alink_malloc(len);
    if (payload == NULL) {
        return ALINK_CODE_MEMORY_NOT_ENOUGH;
    }

    HAL_Snprintf(payload, len, c_deviceinfo_alink_fmt, data_len, user_data);
    res = _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_DEVINFO_DELETE, pk, dn, (uint8_t *)payload, strlen(payload));
    alink_free(payload);

    return res;
}


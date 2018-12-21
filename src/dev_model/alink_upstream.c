/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"



typedef struct {
    uint8_t                 is_subdev;
    const char             *pk;
    const char             *dn;
    alink_msg_uri_index_t   uri_idx;
    uint8_t                *payload;
    uint16_t                len;
} up_stream_msg_meta_t;



static int _alink_upstream_send_request_msg(alink_msg_uri_index_t idx, const char *pk, const char *dn, uint8_t *payload, uint16_t len)
{
    int res = FAIL_RETURN;

    uint8_t dev_type = 1;
    char *uri;
    uint32_t msgid = alink_core_get_msgid();
    char uri_query[20] = {0};

    /* setup query string */
    HAL_Snprintf(uri_query, sizeof(uri_query), "/?i=%d", msgid);

    if (dev_type == 0) {
        alink_format_get_upstream_complete_uri(ALINK_URI_UP_REQ_PROPERTY_POST, uri_query, &uri);
    }
    else {
        alink_format_get_upstream_subdev_complete_url(ALINK_URI_UP_REQ_PROPERTY_POST, pk, dn, uri_query, &uri);
    }

    res = alink_core_send_req_msg(uri, payload, len);
    if (res == SUCCESS_RETURN) {
        res = msgid;
    }

    return res;
}

int alink_upstream_send_response_msg(alink_msg_uri_index_t idx, const char *pk, const char *dn, uint8_t *payload, uint16_t len)
{
    int res = FAIL_RETURN;

    return res;
}




/***************************************************************
 * device model management upstream message
 ***************************************************************/
int alink_upstream_thing_property_post_req(uint16_t devid, char *pk, char *dn, uint8_t *payload, uint16_t len)
{
    return _alink_upstream_send_request_msg(ALINK_URI_UP_REQ_PROPERTY_POST, pk, dn, payload, len);
}

int alink_upstream_thing_event_post_req(int8_t *pk, int8_t *dn, uint8_t *payload, uint16_t len)
{
    int res = FAIL_RETURN;


    return res;
}

int alink_upstream_thing_property_set_rsp(uint16_t devid, int8_t *pk, int8_t *dn, uint8_t *payload, uint16_t len)
{
    int res = FAIL_RETURN;


    return res;
}

#if 0
int alink_upstream_thing_property_get_rsp(int8_t *pk, int8_t *dn, uint8_t *payload, uint16_t len)
{

}

int alink_upstream_thing_service_invoke_rsp(int8_t *pk, int8_t *dn, uint8_t *payload, uint16_t len)
{

}

/***************************************************************
 * device model management raw data mode upstream message
 ***************************************************************/
int alink_upstream_thing_raw_post_req()
{

}

int alink_upstream_thing_raw_post_rsp()
{

}

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

/***************************************************************
 * thing device information management upstream message
 ***************************************************************/


int alink_upstream_thing_deviceinfo_post_req()
{

}

int alink_upstream_thing_deviceinfo_get_req()
{

}

int alink_upstream_thing_deviceinfo_delete_req()
{

}

#endif
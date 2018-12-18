/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"


typedef struct {
    int8_t *pk;
    int8_t *dn;
    alink_msg_type_t type;
    alink_msg_layer_t layer;
    int8_t *resource;

} alink_upstream_params_t;


const char path_upstream_property_post[] = "thing/property/notify";




/***************************************************************
 * device model management upstream message
 ***************************************************************/

int alink_upstream_thing_property_post_req(int8_t *pk, int8_t *dn, uint8_t *payload, uint16_t len)
{
    int res;
    alink_msg_uri_metadata_t uri_meta;
    uri_meta.dist.type = ALINK_MSG_DIST_TYPE_CLOUD;
    
    uri_meta.type = ALINK_MSG_TYPE_REQ;
    uri_meta.layer = ALINK_MSG_LAYER_SYS;
    uri_meta.path = path_upstream_property_post;

    uri_meta.dist.dest_str = "c/iot";
    uri_meta.query = "?i=1";

    res = alink_core_send_msg(&uri_meta, payload, len);

    return res;
}

#if 0

int alink_upstream_thing_property_set_rsp(int8_t *pk, int8_t *dn, uint8_t *payload, uint16_t len)
{

}

int alink_upstream_thing_property_get_rsp(int8_t *pk, int8_t *dn, uint8_t *payload, uint16_t len)
{

}

int alink_upstream_thing_event_post_req(int8_t *pk, int8_t *dn, uint8_t *payload, uint16_t len)
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
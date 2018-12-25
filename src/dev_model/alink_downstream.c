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
static int alink_downstream_thing_property_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_thing_property_set_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_thing_property_get_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_thing_event_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_thing_service_invoke_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);

static int alink_downstream_thing_raw_post_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_thing_raw_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);

static int alink_downstream_subdev_register_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_subdev_unregister_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_subdev_topo_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_subdev_topo_delete_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_subdev_topo_get_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_subdev_topo_notify_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_subdev_login_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_subdev_logout_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_subdev_list_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_subdev_permit_post_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_subdev_config_post_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);

static int alink_downstream_thing_deviceinfo_post_rsq(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_thing_deviceinfo_get_rsq(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);
static int alink_downstream_thing_deviceinfo_delete_rsq(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query);


/****************************************
 * local variables define
 ****************************************/
/** not begin with char '/' **/
const alink_uri_handle_pair_t c_alink_down_uri_handle_map[] = {
    { "rsp/sys/dt/property/post",              alink_downstream_thing_property_post_rsp        },
    { "req/sys/thing/property/post",           alink_downstream_thing_property_set_req         },
    { "req/sys/thing/property/get",            alink_downstream_thing_property_get_req         },
    { "rsp/sys/dt/event/post",                 alink_downstream_thing_event_post_rsp           },
    { "req/sys/thing/service/post",            alink_downstream_thing_service_invoke_req       },
    { "rsp/sys/dt/raw/post",                   alink_downstream_thing_raw_post_req             },
    { "req/sys/thing/raw/post",                alink_downstream_thing_raw_post_rsp             },

    { "rsp/sys/subdev/register/post",          alink_downstream_subdev_register_post_rsp       },
    { "rsp/sys/subdev/register/delete",        alink_downstream_subdev_unregister_post_rsp     },
    { "rsp/sys/dt/topo/post",                  alink_downstream_subdev_topo_post_rsp           },
    { "rsp/sys/dt/topo/delete",                alink_downstream_subdev_topo_delete_rsp         },
    { "rsp/sys/dt/topo/get",                   alink_downstream_subdev_topo_get_rsp            },
    { "req/sys/subdev/topo/post",              alink_downstream_subdev_topo_notify_req         },
    { "rsp/sys/subdev/login/post",             alink_downstream_subdev_login_post_rsp          },
    { "rsp/sys/subdev/logout/post",            alink_downstream_subdev_logout_post_rsp         },
    { "rsp/sys/dt/list/post",                  alink_downstream_subdev_list_post_rsp           },
    { "req/sys/subdev/permit/post",            alink_downstream_subdev_permit_post_req         },
    { "req/sys/subdev/config/post",            alink_downstream_subdev_config_post_req         },
    { "rsp/sys/dt/deviceinfo/post",            alink_downstream_thing_deviceinfo_post_rsq      },
    { "rsp/sys/dt/deviceinfo/get",             alink_downstream_thing_deviceinfo_get_rsq       },
    { "rsp/sys/dt/deviceinfo/delete",          alink_downstream_thing_deviceinfo_delete_rsq    },
};

/*  */
static uri_hash_table_t uri_hash_table[HASH_TABLE_SIZE_MAX] = { NULL };


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





// TODO
int alink_downstream_invoke_mock(const char *uri_string)
{
    alink_downstream_handle_func_t p_handle_func;

    p_handle_func = alink_downstream_get_handle_func(uri_string, strlen(uri_string));

    alink_uri_query_t query = { 0 };

    p_handle_func("1", "2", (uint8_t *)"abc", 3, &query);

    return 1;
}

/**
 * device model management downstream message
 **/

/**
 * use devid or devtype structure???
 */
static int alink_downstream_thing_property_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;

    alink_debug("property post rsp received");







    return res;
}

static int alink_downstream_thing_property_set_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;

    alink_debug("property set req received");

    return res;
}

static int alink_downstream_thing_property_get_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_thing_event_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_thing_service_invoke_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

/***************************************************************
 * device model management raw data mode downstream message
 ***************************************************************/
static int alink_downstream_thing_raw_post_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_thing_raw_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

/***************************************************************
 * subdevice management downstream message
 ***************************************************************/

static int alink_downstream_subdev_register_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_subdev_unregister_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_subdev_topo_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_subdev_topo_delete_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_subdev_topo_get_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_subdev_topo_notify_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_subdev_login_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_subdev_logout_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_subdev_list_post_rsp(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_subdev_permit_post_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_subdev_config_post_req(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

/***************************************************************
 * thing device information management downstream message
 ***************************************************************/

static int alink_downstream_thing_deviceinfo_post_rsq(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_thing_deviceinfo_get_rsq(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

static int alink_downstream_thing_deviceinfo_delete_rsq(const char *pk, const char *dn, uint8_t *payload, uint16_t len, alink_uri_query_t *query)
{
    int res = FAIL_RETURN;


    return res;
}

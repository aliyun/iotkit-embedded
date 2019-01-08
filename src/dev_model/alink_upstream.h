/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_UPSTREAM__
#define __ALINK_UPSTREAM__

#include "iotx_alink_internal.h"

typedef struct {
    const char *pk;
    const char *dn;
} pkdn_pair_t;

typedef struct {
    const char *pk;
    const char *dn;
    const char *ds;
} triple_meta_t;

typedef struct {
    uint8_t subdev_num;
    pkdn_pair_t *subdev_pkdn;
} alink_subdev_pkdn_list_t;

typedef struct {
    uint8_t subdev_num;
    uint32_t *subdev_array;
} alink_subdev_id_list_t;

/* thing model upstream */
int alink_upstream_thing_property_post_req(uint32_t devid, const char *user_data, uint32_t data_len);
int alink_upstream_thing_event_post_req(uint32_t devid, const char *event_id, uint8_t id_len, const char *user_data, uint32_t data_len);
int alink_upstream_thing_property_get_rsp(const char *pk, const char *dn, uint32_t code, const char *user_data, uint32_t data_len, alink_uri_query_t *query);
int alink_upstream_thing_property_set_rsp(const char *pk, const char *dn, alink_uri_query_t *query);
int alink_upstream_thing_service_invoke_rsp(const char  *pk, const char *dn, const char *service_id, const char *user_data, uint32_t data_len, alink_uri_query_t *query);

int alink_upstream_thing_raw_post_req(uint32_t devid, const uint8_t *user_data, uint32_t data_len);

/* subdev manager upstream */
int alink_upstream_subdev_register_post_req(alink_subdev_id_list_t *subdev_list);
int alink_upstream_subdev_register_delete_req(alink_subdev_id_list_t *subdev_list);

int alink_upstream_subdev_login_post_req(alink_subdev_id_list_t *subdev_list);
int alink_upstream_subdev_login_delete_req(alink_subdev_id_list_t *subdev_list);


int alink_upstream_gw_permit_put_rsp(const char *pk, const char *dn, uint32_t code, alink_uri_query_t *query);


/* thing devinfo stream */
int alink_upstream_thing_deviceinfo_post_req(uint32_t devid, const char *user_data, uint32_t data_len);
int alink_upstream_thing_deviceinfo_get_req(uint32_t devid);
int alink_upstream_thing_deviceinfo_delete_req(uint32_t devid, const char *user_data, uint32_t data_len);





#define UPSTREAM_REQ_MASS_SUBDEV_NUM_MAX        10

typedef struct upstream_req_subdev_list {
    uint32_t subdev_id[UPSTREAM_REQ_MASS_SUBDEV_NUM_MAX];
} upstream_req_data_subdv_list_t;

typedef struct {
    uint32_t msgid;
    uint32_t devid;
    alink_msg_uri_index_t msg_uri;
    void *data;
    list_head_t list;
} alink_upstream_req_node_t;

typedef struct {
    void *mutex;
    uint32_t list_num;
    list_head_t req_list;
} alink_upstream_req_ctx_t;

int alink_upstream_req_ctx_init(void);
int alink_upstream_req_ctx_deinit(void);
int alink_upstream_req_list_search(uint32_t msgid, alink_upstream_req_node_t **node);
int alink_upstream_req_list_delete_by_node(alink_upstream_req_node_t *node);



#endif /* #ifndef __ALINK_UPSTREAM__ */


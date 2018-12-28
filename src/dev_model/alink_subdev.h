/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_SUBDEV_H__
#define __ALINK_SUBDEV_H__

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"
#include "alink_api.h"




typedef enum {
    ALINK_DEV_STATUS_UNAUTHORIZED,    /* Subdev Created */
    ALINK_DEV_STATUS_AUTHORIZED,      /* Receive Topo Add Notify */
    ALINK_DEV_STATUS_REGISTERED,      /* Receive Subdev Registered */
    ALINK_DEV_STATUS_ATTACHED,        /* Receive Subdev Topo Add Reply */
    ALINK_DEV_STATUS_LOGINED,         /* Receive Subdev Login Reply */
    ALINK_DEV_STATUS_ONLINE,          /* After All Topic Subscribed */
    ALINK_DEV_STATUS_ENABLE,          /* enabled */
    ALINK_DEV_STATUS_DISABLE          /* disabled by cloud */
} alink_subdev_status_t;



// wait for test, todo
//void _subdev_hash_destroy(subdev_hash_table_t *hash_table, uint32_t size);



/** **/
int alink_subdev_mgr_init(void);
int alink_subdev_open(iotx_dev_meta_info_t *dev_info);
int alink_subdev_connect_cloud(uint32_t devid);


int alink_subdev_get_devid_by_pkdn(const char *product_key, const char *device_name, uint32_t *devid);



#endif /* #ifndef __ALINK_SUBDEV_H__ */
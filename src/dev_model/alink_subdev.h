/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_SUBDEV_H__
#define __ALINK_SUBDEV_H__

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"
#include "alink_api.h"

typedef enum {
    ALINK_SUBDEV_STATUS_CLOSED = 0,
    ALINK_SUBDEV_STATUS_OPENED,
    ALINK_SUBDEV_STATUS_REGISTERED,
    ALINK_SUBDEV_STATUS_ONLINE,
    ALINK_SUBDEV_STATUS_OFFLINE,
    ALINK_SUBDEV_STATUS_DISABLED,
} alink_subdev_status_t;




/** **/
int alink_subdev_mgr_init(void);
int alink_subdev_mgr_deinit(void);
int alink_subdev_open(iotx_dev_meta_info_t *dev_info);
int alink_subdev_close(uint32_t devid);
int alink_subdev_connect_cloud(uint32_t devid);

int alink_subdev_register(uint32_t devid);
int alink_subdev_unregister(uint32_t devid);
int alink_subdev_login(uint32_t *devid, uint8_t devid_num);
int alink_subdev_logout(uint32_t *devid, uint8_t devid_num);

int alink_subdev_get_pkdn_by_devid(uint32_t devid, char *product_key, char *device_name);
int alink_subdev_get_triple_by_devid(uint32_t devid, char *product_key, char *device_name, char *device_secret);
int alink_subdev_get_devid_by_pkdn(const char *product_key, const char *device_name, uint32_t *devid);
int alink_subdev_update_status(uint32_t devid, alink_subdev_status_t status);
int alink_subdev_update_device_secret(uint32_t devid, const char *device_secret);

#endif /* #ifndef __ALINK_SUBDEV_H__ */


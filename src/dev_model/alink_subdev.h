/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_SUBDEV_H__
#define __ALINK_SUBDEV_H__

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"
#include "alink_api.h"




/* sudev funtions prototypes */
int alink_subdev_search_devid_by_pkdn(const char *product_key, const char *device_name, uint32_t *devid);



#endif /* #ifndef __ALINK_SUBDEV_H__ */
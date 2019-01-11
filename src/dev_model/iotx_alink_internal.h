/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __IOTX_ALINK_INTERNAL_H__
#define __IOTX_ALINK_INTERNAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alink_wrapper.h"
#include "iotx_cm.h"
#include "alink_config.h"
#include "alink_utils.h"
#include "alink_format.h"
#include "alink_upstream.h"
#include "alink_subdev.h"
#include "alink_downstream.h"
#include "alink_core.h"
#include "alink_api.h"        /* TODO */


#if CONFIG_ALINK_DEBUG
    #define ALINK_ASSERT_DEBUG(expr) \
        do { \
            if (!(expr)) { \
                HAL_Printf("### %s | %s(%d): ASSERT FAILED ###: %s is FALSE\r\n", \
                            __FILE__, __func__, __LINE__, #expr); \
                while (1); \
            } \
        } while(0)
#else
    #define ALINK_ASSERT_DEBUG(expr)
#endif

#define ALINK_DEVICE_SELF_ID                (0)

typedef enum {
    ALINK_ERROR_CODE_200    = 200,
    ALINK_ERROR_CODE_400    = 400,
} alink_protocol_error_code_t;

typedef enum {
    ALINK_DEV_TYPE_MASTER,
    ALINK_DEV_TYPE_SUBDEV,
    ALINK_DEV_TYPE_MAX
} alink_dev_type_t;

linkkit_event_cb_t alink_get_event_callback(iotx_linkkit_event_type_t event_id);


#endif /* #ifndef __IOTX_ALINK_INTERNAL_H__ */



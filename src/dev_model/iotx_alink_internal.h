/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __IOTX_ALINK_INTERNAL_H__
#define __IOTX_ALINK_INTERNAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "infra_config.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_cjson.h"
#include "infra_timer.h"
#ifdef DEVICE_MODEL_GATEWAY
#include "infra_string.h"
#include "infra_sha256.h"
#include "infra_report.h"
#endif
#if defined (DEVICE_MODEL_GATEWAY) || defined (THREAD_COST_INTERVAL)
#include "infra_list.h"
#include "infra_compat.h"
#endif
#include "wrappers_defs.h"

#include "iotx_cm.h"
#include "alink_config.h"
#include "alink_utils.h"
#include "alink_format.h"
#include "alink_upstream.h"
#include "alink_downstream.h"
#include "alink_core.h"
#include "alink_wrapper.h"
#ifdef DEVICE_MODEL_GATEWAY
#include "alink_subdev.h"
#endif

#ifdef INFRA_LOG
#include "infra_log.h"
#define alink_emerg(...)                log_emerg("ALINK", __VA_ARGS__)
#define alink_crit(...)                 log_crit("ALINK", __VA_ARGS__)
#define alink_err(...)                  log_err("ALINK", __VA_ARGS__)
#define alink_warning(...)              log_warning("ALINK", __VA_ARGS__)
#define alink_info(...)                 log_info("ALINK", __VA_ARGS__)
#define alink_debug(...)                log_debug("ALINK", __VA_ARGS__)
#else

#define alink_emerg(...)                do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define alink_crit(...)                 do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define alink_err(...)                  do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define alink_warning(...)              do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define alink_info(...)                 do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define alink_debug(...)                do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#endif /* #ifdef INFRA_LOG */

#ifdef INFRA_MEM_STATS
#include "infra_mem_stats.h"
#define alink_malloc(size)              LITE_malloc(size, MEM_MAGIC, "dm")
#define alink_free(ptr)                 LITE_free(ptr)
#else
#define alink_malloc(size)              HAL_Malloc(size)
#define alink_free(ptr)                 {HAL_Free((void *)ptr);ptr = NULL;}
#endif /* #ifdef INFRA_MEM_STATS */

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

#define ALINK_EVENT_ID_LEN_MAX              (30)

#define ALINK_SUBDEV_MASS_OPERATION_NUM     (10)

typedef enum {
    ALINK_ERROR_CODE_200    = 200,
    ALINK_ERROR_CODE_400    = 400,
} alink_protocol_error_code_t;

typedef enum {
    ALINK_DEV_TYPE_MASTER,
    ALINK_DEV_TYPE_SUBDEV,
    ALINK_DEV_TYPE_MAX
} alink_dev_type_t;


#endif /* #ifndef __IOTX_ALINK_INTERNAL_H__ */



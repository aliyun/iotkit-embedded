/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __IOTX_ALINK_INTERNAL_H__
#define __IOTX_ALINK_INTERNAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alink_format.h"
#include "alink_core.h"
#include "alink_bearer.h"
#include "alink_bearer_mqtt.h"
#include "alink_utils.h"
#include "alink_upstream.h"



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
#endif

#ifdef INFRA_MEM_STATS
#define alink_malloc(size)              LITE_malloc(size, MEM_MAGIC, "mqtt")
#define alink_free(ptr)                 LITE_free(ptr)
#else
#define alink_malloc(size)              HAL_Malloc(size)
#define alink_free(ptr)                 {HAL_Free((void *)ptr);ptr = NULL;}
#endif


#define ALINK_DEBUG     // TODO
#ifdef ALINK_DEBUG
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




#endif /* #ifndef __IOTX_ALINK_INTERNAL_H__ */

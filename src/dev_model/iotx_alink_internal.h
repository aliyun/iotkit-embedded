/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alink_format.h"
#include "alink_core.h"
#include "alink_subdev.h"
#include "alink_upstream.h"
#include "alink_downstream.h"
#include "alink_wrapper.h"
#include "alink_utils.h"
#include "iotx_cm.h"

#include "alink_api.h"        /* TODO */

#ifndef __IOTX_ALINK_INTERNAL_H__
#define __IOTX_ALINK_INTERNAL_H__

#define ALINK_DEVICE_SELF_ID        (0)


/* TOOD: test only */
#ifndef DEVICE_MODEL_GATEWAY
#define DEVICE_MODEL_GATEWAY
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
#endif

#ifdef INFRA_MEM_STATS
#define alink_malloc(size)              LITE_malloc(size, MEM_MAGIC, "dm")
#define alink_free(ptr)                 LITE_free(ptr)
#else
#define alink_malloc(size)              HAL_Malloc(size)
#define alink_free(ptr)                 {HAL_Free((void *)ptr);ptr = NULL;}
#endif


#define UTILS_HASH_TABLE_ITERATOR_ENABLE        (1) 
#define ALINK_DEBUG                             (1)

#if ALINK_DEBUG
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


#ifndef CONFIG_MQTT_TX_MAXLEN
    #define CONFIG_MQTT_TX_MAXLEN           (1024)
#endif

#ifndef CONFIG_MQTT_RX_MAXLEN
    #define CONFIG_MQTT_RX_MAXLEN           (1024)
#endif

#ifndef CONFIG_SDK_THREAD_COST
    #define CONFIG_SDK_THREAD_COST          (1)
#endif


typedef enum {
    INTERNAL_EVNET_CLOUD_CONNECTED = 0

} alink_internal_evnet_type_t;


typedef enum {
    ALINK_ERROR_CODE_200            = 200,
    ALINK_ERROR_CODE_400            = 400,
} alink_protocol_error_code_t;


typedef enum {
    ALINK_DEV_TYPE_MASTER,
    ALINK_DEV_TYPE_SUBDEV,
    ALINK_DEV_TYPE_MAX
} alink_dev_type_t;

linkkit_event_cb_t alink_get_event_callback(iotx_linkkit_event_type_t event_id);


#endif /* #ifndef __IOTX_ALINK_INTERNAL_H__ */



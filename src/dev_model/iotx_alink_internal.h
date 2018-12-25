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
#include "alink_upstream.h"
#include "alink_downstream.h"
#include "alink_wrapper.h"
#include "alink_utils.h"

#include "alink_api.h"        // TODO



/** TODO **/
#define ALINK_URI_MAX_LEN           50
#define HASH_TABLE_SIZE_MAX         29 /* 13, 17, 19 ,23, 29, 31, 37, 41, 43, 47, 53, 59*/


typedef enum {
    ALINK_DEV_TYPE_MASTER,
    ALINK_DEV_TYPE_SUBDEV,
    ALINK_DEV_TYPE_MAX
} alink_dev_type_t;

linkkit_event_cb_t alink_get_event_callback(iotx_linkkit_event_typde_t event_id);


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



#define UTILS_HASH_TABLE_ITERATOR_ENABLE       (1) 
#define ALINK_DEBUG                     (1)
#define TEST_MOCK       // TODO

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

typedef enum {
    ALINK_CODE_PARAMS_INVALID        = -4001,
    ALINK_CODE_STATE_ERROR           = -4002,   
    ALINK_CODE_NETWORK_ERROR         = -4003,
    ALINK_CODE_AUTH_ERROR            = -4004,
    ALINK_CODE_GATEWAY_UNSUPPORTED   = -4005,
    ALINK_CODE_MEMORY_NOT_ENOUGH     = -4006,
} iotx_alink_errorcode_t;


#endif /* #ifndef __IOTX_ALINK_INTERNAL_H__ */

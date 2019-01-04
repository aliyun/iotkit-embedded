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
#include "alink_subdev.h"
#include "alink_upstream.h"
#include "alink_downstream.h"
#include "alink_wrapper.h"
#include "alink_utils.h"

#include "alink_api.h"        /* TODO */
#include "iotx_cm.h"


#define ALINK_DEVICE_SELF_ID        (0)






typedef enum {
    ALINK_DEV_TYPE_MASTER,
    ALINK_DEV_TYPE_SUBDEV,
    ALINK_DEV_TYPE_MAX
} alink_dev_type_t;

linkkit_event_cb_t alink_get_event_callback(iotx_linkkit_event_type_t event_id);


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


#define ALINK_BEARER_USED                       (0)

#define UTILS_HASH_TABLE_ITERATOR_ENABLE        (1) 
#define ALINK_DEBUG                             (1)
#define TEST_MOCK       /* TODO */

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
    #define CONFIG_SDK_THREAD_COST          (0)
#endif


/* all dm internal event */
typedef enum {
    IOTX_DM_EVENT_CLOUD_CONNECTED  = 0,
    IOTX_DM_EVENT_CLOUD_DISCONNECT,
    IOTX_DM_EVENT_CLOUD_RECONNECT,
    IOTX_DM_EVENT_LOCAL_CONNECTED,
    IOTX_DM_EVENT_LOCAL_DISCONNECT,
    IOTX_DM_EVENT_LOCAL_RECONNECT,
    IOTX_DM_EVENT_FOUND_DEVICE,
    IOTX_DM_EVENT_REMOVE_DEVICE,
    IOTX_DM_EVENT_REGISTER_RESULT,
    IOTX_DM_EVENT_UNREGISTER_RESULT,
    IOTX_DM_EVENT_INITIALIZED,
    IOTX_DM_EVENT_SEND_RESULT,
    IOTX_DM_EVENT_ADD_SERVICE_RESULT,
    IOTX_DM_EVENT_REMOVE_SERVICE_RESULT,
    IOTX_DM_EVENT_NEW_DATA_RECEIVED,
    IOTX_DM_EVENT_PROPERTY_SET,
    IOTX_DM_EVENT_PROPERTY_GET,
    IOTX_DM_EVENT_TOPO_ADD_NOTIFY,
    IOTX_DM_EVENT_THING_SERVICE_REQUEST,
    IOTX_DM_EVENT_THING_DISABLE,
    IOTX_DM_EVENT_THING_ENABLE,
    IOTX_DM_EVENT_THING_DELETE,
    IOTX_DM_EVENT_MODEL_DOWN_RAW,
    IOTX_DM_EVENT_GATEWAY_PERMIT,
    IOTX_DM_EVENT_SUBDEV_REGISTER_REPLY,
    IOTX_DM_EVENT_SUBDEV_UNREGISTER_REPLY,
    IOTX_DM_EVENT_TOPO_ADD_REPLY,
    IOTX_DM_EVENT_TOPO_DELETE_REPLY,
    IOTX_DM_EVENT_TOPO_GET_REPLY,
    IOTX_DM_EVENT_TOPO_ADD_NOTIFY_REPLY,
    IOTX_DM_EVENT_EVENT_PROPERTY_POST_REPLY,
    IOTX_DM_EVENT_EVENT_SPECIFIC_POST_REPLY,
    IOTX_DM_EVENT_DEVICEINFO_UPDATE_REPLY,
    IOTX_DM_EVENT_DEVICEINFO_DELETE_REPLY,
    IOTX_DM_EVENT_DSLTEMPLATE_GET_REPLY,        /* DEPRE */
    IOTX_DM_EVENT_COMBINE_LOGIN_REPLY,
    IOTX_DM_EVENT_COMBINE_LOGOUT_REPLY,
    IOTX_DM_EVENT_MODEL_UP_RAW_REPLY,
    IOTX_DM_EVENT_LEGACY_THING_CREATED,         /* DEPRE */
    IOTX_DM_EVENT_COTA_NEW_CONFIG,
    IOTX_DM_EVENT_FOTA_NEW_FIRMWARE,
    IOTX_DM_EVENT_NTP_RESPONSE,
    IOTX_DM_EVENT_RRPC_REQUEST,                 /* DEPRE */
    IOTX_DM_EVENT_MAX
} iotx_dm_event_types_t;


typedef enum {
    INTERNAL_EVNET_CLOUD_CONNECTED = 0

} alink_internal_evnet_type_t;


typedef enum {
    ALINK_ERROR_CODE_200            = 200,
    ALINK_ERROR_CODE_400            = 400,


} alink_protocol_error_code_t;





#endif /* #ifndef __IOTX_ALINK_INTERNAL_H__ */



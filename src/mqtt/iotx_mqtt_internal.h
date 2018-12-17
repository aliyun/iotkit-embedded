/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */



#ifndef __IOTX_MQTT_INTERNAL_H__
#define __IOTX_MQTT_INTERNAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mqtt_wrapper.h"
#include "mqtt_api.h"

#ifdef INFRA_LOG
#include "infra_log.h"
#define mqtt_emerg(...)             log_emerg("MQTT", __VA_ARGS__)
#define mqtt_crit(...)              log_crit("MQTT", __VA_ARGS__)
#define mqtt_err(...)               log_err("MQTT", __VA_ARGS__)
#define mqtt_warning(...)           log_warning("MQTT", __VA_ARGS__)
#define mqtt_info(...)              log_info("MQTT", __VA_ARGS__)
#define mqtt_debug(...)             log_debug("MQTT", __VA_ARGS__)
#else
#define mqtt_emerg(...)             do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define mqtt_crit(...)              do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define mqtt_err(...)               do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define mqtt_warning(...)           do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define mqtt_info(...)              do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#define mqtt_debug(...)             do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#endif

#ifdef INFRA_MEM_STATS
#define mqtt_malloc(size)            LITE_malloc(size, MEM_MAGIC, "mqtt")
#define mqtt_free(ptr)               LITE_free(ptr)
#else
#define mqtt_malloc(size)            HAL_Malloc(size)
#define mqtt_free(ptr)               {HAL_Free((void *)ptr);ptr = NULL;}
#endif

#define MQTT_DYNBUF_SEND_MARGIN                      (64)

#define MQTT_DYNBUF_RECV_MARGIN                      (8)

/* MQTT send publish packet */

#endif  /* __IOTX_MQTT_INTERNAL_H__ */

/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */




#ifndef _IOTX_SHADOW_H_
#define _IOTX_SHADOW_H_
#if defined(__cplusplus)
extern "C" {
#endif

#include <string.h>

#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "infra_list.h"
#include "infra_timer.h"
#include "infra_log.h"
#include "infra_json_parser.h"
#include "infra_mem_stats.h"
#include "shadow_api.h"
#include "mqtt_api.h"
#include "wrappers.h"

#ifdef INFRA_MEM_STATS
    #include "infra_mem_stats.h"
    #define SHADOW_malloc(size)            LITE_malloc(size, MEM_MAGIC, "shadow")
    #define SHADOW_free(ptr)               LITE_free(ptr)
#else
    #define SHADOW_malloc(size)            HAL_Malloc(size)
    #define SHADOW_free(ptr)               {HAL_Free((void *)ptr);ptr = NULL;}
#endif

#endif /* _IOTX_SHADOW_H_ */

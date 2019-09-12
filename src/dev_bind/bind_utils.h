/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __BIND_UTILS_H__
#define __BIND_UTILS_H__



#ifdef INFRA_LOG
    #include "infra_log.h"
    #define bind_debug(...)     log_debug("bind", __VA_ARGS__)
    #define bind_info(...)      log_info("bind", __VA_ARGS__)
    #define bind_warn(...)      log_warning("bind", __VA_ARGS__)
    #define bind_err(...)       log_err("bind", __VA_ARGS__)
#else
    #define bind_debug(...)             do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
    #define bind_info(...)              do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
    #define bind_warn(...)              do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
    #define bind_err(...)               do{HAL_Printf(__VA_ARGS__);HAL_Printf("\r\n");}while(0)
#endif

void *bind_zalloc(uint32_t size);
void bind_free(void *ptr);
uint32_t bind_time_is_expired(uint32_t time);
int bind_time_countdown_ms(uint32_t *time, uint32_t millisecond);
uint32_t bind_time_left(uint32_t time);
#endif

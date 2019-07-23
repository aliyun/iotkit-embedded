/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */
#ifndef _AT_SSL_H_
#define _AT_SSL_H_

#include "infra_types.h"
#include "infra_defs.h"

int AT_SSL_Read(uintptr_t handle, char *buf, int len, int timeout_ms);

int AT_SSL_Write(uintptr_t handle, const char *buf, int len, int timeout_ms);

int32_t AT_SSL_Destroy(uintptr_t handle);

uintptr_t AT_SSL_Establish(const char *host,
                           uint16_t port,
                           const char *ca_crt,
                           uint32_t ca_crt_len);
#endif
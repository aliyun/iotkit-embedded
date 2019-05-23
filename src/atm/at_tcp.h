/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#ifndef _AT_TCP_H_
#define _AT_TCP_H_

#include <stdio.h>
#include <string.h>

#include "infra_types.h"


uintptr_t AT_TCP_Establish(const char *host, uint16_t port);

int AT_TCP_Destroy(uintptr_t fd);

int32_t AT_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms);

int32_t AT_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms);
#endif
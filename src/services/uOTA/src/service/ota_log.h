/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef _OTA_LOG_H_
#define _OTA_LOG_H_

#include <stdarg.h>
#include <stdio.h>

#define OTA_LOG_D(format, ...) printf(format"\r\n",##__VA_ARGS__)
#define OTA_LOG_I(format, ...) printf(format"\r\n",##__VA_ARGS__)
#define OTA_LOG_W(format, ...) printf(format"\r\n",##__VA_ARGS__)
#define OTA_LOG_E(format, ...) printf(format"\r\n",##__VA_ARGS__)
#endif  // _OTA_LOG_H_

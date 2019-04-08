/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __AWSS_CONNECT_AP_H__
#define __AWSS_CONNECT_AP_H__

#include "awss_log.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif
#define AWSS_TOKEN_LEN       (16)

int awss_connect(char ssid[HAL_MAX_SSID_LEN], char passwd[HAL_MAX_PASSWD_LEN], uint8_t bssid[ETH_ALEN],
                uint8_t token[AWSS_TOKEN_LEN]);

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif

#endif

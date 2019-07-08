/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __AWSS_BEACON_ANNOUNCE_H__
#define __AWSS_BEACON_ANNOUNCE_H__

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {

#endif
#if defined(AWSS_SUPPORT_BEACON_ANNOUNCE)
int aws_send_beacon_announce(void);
int aws_auto_found_init();
int aws_auto_found_deinit();

#endif

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
#endif

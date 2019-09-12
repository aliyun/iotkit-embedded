/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __WIFI_PROVISION_INTERNAL_H__
#define __WIFI_PROVISION_INTERNAL_H__

#include "infra_config.h"
#include <string.h>
#include <stdio.h>
#include "aws_lib.h"
#include "zconfig_lib.h"
#include "zconfig_utils.h"
#include "zconfig_protocol.h"
#include "zconfig_ieee80211.h"
#include "awss_event.h"
#include "awss_main.h"
#include "os_misc.h"
#include "infra_compat.h"
#include "awss_smartconfig.h"
#include "infra_sha1.h"
#include "passwd.h"
#include "awss_utils.h"
#include "wifi_provision_api.h"
#include "awss_crypt.h"
#include <stdlib.h>
#include "infra_json_parser.h"
#include "awss_aplist.h"
#include "connect_ap.h"
#include "infra_aes.h"
#include "wrappers.h"
#include "wifi_coap.h"
#include "wifi_msg.h"

#ifdef AWSS_SUPPORT_SMARTCONFIG_WPS
    #include "awss_wps.h"
#endif

#ifdef AWSS_SUPPORT_HT40
    #include "awss_ht40.h"
#endif

#if defined(AWSS_SUPPORT_AHA)
    #include "awss_wifimgr.h"
#endif

#ifndef AWSS_DISABLE_ENROLLEE
    #include "awss_enrollee.h"
#endif

#if defined(AWSS_SUPPORT_AHA)
    #include "awss_aha.h"
#endif

#ifdef AWSS_SUPPORT_DISCOVER
    #include "awss_discover.h"
#endif

#include "infra_state.h"
#define dump_awss_status(...)       iotx_state_event(ITE_STATE_WIFI_PROV, __VA_ARGS__)

#endif

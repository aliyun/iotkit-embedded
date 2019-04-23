/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include "awss_cmp.h"
#include "awss_notify.h"
#include "awss_reset.h"
#include <stdio.h>
#include "iot_import.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif
void *awss_token_mutex = NULL;
extern int awss_stop_report_token();

int awss_report_cloud()
{
    awss_cmp_online_init();
#ifdef DEVICE_MODEL_ENABLED
    awss_check_reset();
#endif
    awss_report_token();
    awss_cmp_local_init();
    awss_dev_bind_notify_stop();
    awss_dev_bind_notify();
#ifdef WIFI_PROVISION_ENABLED
#ifndef AWSS_DISABLE_REGISTRAR
    extern void awss_registrar_init(void);
    awss_registrar_init();
#endif
#endif
    return 0;
}

void awss_bind_deinit()
{
    awss_stop_report_token();
    awss_dev_bind_notify_stop();

    if (NULL != awss_token_mutex) {
        HAL_MutexDestroy(awss_token_mutex);
    }
    awss_token_mutex = NULL;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif

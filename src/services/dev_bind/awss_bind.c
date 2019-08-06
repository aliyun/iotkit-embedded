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
static void *awss_bind_mutex = NULL;
extern int awss_stop_report_token();

static uint8_t awss_bind_inited = 0;

int awss_start_bind()
{
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

int awss_report_cloud()
{
    int ret;
    if (awss_bind_mutex == NULL) {
        awss_bind_mutex = HAL_MutexCreate();
        if (awss_bind_mutex == NULL) {
            return STATE_SYS_DEPEND_MUTEX_CREATE;
        }
    }

    if (awss_bind_inited == 1) {
        return 0;
    }
    awss_bind_inited = 1;
    HAL_MutexLock(awss_bind_mutex);
    awss_cmp_online_init();

#ifdef DEVICE_MODEL_ENABLED
    extern int awss_check_reset();
    extern int awss_report_reset_to_cloud();
    if (awss_check_reset()) {
        ret = awss_report_reset_to_cloud();
        iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_RST_IN_PROGRESS, NULL);
        HAL_MutexUnlock(awss_bind_mutex);
        return ret;
    }
#endif
    ret = awss_start_bind();
    HAL_MutexUnlock(awss_bind_mutex);
    return ret;

}

void awss_bind_deinit()
{
    if (awss_bind_mutex) {
        HAL_MutexLock(awss_bind_mutex);
    }
    awss_stop_report_reset();
    awss_stop_report_token();
    awss_dev_bind_notify_stop();
    awss_cmp_online_deinit();
    awss_cmp_local_deinit();

#ifndef AWSS_DISABLE_REGISTRAR
    extern void awss_registrar_deinit(void);
    awss_registrar_deinit();
#endif

    if (NULL != awss_token_mutex) {
        HAL_MutexDestroy(awss_token_mutex);
    }
    awss_token_mutex = NULL;

    if (awss_bind_mutex) {
        HAL_MutexUnlock(awss_bind_mutex);
        HAL_MutexDestroy(awss_bind_mutex);
    }

    awss_bind_mutex = NULL;
    awss_bind_inited = 0;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif

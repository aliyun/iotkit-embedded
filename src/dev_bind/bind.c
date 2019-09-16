/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */
#include <stdio.h>
#include "mqtt_api.h"
#include "bind_msg.h"
#include "bind_utils.h"
#include "bind_mqtt.h"
#include "bind_coap.h"
#include "wrappers.h"
#include "infra_state.h"
#include "bind_api.h"
#include "bind_token.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

bind_context_t *g_context = NULL;

int IOT_Bind_Start(void *mqtt_handle, void *coap_handle)
{
    int ret = STATE_SUCCESS;
    int rst = 0;
    int len;
    if (g_context != NULL) {
        return 0;
    }
    g_context = bind_zalloc(sizeof(bind_context_t));
    if (g_context == NULL) {
        return STATE_SYS_DEPEND_MALLOC;
    }
    g_context->lock = HAL_MutexCreate();
    if (g_context->lock == NULL) {
        goto failed;
    }
    ret = bind_mqtt_init(g_context, mqtt_handle);
    if (ret < 0) {
        goto failed;
    }
    ret = bind_coap_init(g_context, coap_handle);
    if (ret < 0) {
        goto failed;
    }
    len = sizeof(rst);
    HAL_Kv_Get(BIND_KV_RST, &rst, &len);

    if (rst == 1) {
        bind_set_step(g_context, BIND_ENUM_STEP_UNBIND);
    } else {
        bind_set_step(g_context, BIND_ENUM_STEP_REPORTING_TOKEN);
    }
    /*schedule immediately*/
    g_context->sched_time = HAL_UptimeMs();
    return 0;

failed:
    IOT_Bind_Stop();
    return ret;
}

int IOT_Bind_Stop(void)
{
    if (g_context == NULL) {
        return 0;
    }

    bind_mqtt_deinit(g_context);
    bind_coap_deinit(g_context);
    HAL_MutexDestroy(g_context->lock);
    bind_free(g_context);
    g_context = NULL;
    return 0;
}

int IOT_Bind_Yield(void)
{
    int ret = 0;

    if (g_context == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }
    HAL_MutexLock(g_context->lock);
    switch (g_context->step) {
        case BIND_ENUM_STEP_UNBIND:
            /*schedule report reset*/
            ret = bind_time_is_expired(g_context->sched_time);
            if (ret) {
                bind_time_countdown_ms(&g_context->sched_time, BIND_RESET_TIMEOUT);
                ret = bind_report_unbind(g_context);
            }
            break;
        case BIND_ENUM_STEP_REPORTING_TOKEN:
            if (g_context->report_cnt > BIND_REPORT_MAX) {
                break;
            }

            ret = bind_time_is_expired(g_context->token_exp_time);
            if (ret) {
                bind_time_countdown_ms(&g_context->token_exp_time, BIND_TOKEN_LIFE);
                ret = bind_refresh_token(g_context);
            }

            ret = bind_time_is_expired(g_context->sched_time);
            if (ret) {
                bind_time_countdown_ms(&g_context->sched_time, BIND_TOKEN_TIMEOUT);
                g_context->report_cnt++;
                ret = bind_report_token(g_context);
            }

            /*schedule report token*/
            break;
        case BIND_ENUM_STEP_REPORTED_TOKEN: {
            if (g_context->notify_cnt > BIND_NOTIFY_MAX) {
                break;
            }
            ret = bind_time_is_expired(g_context->token_exp_time);
            if (ret) {
                bind_set_step(g_context, BIND_ENUM_STEP_REPORTING_TOKEN);
                break;
            }

            ret = bind_time_is_expired(g_context->sched_time);
            if (ret) {
                g_context->notify_cnt++;
                bind_time_countdown_ms(&g_context->sched_time, BIND_NOTIFY_DURATION);
                ret = bind_coap_notify(g_context);
                /* notify token with coap*/
            }
        }
        break;
        default:
            break;
    }
    HAL_MutexUnlock(g_context->lock);
    /*do common work*/

    return ret;
}

int IOT_Bind_SetToken(uint8_t *token)
{
    return bind_set_token(g_context, token, BIND_TOKEN_LEN, NULL, NULL, 0);
}

int IOT_Bind_SetToken_Ext(uint8_t *token, uint8_t token_len, char *passwd, uint8_t *bssid, uint8_t bssid_len)
{
    return bind_set_token(g_context, token, token_len, passwd, bssid, bssid_len);
}

int IOT_Bind_Reset(void)
{
    int rst = 1;
    int ret;
    if (g_context == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }
    ret = HAL_Kv_Set(BIND_KV_RST, &rst, sizeof(rst), 0);
    if (ret < 0) {
        iotx_state_event(ITE_STATE_DEV_BIND, STATE_SYS_DEPEND_KV_GET, NULL);
    }
    bind_set_step(g_context, BIND_ENUM_STEP_UNBIND);
    return ret;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif

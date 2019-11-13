/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include "wifi_provision_internal.h"
#include "wifi_coap.h"
#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

#define AWSS_PRESS_TIMEOUT_MS  (60 * 1000)
#define AHA_MONITOR_TIMEOUT_MS  (1 * 60 * 1000)

extern int switch_ap_done;
static uint8_t awss_stopped = 1;
static uint8_t g_user_press = 0;
static uint32_t g_config_press_timeout_ms = 3 * 60 * 1000;
static uint32_t g_channel_scan_timeout_ms = 250;
static uint32_t config_press_start_timestamp = 0;

static void awss_press_timeout(void);

void awss_set_press_timeout_ms(unsigned int timeout_ms)
{
    if (timeout_ms < AWSS_PRESS_TIMEOUT_MS) {
        timeout_ms = AWSS_PRESS_TIMEOUT_MS;
    }
    g_config_press_timeout_ms = timeout_ms;
}

uint32_t awss_get_press_timeout_ms()
{
    return g_config_press_timeout_ms;
}

void awss_update_config_press()
{
    if (g_user_press && time_elapsed_ms_since(config_press_start_timestamp) > g_config_press_timeout_ms) {
        awss_press_timeout();
    }
}

uint32_t awss_get_channel_scan_interval_ms()
{
    return g_channel_scan_timeout_ms;
}

void awss_set_channel_scan_interval_ms(uint32_t timeout_ms)
{
    g_channel_scan_timeout_ms = timeout_ms;
}

#if defined(AWSS_SUPPORT_AHA)
static char awss_aha_connect_to_router()
{
    int iter = 0;
    char dest_ap = 0;
    char ssid[PLATFORM_MAX_SSID_LEN + 1] = {0};
    int count = AHA_MONITOR_TIMEOUT_MS / 200;
    for (iter = 0; iter < count; iter++) {
        /*send dev info here*/
        if (!wifi_got_notify_resp(AWSS_NOTIFY_DEV_RAND_SIGN)) {
            wifi_notify_dev_info(AWSS_NOTIFY_DEV_RAND_SIGN);
        }
        memset(ssid, 0, sizeof(ssid));
        HAL_Wifi_Get_Ap_Info(ssid, NULL, NULL);
        if (HAL_Sys_Net_Is_Ready() &&
            strlen(ssid) > 0 && strcmp(ssid, DEFAULT_SSID)) {  /* not AHA */
            dest_ap = 1;
            break;
        }
        if (awss_stopped) {
            break;
        }

        HAL_SleepMs(200);
    }

    return dest_ap;
}
#endif
int awss_start(void)
{
    if (awss_stopped == 0) {
        dump_awss_status(STATE_WIFI_IN_PROGRESS, "awss still in progress");
        return STATE_WIFI_IN_PROGRESS;
    }

    awss_stopped = 0;
    awss_event_post(IOTX_AWSS_START);

    do {
        __awss_start();
#if defined(AWSS_SUPPORT_AHA)
        do {
            char ssid[PLATFORM_MAX_SSID_LEN + 1] = {0};
            if (awss_stopped) {
                break;
            }

            if (switch_ap_done) {
                break;
            }

            HAL_Wifi_Get_Ap_Info(ssid, NULL, NULL);
            if (strlen(ssid) == 0 ) { /* not AHA */
                break;
            }
            if (strcmp(ssid, DEFAULT_SSID)) { /* not AHA */
                break;
            }

            if (HAL_Sys_Net_Is_Ready()) {
                char dest_ap = 0;
                /* register switchap here */
                char topic[TOPIC_LEN_MAX] = {0};
                wifi_coap_init();
                wifi_build_topic(TOPIC_AWSS_SWITCHAP, topic, TOPIC_LEN_MAX);
                wifi_coap_register(topic, wifimgr_process_switch_ap_request);

                memset(topic, 0, TOPIC_LEN_MAX);
                wifi_build_topic(TOPIC_AWSS_GETDEVICEINFO_UCAST, topic, TOPIC_LEN_MAX);
                wifi_coap_register(topic, wifimgr_process_ucast_get_device_info);
                wifi_coap_register(TOPIC_AWSS_GETDEVICEINFO_MCAST, wifimgr_process_mcast_get_device_info);
                dest_ap = awss_aha_connect_to_router();
                if (switch_ap_done || awss_stopped) {
                    break;
                }

                if (dest_ap == 1) {
                    break;
                }
            }
            awss_event_post(IOTX_AWSS_ENABLE_TIMEOUT);
        } while (1);
#endif
        if (awss_stopped) {
            break;
        }

        if (HAL_Sys_Net_Is_Ready()) {
            break;
        }
        wifi_coap_deinit();
    } while (1);

    if (awss_stopped) {
        dump_awss_status(STATE_WIFI_FORCE_STOPPED, "awss stopped in %s", __func__);
        return STATE_WIFI_FORCE_STOPPED;
    }

    g_user_press = 0;
    awss_press_timeout();
    wifi_coap_init();
    wifi_start_connectap_notify();
    awss_stopped = 1;

    return STATE_SUCCESS;
}

int awss_stop(void)
{
    awss_stopped = 1;
    /*awss_stop_connectap_monitor();*/
    g_user_press = 0;
    awss_press_timeout();

    __awss_stop();

    return 0;
}

static void awss_press_timeout(void)
{
    if (g_user_press) {
        awss_event_post(IOTX_AWSS_ENABLE_TIMEOUT);
    }
    g_user_press = 0;
}

int awss_config_press(void)
{
    config_press_start_timestamp = os_get_time_ms();
    dump_awss_status(STATE_WIFI_ENABLE_AWSS, "awss enabled in %s", __func__);
    g_user_press = 1;
    awss_event_post(IOTX_AWSS_ENABLE);

    return 0;
}

uint8_t awss_get_config_press(void)
{
    return g_user_press;
}

int IOCTL_FUNC(IOTX_IOCTL_SET_AWSS_ENABLE_INTERVAL, void *data)
{
    int res;
    uint32_t timeout;
    if(data == NULL) {
        return FAIL_RETURN;
    }
    timeout = *(uint32_t *) data;
    awss_set_press_timeout_ms(timeout);
    return SUCCESS_RETURN;
}

int IOCTL_FUNC(IOTX_IOCTL_SET_AWSS_CHANNEL_SCAN_INTERVAL, void *data)
{
    int res;
    uint32_t timeout;
    if(data == NULL) {
        return FAIL_RETURN;
    }

    timeout = *(uint32_t *) data;
    awss_set_channel_scan_interval_ms(timeout);
    return SUCCESS_RETURN;
}



#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif

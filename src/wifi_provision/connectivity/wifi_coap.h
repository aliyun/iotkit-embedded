/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __WIFI_COAP_H__
#define __WIFI_COAP_H__
#include <stdint.h>
#include "wifi_msg.h"
enum {
    AWSS_NOTIFY_DEV_BIND_TOKEN,
    /*for phone ap */
    AWSS_NOTIFY_DEV_RAND_SIGN,
    /*for smartconfig, zero config */
    AWSS_NOTIFY_SUCCESS,

    AWSS_NOTIFY_TYPE_MAX,
};

struct wifi_register_couple {
    char *topic;
    void *cb;
};

typedef struct {
    void *coap_handle;
    uint8_t notify_id;
    char notify_resp[AWSS_NOTIFY_TYPE_MAX];
    uint16_t notify_msg_id[AWSS_NOTIFY_TYPE_MAX];
    uint8_t aes_random[RANDOM_MAX_LEN];
    uint16_t connectap_notify_cnt;
    uint32_t notify_start_time;
} p_local_handle_t;

extern p_local_handle_t *p_local_handle;

int wifi_coap_init(void);
int wifi_coap_deinit(void);
int wifi_coap_common_register();
int wifi_coap_register(const char *topic, void *cb);
int wifi_got_notify_resp(int type);
int wifi_notify_dev_info(int type);
int wifi_build_topic(const char *topic_fmt, char *topic, uint32_t tlen);
char *wifi_build_sign_src(char *sign_src, int *sign_src_len);
char *wifi_get_coap_payload(void *request, int *payload_len);
uint8_t *wifi_get_rand();
int wifi_coap_send_resp(void *buf, uint32_t len, void *sa, const char *uri, void *req, void *cb, uint16_t *msgid,
                        char qos);
int wifi_get_encrypt_type();
int wifi_coap_cancel_packet(uint16_t msgid);
void *wifi_build_dev_info(int type, void *dev_info, int info_len);
int wifimgr_process_ucast_get_device_info(void *ctx, void *resource, void *remote, void *request);
int wifimgr_process_mcast_get_device_info(void *ctx, void *resource, void *remote, void *request);
int wifi_start_connectap_notify(void);
int wifi_coap_yield(void);
#endif

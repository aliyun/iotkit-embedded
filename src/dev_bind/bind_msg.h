/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __BIND_MSG_H__
#define __BIND_MSG_H__
#include <memory.h>
#include "bind_utils.h"

#define BIND_NOTIFY_PORT           5683

#define BIND_COAP_MSG_LEN          386
#define BIND_MSG_BUFFER_LEN        300
#define BIND_TOPIC_BUFFER_LEN      128
#define BIND_TOKEN_LEN             16
#define BIND_UINT32_STRLEN         10
#define BIND_REPORT_MAX            10
#define BIND_NOTIFY_MAX            120
#define BIND_IDENTIFY_LEN          64
#define BIND_PARAMS_LEN            160
#define BIND_TOKEN_LIFE            (90*1000)
#define BIND_ID_LEN                16
#define BIND_IP_ADDR_LEN           16
#define BIND_CRYPTO_LEVEL          3
#define BIND_RESET_TIMEOUT         10000
#define BIND_TOKEN_TIMEOUT         10000
#define BIND_NOTIFY_DURATION       500

#define BIND_KV_RST                          "bind_rst"
#define BIND_JSON_ID                         "id"
#define BIND_JSON_IDENTIFY                   "identifier"
#define BIND_JSON_PARAMS                     "params"
#define BIND_JSON_CODE                       "code"

#define BIND_DEVINFO_DATA_FMT                "{\"awssVer\":%s,\"productKey\":\"%s\",\"deviceName\":\"%s\",\"mac\":\"%s\",\"ip\":\"%s\",\"cipherType\":%d,\"token\":\"%s\",\"remainTime\":%d,\"type\":%d}"
#define BIND_DEV_NOTIFY_FMT                  "{\"id\":\"%u\",\"version\":\"1.0\",\"method\":\"device.info.notify\",\"params\":%s}"
#define BIND_COAP_MSG_RESP_FMT               "{\"id\":\"%s\",\"code\":%d,\"data\":"

#define BIND_TOPIC_TOKEN_FMT                 "/sys/%s/%s/thing/awss/enrollee/match"
#define BIND_TOPIC_TOKEN_REPY_FMT            "/sys/%s/%s/thing/awss/enrollee/match_reply"
#define BIND_TOPIC_RESET_FMT                 "/sys/%s/%s/thing/reset"
#define BIND_TOPIC_RESET_REPY_FMT            "/sys/%s/%s/thing/reset_reply"
#define BIND_TOPIC_EVENT_FMT                 "/sys/%s/%s/_thing/event/notify"
#define BIND_TOPIC_EVENT_REPLY_FMT           "/sys/%s/%s/_thing/event/notify_reply"
#define BIND_TOPIC_DEVINFO_FMT               "/sys/%s/%s/device/info/get"
#define BIND_TOPIC_DEVINFO_MCAST             "/sys/device/info/get"
#define BIND_TOPIC_NOTIFY                    "/sys/device/info/notify"

#define BIND_REPORT_TOKEN_FMT                "{\"id\":\"%d\",\"version\":\"1.0\",\"method\":\"thing.awss.enrollee.match\",\"params\":{\"token\":\"%s\"}}"
#define BIND_REPORT_RESET_FMT                "{\"id\":\"%d\",\"version\":\"1.0\",\"method\":\"thing.reset\",\"params\":{}}"
#define BIND_EVENT_REPLY_FMT                 "{\"code\":200,\"id\":\"%d\",\"version\":\"1.0\",\"method\":\"_thing.event.notify\",\
\"message\":\"success\",\"data\":{\"identifier\":\"%s\",\"serviceResult\": {}}}"

#define BIND_AWSS_VER                        "{\"smartconfig\":\"2.0\",\"zconfig\":\"2.0\",\"router\":\"2.0\",\"ap\":\"2.0\",\"softap\":\"2.0\"}"
#define BIND_NOTIFY_HOST                     "255.255.255.255"


typedef enum {
    BIND_ENUM_TOPIC_MATCH,
    BIND_ENUM_TOPIC_MATCH_REPLY,
    BIND_ENUM_TOPIC_RESET,
    BIND_ENUM_TOPIC_RESET_REPLY,
    BIND_ENUM_TOPIC_EVENT,
    BIND_ENUM_TOPIC_EVENT_REPLY,
    BIND_ENUM_TOPIC_DEVINFO_MCAST,
    BIND_ENUM_TOPIC_DEVINFO_UCAST,
    BIND_ENUM_TOPIC_NOTIFY
} bind_topic_type;

typedef enum {
    BIND_ENUM_STEP_UNBIND,
    BIND_ENUM_STEP_REPORTING_TOKEN,
    BIND_ENUM_STEP_REPORTED_TOKEN,

} bind_steps_type;

/**
 * @brief this is a network address structure, including host(ip or host name) and port.
 */
typedef struct {
    char host[16]; /**< host ip(dotted-decimal notation) or host name(string) */
    uint16_t port; /**< udp port or tcp port */
} bind_netaddr_t;

typedef struct {
    void *mqtt_handle;
    void *coap_handle;
    void *lock;

    bind_steps_type step;
    uint8_t token[BIND_TOKEN_LEN];
    uint32_t  token_exp_time;
    uint32_t  sched_time;
    uint16_t  report_cnt;
    uint16_t  notify_cnt;
    uint16_t mqtt_msg_id;
    uint16_t coap_msg_id;
} bind_context_t;

void bind_set_step(void *context, bind_steps_type step);
int bind_get_topic(bind_topic_type type, char *buff, int buff_len);

int bind_req_payload_parser(const char *payload, int len, char **id, int *seqlen, char **data, int *datalen);
char *bind_get_mac_str(char mac_str[HAL_MAC_LEN]);
int bind_build_dev_info(bind_context_t *handle, void *dev_info, int info_len);
int bind_get_broadcast_addr(bind_netaddr_t *bcast_addr);
int bind_buid_req_msg(int id, void *method, char *data, char *packet, int *packet_len) ;
int bind_build_packet(int type, void *id, void *ver, void *method, void *data, int code, void *packet,
                      int *packet_len);
#endif

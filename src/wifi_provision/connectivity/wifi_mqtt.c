/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */
#include <string.h>
#include <stdlib.h>
#include "wifi_provision_internal.h"
#include "mqtt_api.h"
#include "infra_json_parser.h"
#include "wrappers.h"
#include "infra_state.h"
#include "infra_compat.h"
#include "infra_string.h"
#include "wifi_mqtt.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

#define TOPIC_ZC_ENROLLEE_REPLY              "/sys/%s/%s/thing/awss/enrollee/found_reply"
#define TOPIC_ZC_CIPHER_REPLY                "/sys/%s/%s/thing/cipher/get_reply"
#define TOPIC_ZC_CHECKIN                     "/sys/%s/%s/thing/awss/enrollee/checkin"

struct wifi_mqtt_couple_t {
    const char *topic;
    void *cb;
};

void *wifi_mqtt_handle = NULL;

extern void awss_enrollee_checkin(void *pcontext, void *pclient, void *msg);
extern void awss_report_enrollee_reply(void *pcontext, void *pclient, void *msg);
extern void awss_get_cipher_reply(void *pcontext, void *pclient, void *msg);

/*for zero config only */
const struct wifi_mqtt_couple_t wifi_mqtt_couple[] = {
    {TOPIC_ZC_CHECKIN,         awss_enrollee_checkin},
    {TOPIC_ZC_ENROLLEE_REPLY,  awss_report_enrollee_reply},
    {TOPIC_ZC_CIPHER_REPLY,    awss_get_cipher_reply},
};

int wifi_mqtt_init(void *handle)
{
    int i;
    int ret = 0;
    char topic[TOPIC_LEN_MAX] = {0};

    if (handle == NULL) {
        handle = IOT_MQTT_Construct(NULL);
    }

    if (handle == NULL) {
        return STATE_WIFI_MQTT_INIT_FAILED;
    }

    wifi_mqtt_handle = handle;

    for (i = 0; i < sizeof(wifi_mqtt_couple) / sizeof(wifi_mqtt_couple[0]); i ++) {
        memset(topic, 0, sizeof(topic));
        wifi_build_topic(wifi_mqtt_couple[i].topic, topic, TOPIC_LEN_MAX);
        ret = IOT_MQTT_Subscribe(handle, topic, 1, wifi_mqtt_couple[i].cb, handle);
    }

    return ret;
}


int wifi_mqtt_deinit(void)
{

    wifi_mqtt_handle = NULL;
    return 0;
}

int wifi_mqtt_report(char *topic, char *data, int len, int qos)
{
    return IOT_MQTT_Publish_Simple(wifi_mqtt_handle, topic, qos, (uint8_t *)data, len);
}

int wifi_build_packet(int type, void *id, void *ver, void *method, void *data, int code, void *packet, int *packet_len)
{
    int len;
    if (packet_len == NULL || data == NULL || packet == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    len = *packet_len;
    if (len <= 0) {
        return STATE_USER_INPUT_INVALID;
    }

    if (type == WIFI_CMP_PKT_TYPE_REQ) {
        if (ver == NULL || method == NULL) {
            return STATE_USER_INPUT_NULL_POINTER;;
        }

        len = HAL_Snprintf(packet, len - 1, WIFI_REQ_FMT, (char *)id, (char *)ver, (char *)method, (char *)data);
        return 0;
    } else if (type == WIFI_CMP_PKT_TYPE_RSP) {
        len = HAL_Snprintf(packet, len - 1, WIFI_ACK_FMT, (char *)id, code, (char *)data);
        return 0;
    }
    return STATE_WIFI_COAP_PKTBLD_FAILED;
}

int wifi_mqtt_get_payload(void *mesg, char **payload, uint32_t *payload_len)
{
    iotx_mqtt_event_msg_pt msg;
    iotx_mqtt_topic_info_pt ptopic_info;
    if (mesg == NULL || payload == NULL || payload_len == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    msg = (iotx_mqtt_event_msg_pt)mesg;
    ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;


    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            *payload_len = ptopic_info->payload_len;
            *payload = (char *)ptopic_info->payload;
            break;
        default:
            return STATE_WIFI_INVALID_MQTT_EVENT;
    }
    return 0;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif

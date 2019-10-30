/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */
#include <string.h>
#include <stdlib.h>
#include "mqtt_api.h"
#include "bind_msg.h"
#include "bind_utils.h"
#include "infra_json_parser.h"
#include "wrappers.h"
#include "infra_state.h"
#include "infra_compat.h"
#include "infra_string.h"


#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

static int bind_report_event_reply(void *context, int id, const char *identify);
static void bind_mqtt_deal_match_repy(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg);
static void bind_mqtt_deal_reset_repy(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg);
static void bind_mqtt_deal_event(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg);
static int bind_mqtt_report(void *handle, char *topic, char *data, int len);

extern bind_context_t *g_context;
typedef int (* bind_event_callback)(const char *detail);

int bind_mqtt_init(void *handle, void *mqtt_handle)
{
    int ret;
    char buff[BIND_TOPIC_BUFFER_LEN] = {0};
    bind_context_t *bind_handle = (bind_context_t *)handle;
    if (bind_handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    if (mqtt_handle == NULL) {
        mqtt_handle = IOT_MQTT_Construct(NULL);
    }

    bind_handle->mqtt_handle = mqtt_handle;

    bind_get_topic(BIND_ENUM_TOPIC_MATCH_REPLY, buff, BIND_TOPIC_BUFFER_LEN);

    ret = IOT_MQTT_Subscribe(bind_handle->mqtt_handle, buff, 1, bind_mqtt_deal_match_repy, bind_handle);
    if (ret < 0) {
        return ret;
    }
    memset(buff, 0, BIND_TOPIC_BUFFER_LEN);
    bind_get_topic(BIND_ENUM_TOPIC_RESET_REPLY, buff, BIND_TOPIC_BUFFER_LEN);
    ret = IOT_MQTT_Subscribe(bind_handle->mqtt_handle, buff, 1, bind_mqtt_deal_reset_repy, bind_handle);
    if (ret < 0) {
        return ret;
    }

    memset(buff, 0, BIND_TOPIC_BUFFER_LEN);
    bind_get_topic(BIND_ENUM_TOPIC_EVENT, buff, BIND_TOPIC_BUFFER_LEN);
    ret = IOT_MQTT_Subscribe(bind_handle->mqtt_handle, buff, 1, bind_mqtt_deal_event, bind_handle);

    return ret;
}



int bind_mqtt_deinit(void *context)
{

    bind_context_t *bind_conect = (bind_context_t *)context;
    bind_conect->mqtt_handle = NULL;
    return 0;
}

int bind_report_token(void *bind_handle)
{
    int ret = 0;
    char buffer[BIND_MSG_BUFFER_LEN] = {0};
    char topic[BIND_TOPIC_BUFFER_LEN] = {0};
    char rand_str[(BIND_TOKEN_LEN << 1) + 1] = {0};
    int len;
    bind_context_t *bind_context = (bind_context_t *)bind_handle;
    if (bind_context == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }
    LITE_hexbuf_convert(bind_context->token,  rand_str, BIND_TOKEN_LEN, 1);
    bind_get_topic(BIND_ENUM_TOPIC_MATCH, topic, BIND_TOPIC_BUFFER_LEN);
    len = HAL_Snprintf(buffer, BIND_MSG_BUFFER_LEN - 1, BIND_REPORT_TOKEN_FMT, bind_context->mqtt_msg_id++, rand_str);
    ret = bind_mqtt_report(bind_context->mqtt_handle, topic, buffer, len);

    iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_REPORT_TOKEN, rand_str);
    return ret;
}



int bind_report_unbind(void *bind_handle)
{
    int ret = 0;
    char buffer[BIND_MSG_BUFFER_LEN] = {0};
    char topic[BIND_TOPIC_BUFFER_LEN] = {0};
    int len ;
    bind_context_t *bind_conect = (bind_context_t *)bind_handle;
    if (bind_conect == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    bind_get_topic(BIND_ENUM_TOPIC_RESET, topic, BIND_TOPIC_BUFFER_LEN);
    len = HAL_Snprintf(buffer, BIND_MSG_BUFFER_LEN - 1, BIND_REPORT_RESET_FMT, bind_conect->mqtt_msg_id++);
    ret = bind_mqtt_report(bind_conect->mqtt_handle, topic, buffer, len);

    return ret;
}

static int bind_report_event_reply(void *context, int id, const char *identify)
{
    int ret = 0;
    char buffer[BIND_MSG_BUFFER_LEN] = {0};
    char topic[BIND_TOPIC_BUFFER_LEN] = {0};
    int len;
    bind_context_t *bind_conect = (bind_context_t *)context;
    if (identify == NULL || bind_conect == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    bind_get_topic(BIND_ENUM_TOPIC_EVENT_REPLY, topic, BIND_TOPIC_BUFFER_LEN);
    len = HAL_Snprintf(buffer, BIND_MSG_BUFFER_LEN - 1, BIND_EVENT_REPLY_FMT, id, identify);
    ret = bind_mqtt_report(bind_conect->mqtt_handle, topic, buffer, len);
    return ret;
}

static void bind_mqtt_deal_match_repy(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    int ret;
    iotx_mqtt_topic_info_t  *topic_info;
    bind_context_t *context = g_context;
    if (msg == NULL || context == NULL) {
        return;
    }

    topic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED: {
            int len = 0;
            int reply_id;

            char *id = json_get_value_by_name((char *)topic_info->payload, topic_info->payload_len, BIND_JSON_ID, &len, NULL);
            if (id == NULL) {
                iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_MQTT_RSP_INVALID, topic_info->payload);
                return;
            }
            reply_id = atoi(id);
            if (reply_id + 1 < context->mqtt_msg_id) {
                iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_MQTT_MSGID_INVALID, topic_info->payload);
                return;
            }

            iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_REPORT_TOKEN_SUCCESS, NULL);
            HAL_MutexLock(context->lock);
            bind_set_step(context, BIND_ENUM_STEP_REPORTED_TOKEN);
            HAL_MutexUnlock(context->lock);
            break;

            default:
                break;
            }
    }
}
static void bind_mqtt_deal_reset_repy(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t *topic_info;
    bind_context_t *context = g_context;
    if (msg == NULL || context == NULL) {
        return;
    }
    topic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED: {
            int rst = 0;
            iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_REPORT_RESET_SUCCESS, NULL);
            HAL_MutexLock(context->lock);
            HAL_Kv_Set(BIND_KV_RST, &rst, sizeof(rst), 0);
            bind_set_step(context, BIND_ENUM_STEP_REPORTING_TOKEN);
            HAL_MutexUnlock(context->lock);
            break;
        }
        default:
            break;
    }
}

static void bind_mqtt_deal_event(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t  *topic_info;
    int reply_id;
    bind_context_t *context = g_context;
    if (msg == NULL || context == NULL) {
        return;
    }
    topic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED: {
            int len = 0;
            char *id, *params, *identify;
            char identify_buf[BIND_IDENTIFY_LEN] = {0};
            char params_buf[BIND_PARAMS_LEN] = {0};
            bind_event_callback callback;
            id = json_get_value_by_name((char *)topic_info->payload, topic_info->payload_len, BIND_JSON_ID, &len, NULL);
            if (id == NULL || len <= 0 || len > BIND_UINT32_STRLEN) {
                iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_MQTT_RSP_INVALID, topic_info->payload);
                return;
            }
            reply_id = atoi(id);

            params = json_get_value_by_name((char *)topic_info->payload, topic_info->payload_len, BIND_JSON_PARAMS, &len, NULL);
            if (params == NULL) {
                iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_MQTT_RSP_INVALID, topic_info->payload);
                return;
            }
            strncpy(params_buf, params, len < BIND_PARAMS_LEN ? len : BIND_PARAMS_LEN - 1);
            iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_RECV_CLOUD_NOTIFY, (const char *)params_buf);
            identify = json_get_value_by_name(params, len, BIND_JSON_IDENTIFY, &len, NULL);
            if (identify == NULL) {
                iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_MQTT_RSP_INVALID, topic_info->payload);
                return;
            }
            strncpy(identify_buf, identify, len < BIND_IDENTIFY_LEN ? len : BIND_IDENTIFY_LEN - 1);

            callback = iotx_event_callback(ITE_BIND_EVENT);
            if (callback) {
                callback(params_buf);
            }
            bind_report_event_reply(pcontext, reply_id, identify_buf);
            break;
        }
        default:
            break;
    }
}

static int bind_mqtt_report(void *handle, char *topic, char *data, int len)
{
    return IOT_MQTT_Publish_Simple(handle, topic, 1, data, len);
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif

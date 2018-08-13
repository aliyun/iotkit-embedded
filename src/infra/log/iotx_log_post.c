#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mqtt_instance.h"
#include "iotx_system_internal.h"
#include "iotx_mqtt_internal.h"
#include "iotx_log_post.h"


static void iotx_log_post_cloud(const char *module, const int level, const char *fmt, va_list *params);
static int iotx_mc_log_post(void *pclient, char *payload);


static const char *LVL_NAMES[] = {
    "emg", "crt", "err", "wrn", "inf", "dbg",
};

static const char THING_LOG_POST_PARAMS[] = 
"{\"id\":\"%d\",\"versoin\":\"1.0\",\"params\":[{\"timestamp\":%lld,\"logLevel\":\"%s\",\"module\":\"%s\",\"logContent\":\"";

static const char THING_LOG_POST_PARAMS_TAIL[] = 
"...\"}],\"method\":\"thing.log.post\"}";

void LITE_syslog_post(const char *module, const int level, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    iotx_log_post_cloud(module, level, fmt, &ap);
    
    va_end(ap);
}

static void log_post_relay_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    /* print topic name and topic message */
    log_info("logpost", "Topic: '%.*s' (Length: %d)",
                  ptopic_info->topic_len,
                  ptopic_info->ptopic,
                  ptopic_info->topic_len);
    log_info("logpost", "Payload: '%.*s' (Length: %d)",
                  ptopic_info->payload_len,
                  ptopic_info->payload,
                  ptopic_info->payload_len);
}

void iotx_log_post_init(void)
{
    char topic_name[50] = {0};
    int ret = 0;

    void *pHandle = mqtt_get_instance();
    if (pHandle == NULL) {
        return;
    }
    iotx_device_info_pt dev = iotx_device_info_get();

    HAL_Snprintf(topic_name,
                       IOTX_URI_MAX_LEN,
                       "/sys/%s/%s/thing/log/post_reply",
                       dev->product_key,
                       dev->device_name);

    ret = iotx_mc_subscribe(pHandle, topic_name, 0, log_post_relay_message_arrive, NULL);
    if (ret <= 0) {
        log_err("logpost", "log post subscribe error\n\n");
    }
}

static void iotx_log_post_cloud(const char *module, const int level, const char *fmt, va_list *params)
{
    char *logbuf = NULL;
    int msgid = 123;
    int length = 0;
    int ret = 0;

    void *pHandle = mqtt_get_instance();
    if (pHandle == NULL) {
        return;
    }

    logbuf = LITE_malloc(LOG_PUBLIC_MSG_MAXLEN+1);
    if (logbuf == NULL) {
        return;
    }
    memset(logbuf, 0, LOG_PUBLIC_MSG_MAXLEN+1);

    /* generate log post json data */
    ret = HAL_Snprintf(logbuf, LOG_PUBLIC_MSG_MAXLEN, THING_LOG_POST_PARAMS, msgid, HAL_UTC_Get(), LVL_NAMES[level], module);
    if (ret < 0) {
        LITE_free(logbuf);
        return;
    }

    length = strlen(logbuf);
    ret = HAL_Vsnprintf(logbuf+length, LOG_PUBLIC_MSG_MAXLEN-length, fmt, *params);
    if (ret < 0) {
        LITE_free(logbuf);
        return;
    }

    length = strlen(logbuf);
    if (length >= (LOG_PUBLIC_MSG_MAXLEN - sizeof(THING_LOG_POST_PARAMS_TAIL))) {
        strcpy(logbuf + LOG_PUBLIC_MSG_MAXLEN - sizeof(THING_LOG_POST_PARAMS_TAIL)-3, THING_LOG_POST_PARAMS_TAIL);
    }
    else {
        strcat(logbuf, THING_LOG_POST_PARAMS_TAIL+3);
    }    

    ret = iotx_mc_log_post(pHandle, logbuf);
    if (ret < 0) {
        log_info((char *)module, "log post to cloud fail, ret = %d\n", ret);
    } else {
        log_err((char *)module, "log post to cloud success");
    }
    
    LITE_free(logbuf);
}

static int iotx_mc_log_post(void *pclient, char *payload)
{
    int ret;
    char topic_name[IOTX_URI_MAX_LEN + 1] = {0};
    iotx_mqtt_topic_info_t topic_info;
    iotx_device_info_pt dev = iotx_device_info_get();

    if (!payload || !pclient) {
        return FAIL_RETURN;
    }

    /* log post topic name */
    ret = HAL_Snprintf(topic_name,
                       IOTX_URI_MAX_LEN,
                       "/sys/%s/%s/thing/log/post",
                       dev->product_key,
                       dev->device_name);
    if (ret <= 0) {
        log_err("log","log topic generate err");
        return FAIL_RETURN;
    }

    /* generate log post json data */
    log_debug("logpost","log post data: %s", payload);

    topic_info.qos = IOTX_MQTT_QOS0;
    topic_info.payload = (void *)payload;
    topic_info.payload_len = strlen(payload);
    topic_info.retain = 0;
    topic_info.dup = 0;

    /* publish message */
    ret = iotx_mc_publish((iotx_mc_client_t*)pclient, topic_name, &topic_info);
    if (ret < 0) {
        log_err("log","publish failed");
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}


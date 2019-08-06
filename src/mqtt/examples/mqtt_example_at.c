#include <stdio.h>
#include <string.h>

#include "dev_sign_api.h"
#include "mqtt_api.h"
#include "wrappers.h"

#ifdef ATM_ENABLED
    #include "at_api.h"
#endif

char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1RIsMLz2BJ";
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "fSAF0hle6xL0oRWd";
char g_device_name[IOTX_DEVICE_NAME_LEN + 1]       = "example1";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "RDXf67itLqZCwdMCRrw0N5FHbv5D7jrE";

static char g_topic_name[CONFIG_MQTT_TOPIC_MAXLEN];

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            HAL_Printf("Message Arrived: \n");
            HAL_Printf("Topic  : %.*s\n", topic_info->topic_len, topic_info->ptopic);
            HAL_Printf("Payload: %.*s\n", topic_info->payload_len, topic_info->payload);
            HAL_Printf("\n");
            break;
        default:
            break;
    }
}

int example_subscribe(void *handle)
{
    int res = 0;
    const char *fmt = "/%s/%s/user/get";
    char *topic = NULL;
    int topic_len = 0;

    topic_len = strlen(fmt) + strlen(g_product_key) + strlen(g_device_name) + 1;
    if (topic_len > CONFIG_MQTT_TOPIC_MAXLEN) {
        HAL_Printf("topic too long\n");
        return -1;
    }
    topic = g_topic_name;
    memset(topic, 0, CONFIG_MQTT_TOPIC_MAXLEN);
    HAL_Snprintf(topic, topic_len, fmt, g_product_key, g_device_name);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
    if (res < 0) {
        HAL_Printf("subscribe failed\n");
        return -1;
    }

    return 0;
}

int example_publish(void *handle)
{
    int res = 0;
    iotx_mqtt_topic_info_t topic_msg;
    const char *fmt = "/%s/%s/user/get";
    char *topic = NULL;
    int topic_len = 0;
    char *payload = "hello,world";

    topic_len = strlen(fmt) + strlen(g_product_key) + strlen(g_device_name) + 1;
    if (topic_len > CONFIG_MQTT_TOPIC_MAXLEN) {
        HAL_Printf("topic too long\n");
        return -1;
    }
    topic = g_topic_name;
    memset(topic, 0, CONFIG_MQTT_TOPIC_MAXLEN);
    HAL_Snprintf(topic, topic_len, fmt, g_product_key, g_device_name);


    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)payload;
    topic_msg.payload_len = strlen(payload);

    res = IOT_MQTT_Publish(handle, topic, &topic_msg);
    if (res < 0) {
        HAL_Printf("publish failed\n");
        return -1;
    }

    return 0;
}

void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    HAL_Printf("msg->event_type : %d\n", msg->event_type);
}

/*
 *  NOTE: About demo topic of /${productKey}/${deviceName}/user/get
 *
 *  The demo device has been configured in IoT console (https://iot.console.aliyun.com)
 *  so that its /${productKey}/${deviceName}/user/get can both be subscribed and published
 *
 *  We design this to completely demostrate publish & subscribe process, in this way
 *  MQTT client can receive original packet sent by itself
 *
 *  For new devices created by yourself, pub/sub privilege also required to be granted
 *  to its /${productKey}/${deviceName}/user/get to run whole example
 */

int main(int argc, char *argv[])
{
    void       *pclient = NULL;
    int         res = 0;
    int         loop_cnt = 0;

    iotx_mqtt_region_types_t    region = IOTX_CLOUD_REGION_SHANGHAI;
    iotx_sign_mqtt_t            sign_mqtt;
    iotx_dev_meta_info_t        meta;
    iotx_mqtt_param_t           mqtt_params;

#ifdef ATM_ENABLED
    if (IOT_ATM_Init() < 0) {
        HAL_Printf("IOT ATM init failed!\n");
        return -1;
    }
#endif
    HAL_Printf("mqtt example\n");

    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, g_product_key); 
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_NAME, g_device_name);

    memset(&meta, 0, sizeof(iotx_dev_meta_info_t));
    memcpy(meta.product_key, g_product_key, strlen(g_product_key));
    memcpy(meta.device_name, g_device_name, strlen(g_device_name));
    memcpy(meta.device_secret, g_device_secret, strlen(g_device_secret));

    memset(&sign_mqtt, 0x0, sizeof(iotx_sign_mqtt_t));

    if (IOT_Sign_MQTT(region, &meta, &sign_mqtt) < 0) {
        return -1;
    }

#if 0   /* Uncomment this to show more information */
    HAL_Printf("sign_mqtt.hostname: %s\n", sign_mqtt.hostname);
    HAL_Printf("sign_mqtt.port    : %d\n", sign_mqtt.port);
    HAL_Printf("sign_mqtt.username: %s\n", sign_mqtt.username);
    HAL_Printf("sign_mqtt.password: %s\n", sign_mqtt.password);
    HAL_Printf("sign_mqtt.clientid: %s\n", sign_mqtt.clientid);
#endif

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = sign_mqtt.port;
    mqtt_params.host = sign_mqtt.hostname;
    mqtt_params.client_id = sign_mqtt.clientid;
    mqtt_params.username = sign_mqtt.username;
    mqtt_params.password = sign_mqtt.password;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.read_buf_size = 1024;
    mqtt_params.write_buf_size = 1024;

    mqtt_params.handle_event.h_fp = example_event_handle;
    mqtt_params.handle_event.pcontext = NULL;

    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        return -1;
    }

    res = example_subscribe(pclient);
    if (res < 0) {
        IOT_MQTT_Destroy(&pclient);
        return -1;
    }

    while (1) {
        if (0 == loop_cnt % 20) {
            example_publish(pclient);
        }

        IOT_MQTT_Yield(pclient, 200);

        loop_cnt += 1;
    }

    return 0;
}


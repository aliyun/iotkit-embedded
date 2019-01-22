#include <stdio.h>
#include <string.h>
#include "dev_sign_api.h"
#include "mqtt_api.h"

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN]);
uint64_t HAL_UptimeMs(void);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);

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
            EXAMPLE_TRACE("Message Arrived:");
            EXAMPLE_TRACE("Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
            EXAMPLE_TRACE("Payload: %.*s", topic_info->payload_len, topic_info->payload);
            EXAMPLE_TRACE("\n");
            break;
        default:
            break;
    }
}

int example_subscribe(void *handle)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN] = {0};
    const char *fmt = "/%s/%s/get";
    char *topic = NULL;
    int topic_len = 0;

    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);

    topic_len = strlen(fmt) + strlen(product_key) + strlen(device_name) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, product_key, device_name);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
    if (res < 0) {
        EXAMPLE_TRACE("subscribe failed");
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}

int example_publish(void *handle)
{
    int res = 0;
    iotx_mqtt_topic_info_t topic_msg;
    char product_key[IOTX_PRODUCT_KEY_LEN] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN] = {0};
    const char *fmt = "/%s/%s/get";
    char *topic = NULL;
    int topic_len = 0;
    char *payload = "{\"message\":\"hello!\"}";

    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);

    topic_len = strlen(fmt) + strlen(product_key) + strlen(device_name) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, product_key, device_name);


    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)payload;
    topic_msg.payload_len = strlen(payload);

    res = IOT_MQTT_Publish(handle, topic, &topic_msg);
    if (res < 0) {
        EXAMPLE_TRACE("publish failed");
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}

void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    EXAMPLE_TRACE("msg->event_type : %d", msg->event_type);
}

/*
 *  NOTE: About demo topic of /${productKey}/${deviceName}/get
 *
 *  The demo device has been configured in IoT console (https://iot.console.aliyun.com)
 *  so that its /${productKey}/${deviceName}/get can both be subscribed and published
 *
 *  We design this to completely demonstrate publish & subscribe process, in this way
 *  MQTT client can receive original packet sent by itself
 *
 *  For new devices created by yourself, pub/sub privilege also requires being granted
 *  to its /${productKey}/${deviceName}/get for successfully running whole example
 */

int main(int argc, char *argv[])
{
    void *      pclient = NULL;
    int         res = 0;
    int         loop_cnt = 0;

    iotx_mqtt_region_types_t    region = IOTX_CLOUD_REGION_SHANGHAI;
    iotx_sign_mqtt_t            sign_mqtt;
    iotx_dev_meta_info_t        meta;
    iotx_mqtt_param_t           mqtt_params;

    EXAMPLE_TRACE("mqtt example");

    memset(&meta, 0, sizeof(iotx_dev_meta_info_t));
    HAL_GetProductKey(meta.product_key);
    HAL_GetDeviceName(meta.device_name);
    HAL_GetDeviceSecret(meta.device_secret);

    memset(&sign_mqtt, 0x0, sizeof(iotx_sign_mqtt_t));

    if (IOT_Sign_MQTT(region, &meta, &sign_mqtt) < 0) {
        return -1;
    }

#if 0   /* Uncomment this to show more information */
    EXAMPLE_TRACE("sign_mqtt.hostname: %s\n", sign_mqtt.hostname);
    EXAMPLE_TRACE("sign_mqtt.port    : %d\n", sign_mqtt.port);
    EXAMPLE_TRACE("sign_mqtt.username: %s\n", sign_mqtt.username);
    EXAMPLE_TRACE("sign_mqtt.password: %s\n", sign_mqtt.password);
    EXAMPLE_TRACE("sign_mqtt.clientid: %s\n", sign_mqtt.clientid);
#endif

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = sign_mqtt.port;
    mqtt_params.host = sign_mqtt.hostname;
    mqtt_params.client_id = sign_mqtt.clientid;
    mqtt_params.username = sign_mqtt.username;
    mqtt_params.password = sign_mqtt.password;

    /* MQTT message request timeout for waiting ACK in MQTT Protocol, default value is 2000ms */
    /* mqtt_params.request_timeout_ms = 2000; */

    /** 
     * 
     * MQTT clean session flag
     * 
     * If CleanSession is set to 0, the Server MUST resume communications with the Client based on state from 
     * the current Session (as identified by the Client identifier).  
     * 
     * If CleanSession is set to 1, the Client and Server MUST discard any previous Session and Start a new one.
     * 
     * default value is 0.
     */
    /* mqtt_params.clean_session = 0; */

    /** 
     * 
     * MQTT KeepAlive bytes
     * 
     * KeepAlive is the maximum time interval that is permitted to elapse between the point at which 
     * the Client finishes transmitting one Control Packet and the point it starts sending the next.
     * 
     * default value is 60000.
     */
    /* mqtt_params.keepalive_interval_ms = 60000; */

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


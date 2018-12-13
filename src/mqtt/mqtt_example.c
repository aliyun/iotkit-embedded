#include <stdio.h>
#include <string.h>
#include "infra_types.h"
#include "infra_defs.h"
#include "dev_sign_api.h"
#include "mqtt_api.h"

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);
int8_t HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int8_t HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);
uint64_t HAL_UptimeMs(void);
int8_t HAL_Snprintf(char *str, const int len, const char *fmt, ...);

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
            printf("Message Arrived: \n");
            printf("Topic  : %.*s\n",topic_info->topic_len,topic_info->ptopic);
            printf("Payload: %.*s\n",topic_info->payload_len,topic_info->payload);
            printf("\n");
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
    const char *fmt = "/sys/%s/%s/thing/event/+/post_reply";
    char *topic = NULL;
    int topic_len = 0;

    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);

    topic_len = strlen(fmt) + strlen(product_key) + strlen(device_name) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        printf("memory not enough\n");
        return -1;
    }
    memset(topic,0,topic_len);
    HAL_Snprintf(topic,topic_len,fmt,product_key,device_name);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
    if (res < 0) {
        printf("subscribe failed\n");
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
    const char *fmt = "/sys/%s/%s/thing/event/property/post";
    char *topic = NULL;
    int topic_len = 0;
    char *payload = "{\"id\":\"1\",\"version\":\"1.0\",\"params\":{\"LightSwitch\":1},\"method\":\"thing.event.property.post\"}";

    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);

    topic_len = strlen(fmt) + strlen(product_key) + strlen(device_name) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        printf("memory not enough\n");
        return -1;
    }
    memset(topic,0,topic_len);
    HAL_Snprintf(topic,topic_len,fmt,product_key,device_name);


    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)payload;
    topic_msg.payload_len = strlen(payload);

    res = IOT_MQTT_Publish(handle, topic, &topic_msg);
    if (res < 0) {
        printf("publish failed\n");
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}

void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    printf("msg->event_type : %d\n",msg->event_type);
}

int main(int argc, char *argv[])
{
    printf("mqtt example\n");

    int res = 0;
    void *pclient = NULL;
    uint64_t time_prev = 0;
    iotx_cloud_region_types_t region = IOTX_CLOUD_REGION_SHANGHAI;
    iotx_sign_mqtt_t sign_mqtt;
    iotx_mqtt_param_t mqtt_params;

    memset(&sign_mqtt, 0x0, sizeof(iotx_sign_mqtt_t));

    if (IOT_Sign_MQTT(region,&sign_mqtt) < 0) {
        return -1;
    }

    printf("sign_mqtt.hostname: %s\n",sign_mqtt.hostname);
    printf("sign_mqtt.port    : %d\n",sign_mqtt.port);
    printf("sign_mqtt.username: %s\n",sign_mqtt.username);
    printf("sign_mqtt.password: %s\n",sign_mqtt.password);
    printf("sign_mqtt.clientid: %s\n",sign_mqtt.clientid);

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

    HAL_Free(sign_mqtt.hostname);
    HAL_Free(sign_mqtt.username);
    HAL_Free(sign_mqtt.password);
    HAL_Free(sign_mqtt.clientid);

    res = example_subscribe(pclient);
    if (res < 0) {
        IOT_MQTT_Destroy(&pclient);
        return -1;
    }

    while(1) {
        if (HAL_UptimeMs() - time_prev > 5000) {
            example_publish(pclient);
            time_prev = HAL_UptimeMs();
        }

        IOT_MQTT_Yield(pclient, 200);
    }

    return 0;
}
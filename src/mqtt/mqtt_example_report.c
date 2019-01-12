#include <stdio.h>
#include <string.h>
#include "infra_types.h"
#include "infra_defs.h"
#include "dev_sign_api.h"
#include "mqtt_api.h"
#include "infra_log.h"

#define PID_STRING_LEN_MAX          64      /* do not larger than this value */
#define MID_STRING_LEN_MAX          64      /* do not larger than this value */

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

void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    HAL_Printf("msg->event_type : %d\n", msg->event_type);
}

int example_report_pid_mid(void *pclient, const char *pid, const char *mid)
{
    int res = 0;
    iotx_mqtt_topic_info_t topic_msg;
    char product_key[IOTX_PRODUCT_KEY_LEN] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN] = {0};

    const char topic_frag1[] = "/sys/";
    const char topic_frag2[] = "/";
    const char topic_frag3[] = "/thing/status/update";
    char topic[sizeof(topic_frag1) + IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN] = {0};

    const char payload_frag1[] = "{\"id\":\"pid_mid\",\"params\":{\"_sys_device_mid\":\"";
    const char payload_frag2[] = "\",\"_sys_device_pid\":\"";
    const char payload_frag3[] = "\"}}";
    char payload[sizeof(payload_frag1) + sizeof(payload_frag2) + sizeof(payload_frag3) + PID_STRING_LEN_MAX + MID_STRING_LEN_MAX] = {0};

    if (strlen(pid) > PID_STRING_LEN_MAX || strlen(mid) > MID_STRING_LEN_MAX) {
        return -1;
    }

    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);

    /* assemble the topic */
    memcpy(topic, topic_frag1, strlen(topic_frag1));
    memcpy(topic + strlen(topic), product_key, strlen(product_key));
    memcpy(topic + strlen(topic), topic_frag2, strlen(topic_frag2));
    memcpy(topic + strlen(topic), device_name, strlen(device_name));
    memcpy(topic + strlen(topic), topic_frag3, strlen(topic_frag3));
    HAL_Printf("pid_mid topic = %s\r\n", topic);

    /* assemble the topic */
    memcpy(payload, payload_frag1, strlen(payload_frag1));
    memcpy(payload + strlen(payload), mid, strlen(mid));
    memcpy(payload + strlen(payload), payload_frag2, strlen(payload_frag2));
    memcpy(payload + strlen(payload), pid, strlen(pid));
    memcpy(payload + strlen(payload), payload_frag3, strlen(payload_frag3));
    HAL_Printf("pid_mid payload = %s\r\n", payload);  

    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)payload;
    topic_msg.payload_len = strlen(payload);

    res = IOT_MQTT_Publish(pclient, topic, &topic_msg);
    if (res < 0) {
        HAL_Printf("publish pid_mid failed\r\n");
        return -1;
    }

    HAL_Printf("publish pid_mid succeed\r\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int res = 0;
    void *pclient = NULL;
    uint64_t time_prev = 0;
    iotx_mqtt_region_types_t region = IOTX_CLOUD_REGION_SHANGHAI;
    iotx_dev_meta_info_t meta;
    iotx_sign_mqtt_t sign_mqtt;
    iotx_mqtt_param_t mqtt_params;

    HAL_Printf("mqtt example\n");

    memset(&meta, 0, sizeof(iotx_dev_meta_info_t));
    HAL_GetProductKey(meta.product_key);
    HAL_GetDeviceName(meta.device_name);
    HAL_GetDeviceSecret(meta.device_secret);

    memset(&sign_mqtt, 0x0, sizeof(iotx_sign_mqtt_t));

    if (IOT_Sign_MQTT(region, &meta, &sign_mqtt) < 0) {
        return -1;
    }

    HAL_Printf("sign_mqtt.hostname: %s\n", sign_mqtt.hostname);
    HAL_Printf("sign_mqtt.port    : %d\n", sign_mqtt.port);
    HAL_Printf("sign_mqtt.username: %s\n", sign_mqtt.username);
    HAL_Printf("sign_mqtt.password: %s\n", sign_mqtt.password);
    HAL_Printf("sign_mqtt.clientid: %s\n", sign_mqtt.clientid);

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

    if (res < 0) {
        IOT_MQTT_Destroy(&pclient);
        return -1;
    }

    /* report module vendor id(partnerId = pid) and module id(= mid) here */
    example_report_pid_mid(pclient, "this is pid string", "this is mid string");

    while (1) {
        if (HAL_UptimeMs() - time_prev > 5000) {
            time_prev = HAL_UptimeMs();
            /* it's a pid_mid report demo, just return now */
            break;
        }

        IOT_MQTT_Yield(pclient, 200);
    }

    return 0;
}


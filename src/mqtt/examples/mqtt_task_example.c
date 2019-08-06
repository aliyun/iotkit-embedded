#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "infra_compat.h"
#include "dev_sign_api.h"
#include "mqtt_api.h"
#include "wrappers.h"
#include "cJSON.h"

char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1RIsMLz2BJ";
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "fSAF0hle6xL0oRWd";
char g_device_name[IOTX_DEVICE_NAME_LEN + 1]       = "example1";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "RDXf67itLqZCwdMCRrw0N5FHbv5D7jrE";

#define TASK_STATUS_IN_PROCESS "IN_PROGRESS"
#define TASK_STATUS_SUCCEEDED  "SUCCEEDED"
#define TASK_STATUS_FAILED     "FAILED"
#define TASK_STATUS_REJECTED   "REJECTED"

static void *pclient = NULL;
static void *g_task_pthread = NULL;
static uint8_t g_task_running = 0;

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

int example_task_update(void *handle, char *task_id, char *status, uint8_t progress)
{
    int             res = 0;
    const char     *fmt = "/sys/%s/%s/thing/job/update";
    char           *topic = NULL;
    int             topic_len = 0;
    char           *payload = NULL;
    cJSON          *root = NULL, *params = NULL;

    /* prepare topic */
    topic_len = strlen(fmt) + strlen(g_product_key) + strlen(g_device_name) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, g_product_key, g_device_name);

    /* prepare payload */
    root = cJSON_CreateObject();
    params = cJSON_CreateObject();
    if (root == NULL || params == NULL) {
        HAL_Free(topic);
        return -1;
    }
    cJSON_AddStringToObject(root, "id", "123");
    cJSON_AddStringToObject(root, "version", "1.0.0");
    cJSON_AddItemToObject(root, "params", params);
    cJSON_AddStringToObject(params, "taskId", task_id);
    cJSON_AddStringToObject(params, "status", status);
    if ((strlen(status) == strlen(TASK_STATUS_IN_PROCESS)) &&
        (memcmp(status, TASK_STATUS_IN_PROCESS, strlen(TASK_STATUS_IN_PROCESS)) == 0)) {
        cJSON_AddNumberToObject(params, "progress", progress);
    }

    payload = cJSON_PrintUnformatted(root);
    if (payload == NULL) {
        HAL_Free(topic);
        cJSON_Delete(root);
        return -1;
    }

    res = IOT_MQTT_Publish_Simple(0, topic, IOTX_MQTT_QOS0, payload, strlen(payload));
    if (res < 0) {
        EXAMPLE_TRACE("publish failed, res = %d", res);
        HAL_Free(topic);
        HAL_Free(payload);
        cJSON_Delete(root);
        return -1;
    }

    HAL_Free(topic);
    HAL_Free(payload);
    cJSON_Delete(root);
    return 0;
}

void* example_task_thread(void *param)
{
    uint8_t progress = 0;
    cJSON *root = (cJSON *)param;
    cJSON *params = NULL, *task = NULL, *task_id = NULL, *status = NULL, *job_document = NULL;

    g_task_running = 1;
    if ((params = cJSON_GetObjectItem(root, "params")) == NULL ||
        (task = cJSON_GetObjectItem(params, "task")) == NULL ||
        (task_id = cJSON_GetObjectItem(task, "taskId")) == NULL ||
        (status = cJSON_GetObjectItem(task, "status")) == NULL ||
        (job_document = cJSON_GetObjectItem(task, "jobDocument")) == NULL) {
        cJSON_Delete(root);
        g_task_running = 0;
        return NULL;
    }
    EXAMPLE_TRACE("task id     : %s\n", task_id->valuestring);
    EXAMPLE_TRACE("status      : %s\n", status->valuestring);

    while(progress <= 100) {
        example_task_update(pclient, task_id->valuestring, TASK_STATUS_IN_PROCESS, progress);
        progress += 10;
        HAL_SleepMs(5000);
    }
    example_task_update(pclient, task_id->valuestring, TASK_STATUS_IN_PROCESS, 100);
    example_task_update(pclient, task_id->valuestring, TASK_STATUS_SUCCEEDED, 100);

    cJSON_Delete(root);
    g_task_running = 0;
    return NULL;
}

void example_task_notify(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED: {
            int res = 0;
            char *job = NULL;
            cJSON *root = NULL;

            /* print topic name and topic message */
            EXAMPLE_TRACE("Message Arrived:");
            EXAMPLE_TRACE("Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
            EXAMPLE_TRACE("Payload: %.*s", topic_info->payload_len, topic_info->payload);
            EXAMPLE_TRACE("\n");

            /* check the task thread is running */
            if (g_task_running) {
                /* TODO: */

                return;
            }

            /* parse job json */
            job = malloc(topic_info->payload_len + 1);
            if (job == NULL) {
                return;
            }
            memset(job, 0, topic_info->payload_len + 1);
            memcpy(job, topic_info->payload, topic_info->payload_len);

            root = cJSON_Parse(job);
            free(job);
            if (root == NULL) {
                return ;
            }

            HAL_ThreadCreate(&g_task_pthread,example_task_thread, root, NULL, NULL);
        }
            break;
        default:
            break;
    }
}

int example_task_subscribe(void *handle)
{
    int res = 0;
    const char *fmt = "/sys/%s/%s/thing/job/notify";
    char *topic = NULL;
    int topic_len = 0;

    topic_len = strlen(fmt) + strlen(g_product_key) + strlen(g_device_name) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, g_product_key, g_device_name);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_task_notify, NULL);
    if (res < 0) {
        EXAMPLE_TRACE("subscribe failed");
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
 *  NOTE: About demo topic of /${productKey}/${deviceName}/user/get
 *
 *  The demo device has been configured in IoT console (https://iot.console.aliyun.com)
 *  so that its /${productKey}/${deviceName}/user/get can both be subscribed and published
 *
 *  We design this to completely demonstrate publish & subscribe process, in this way
 *  MQTT client can receive original packet sent by itself
 *
 *  For new devices created by yourself, pub/sub privilege also requires being granted
 *  to its /${productKey}/${deviceName}/user/get for successfully running whole example
 */

int main(int argc, char *argv[])
{
    int                     res = 0;
    int                     loop_cnt = 0;
    iotx_mqtt_param_t       mqtt_params;

    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, g_product_key);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_NAME, g_device_name);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_SECRET, g_device_secret);

    EXAMPLE_TRACE("mqtt example");

    /* Initialize MQTT parameter */
    /*
     * Note:
     *
     * If you did NOT set value for members of mqtt_params, SDK will use their default values
     * If you wish to customize some parameter, just un-comment value assigning expressions below
     *
     **/
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    /**
     *
     *  MQTT connect hostname string
     *
     *  MQTT server's hostname can be customized here
     *
     *  default value is ${productKey}.iot-as-mqtt.cn-shanghai.aliyuncs.com
     */
    /* mqtt_params.host = "something.iot-as-mqtt.cn-shanghai.aliyuncs.com"; */

    /**
     *
     *  MQTT connect port number
     *
     *  TCP/TLS port which can be 443 or 1883 or 80 or etc, you can customize it here
     *
     *  default value is 1883 in TCP case, and 443 in TLS case
     */
    /* mqtt_params.port = 1883; */

    /**
     *
     * MQTT request timeout interval
     *
     * MQTT message request timeout for waiting ACK in MQTT Protocol
     *
     * default value is 2000ms.
     */
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
     * MQTT keepAlive interval
     *
     * KeepAlive is the maximum time interval that is permitted to elapse between the point at which
     * the Client finishes transmitting one Control Packet and the point it starts sending the next.
     *
     * default value is 60000.
     */
    /* mqtt_params.keepalive_interval_ms = 60000; */

    /**
     *
     * MQTT write buffer size
     *
     * Write buffer is allocated to place upstream MQTT messages, MQTT client will be limitted
     * to send packet no longer than this to Cloud
     *
     * default value is 1024.
     *
     */
    /* mqtt_params.write_buf_size = 1024; */

    /**
     *
     * MQTT read buffer size
     *
     * Write buffer is allocated to place downstream MQTT messages, MQTT client will be limitted
     * to recv packet no longer than this from Cloud
     *
     * default value is 1024.
     *
     */
    /* mqtt_params.read_buf_size = 1024; */

    /**
     *
     * MQTT event callback function
     *
     * Event callback function will be called by SDK when it want to notify user what is happening inside itself
     *
     * default value is NULL, which means PUB/SUB event won't be exposed.
     *
     */
    mqtt_params.host = "iot-test-daily.iot-as-mqtt.unify.aliyuncs.com";
    mqtt_params.port = 1883;
    mqtt_params.handle_event.h_fp = example_event_handle;

    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        return -1;
    }

    res = example_task_subscribe(pclient);
    if (res < 0) {
        IOT_MQTT_Destroy(&pclient);
        return -1;
    }

    while (1) {

        IOT_MQTT_Yield(pclient, 200);

        loop_cnt += 1;
    }

    return 0;
}


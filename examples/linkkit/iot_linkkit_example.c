#include "stdio.h"
#include "iot_export_linkkit.h"
#include "cJSON.h"

#define PRODUCT_KEY     "a13Npv1vjZ4"
#define PRODUCT_SECRET  "9MQCVFRsNcWTg7ak"
#define DEVICE_NAME     "example_zc"
#define DEVICE_SECRET   "ZlexLJ4G0aXiSwkGmUFWuZBLLySKcG8h"

#define USER_EXAMPLE_YIELD_TIMEOUT_MS (200)

#define EXAMPLE_TRACE(...)                               \
    do {                                                     \
        HAL_Printf("\033[1;31;40m%s.%d: ", __func__, __LINE__);  \
        HAL_Printf(__VA_ARGS__);                                 \
        HAL_Printf("\033[0m\n\n");                                   \
    } while (0)

static int master_devid = 0;

static int user_connected_event_handler(void)
{
    EXAMPLE_TRACE("Cloud Connected");
    return 0;
}

static int user_disconnected_event_handler(void)
{
    EXAMPLE_TRACE("Cloud Disconnected");
    return 0;
}

static int user_down_raw_event_handler(const int devid, const unsigned char *payload, const int payload_len)
{
    EXAMPLE_TRACE("Down Raw Data, Devid: %d, Payload: %.*s", devid, payload_len, payload);
    return 0;
}

int user_service_request_event_handler(const int devid, const char *serviceid, const int serviceid_len,
                                       const char *request, const int request_len,
                                       char **response, int *response_len, int *response_sync)
{
    int contrastratio = 0;
    cJSON *root = NULL, *item_transparency = NULL;
    const char *response_fmt = "{\"Contrastratio\":%d}";
    EXAMPLE_TRACE("Service Set Received, Devid: %d, Service ID: %.*s,Payload: %.*s", devid, serviceid_len, serviceid,
                  request_len, request);

    /* Parse Root */
    root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root)) {
        EXAMPLE_TRACE("JSON Parse Error");
        return -1;
    }


    if (strlen("Custom") == serviceid_len && memcmp("Custom", serviceid, serviceid_len) == 0) {
        /* Parse Item */
        item_transparency = cJSON_GetObjectItem(root, "transparency");
        if (item_transparency == NULL || !cJSON_IsNumber(item_transparency)) {
            cJSON_Delete(root);
            return -1;
        }
        EXAMPLE_TRACE("transparency: %d", item_transparency->valueint);
        contrastratio = item_transparency->valueint + 1;
    }
    cJSON_Delete(root);

    /* Send Service Response To Cloud */
    *response_len = strlen(response_fmt) + 10 + 1;
    *response = HAL_Malloc(*response_len);
    if (*response == NULL) {
        EXAMPLE_TRACE("Memory Not Enough");
        return -1;
    }
    memset(*response, 0, *response_len);
    HAL_Snprintf(*response, *response_len, response_fmt, contrastratio);

    return 0;
}

int user_property_set_event_handler(const int devid, const char *payload, const int payload_len)
{
    int res = 0;
    EXAMPLE_TRACE("Property Set Received, Devid: %d, Payload: %s", devid, payload);

    res = IOT_Linkkit_Post(master_devid, IOTX_LINKKIT_MSG_POST_PROPERTY, NULL, 0, (unsigned char *)payload, payload_len);
    EXAMPLE_TRACE("Post Property Message ID: %d", res);

    return 0;
}

int user_post_reply_event_handler(const int devid, const int msgid, const int code, const char *payload,
                                  const int payload_len)
{
    const char *reply = (payload == NULL) ? ("NULL") : (payload);
    const int reply_len = (payload_len == 0) ? (strlen("NULL")) : (payload_len);

    EXAMPLE_TRACE("Message Post Reply Received, Devid: %d, Message ID: %d, Code: %d, Payload: %.*s", devid, msgid, code,
                  reply_len,
                  reply);
    return 0;
}

static iotx_linkkit_event_handler_t user_event_handler = {
    .connected =       user_connected_event_handler,
    .disconnected =    user_disconnected_event_handler,
    .down_raw =        user_down_raw_event_handler,
    .service_request = user_service_request_event_handler,
    .property_set =    user_property_set_event_handler,
    .post_reply =      user_post_reply_event_handler,
};

static uint64_t user_update_sec(void)
{
    static uint64_t time_start_ms = 0;

    if (time_start_ms == 0) {
        time_start_ms = HAL_UptimeMs();
    }

    return (HAL_UptimeMs() - time_start_ms) / 1000;
}

int main(int argc, char *argv[])
{
    int res = 0;
    uint64_t time_prev_sec = 0, time_now_sec = 0;
    iotx_linkkit_dev_meta_info_t master_meta_info;

    /* Event Post Example */
    char *event_id = "Error";
    char *event_payload = "{\"ErrorCode\":0}";

    /* Device Info Update Example */
    char *device_info_update = "[{\"attrKey\":\"abc\",\"attrValue\":\"hello,world\"}]";

    /* Device Info Delete Example */
    char *device_info_delete = "[{\"attrKey\":\"abc\"}]";

    IOT_OpenLog("iot_linkkit");
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    memset(&master_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
    memcpy(master_meta_info.product_key, PRODUCT_KEY, strlen(PRODUCT_KEY));
    memcpy(master_meta_info.product_secret, PRODUCT_SECRET, strlen(PRODUCT_SECRET));
    memcpy(master_meta_info.device_name, DEVICE_NAME, strlen(DEVICE_NAME));
    memcpy(master_meta_info.device_secret, DEVICE_SECRET, strlen(DEVICE_SECRET));

    /* Create Master Device Resources */
    master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &master_meta_info);
    if (master_devid < 0) {
        EXAMPLE_TRACE("IOT_Linkkit_Open Failed\n");
        return -1;
    }

    /* Choose Login Server */
    int domain_type = IOTX_CLOUD_DOMAIN_SH;
    IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);

    /* Choose Login Method */
    int dynamic_register = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    /* Choose Whether You Need Post Property Reply */
    int post_property_reply = 0;
    IOT_Linkkit_Ioctl(master_devid, IOTX_LINKKIT_CMD_OPTION_PROPERTY_POST_REPLY, (void *)&post_property_reply);

    /* Choose Whether You Need Post Event Reply */
    int post_event_reply = 0;
    IOT_Linkkit_Ioctl(master_devid, IOTX_LINKKIT_CMD_OPTION_EVENT_POST_REPLY, (void *)&post_event_reply);

    /* Start Connect Aliyun Server */
    res = IOT_Linkkit_Start(master_devid, &user_event_handler);
    if (res < 0) {
        EXAMPLE_TRACE("IOT_Linkkit_Start Failed\n");
        return -1;
    }

    while (1) {
        IOT_Linkkit_Yield(USER_EXAMPLE_YIELD_TIMEOUT_MS);

        time_now_sec = user_update_sec();
        if (time_prev_sec == time_now_sec) {
            continue;
        }

        if (time_now_sec % 11 == 0) {
            res = IOT_Linkkit_Post(master_devid, IOTX_LINKKIT_MSG_POST_EVENT, event_id, strlen(event_id),
                                   (unsigned char *)event_payload, strlen(event_payload));
            EXAMPLE_TRACE("Post Event Message ID: %d", res);
        }

        if (time_now_sec % 13 == 0) {
            res = IOT_Linkkit_Post(master_devid, IOTX_LINKKIT_MSG_DEVICEINFO_UPDATE, event_id, strlen(event_id),
                                   (unsigned char *)device_info_update, strlen(device_info_update));
            EXAMPLE_TRACE("Device Info Update Result: %d", res);
        }

        if (time_now_sec % 17 == 0) {
            res = IOT_Linkkit_Post(master_devid, IOTX_LINKKIT_MSG_DEVICEINFO_DELETE, event_id, strlen(event_id),
                                   (unsigned char *)device_info_delete, strlen(device_info_delete));
            EXAMPLE_TRACE("Device Info Update Result: %d", res);
        }

        time_prev_sec = time_now_sec;
    }
    return 0;
}
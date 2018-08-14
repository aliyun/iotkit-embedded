#include "stdio.h"
#include "iot_export_linkkit.h"

#define PRODUCT_KEY     "a13Npv1vjZ4"
#define PRODUCT_SECRET  "9MQCVFRsNcWTg7ak"
#define DEVICE_NAME     "example1"
#define DEVICE_SECRET   "T85uRb3b2L3rEMydTTAwgSkh78SJoTG6"

#define USER_EXAMPLE_YIELD_TIMEOUT_MS (200)

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

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

int user_service_set_event_handler(const int devid, const char *service_id, const char *request, const int request_len,
                                   char **response, int *response_len)
{
    EXAMPLE_TRACE("Service Set Received, Devid: %d, Service ID: %s,Payload: %.*s", devid, service_id, request_len, request);
    return 0;
}

int user_property_set_event_handler(const int devid, const char *payload, const int payload_len)
{
    EXAMPLE_TRACE("Property Set Received, Devid: %d, Payload: %.*s", devid, payload_len, payload);
    return 0;
}

int user_post_reply_event_handler(const int devid, const int msgid, const int code, const char *payload,
                                  const int payload_len)
{
    const char *reply = (payload == NULL) ? ("NULL") : (payload);
    const int reply_len = (payload_len == 0) ? (strlen("NULL")) : (payload_len);

    EXAMPLE_TRACE("Property Reply Received, Devid: %d, MsgID: %d, Code: %d, Payload: %.*s", devid, msgid, code, reply_len,
                  reply);
    return 0;
}

static iotx_linkkit_event_handler_t user_event_handler = {
    .connected =       user_connected_event_handler,
    .disconnected =    user_disconnected_event_handler,
    .down_raw =        user_down_raw_event_handler,
    .service_set =     user_service_set_event_handler,
    .property_set =    user_property_set_event_handler,
    .post_reply =      user_post_reply_event_handler,
};

int main(int argc, char *argv[])
{
    int res = 0, master_devid = 0;
    iotx_linkkit_dev_meta_info_t master_meta_info;

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

    /* Start Connect Aliyun Server */
    res = IOT_Linkkit_Start(master_devid, &user_event_handler);
    if (res < 0) {
        EXAMPLE_TRACE("IOT_Linkkit_Start Failed\n");
        return -1;
    }

    while (1) {
        IOT_Linkkit_Yield(USER_EXAMPLE_YIELD_TIMEOUT_MS);
    }
    return 0;
}
#include <stdio.h>
#include <string.h>

#include "alink_api.h"
#include "alink_wrapper.h"

#define EXAMPLE_TRACE(...)                               \
    do {                                                     \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__);  \
        HAL_Printf(__VA_ARGS__);                                 \
        HAL_Printf("\033[0m\r\n");                                   \
    } while (0)


#define PROP_ALINK1_TEST   "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"test1\":1234},\"method\":\"thing.event.property.post\"}"

#define ALINK2_PROP_POST_DATA       "{\"Data\": \"1024\"}"
#define ALINK2_DEVINFO_POST_DATA    "[{\"attrKey\":\"devinfo_k\",\"attrValue\":\"devinfo_v\"}]"

#define ALINK2_PROP_POST_DATA_TMP   "{\"BatteryRemain\": 2.9,\"TiltValue\":100,\"HeartBeatInterval\": 1234}"
#define ALINK2_EVENT_POST_DATA      "{\"intParam\": 400}"
#define ALINK2_EVENT_POST_EMPTY     "{}"


/** type define **/
typedef struct {
    int master_devid;
    int cloud_connected;
    int master_initialized;
    int thread_running;
    void *example_thread_yield;
    void *example_thread1;
    void *example_thread2;
} user_example_ctx_t;


/** local variables define */
static user_example_ctx_t g_user_example_ctx;

static user_example_ctx_t *user_example_get_ctx(void)
{
    return &g_user_example_ctx;
}

/** connected event **/
static int user_connected_event_handler(uint32_t devid)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("Cloud Connected");
    user_example_ctx->cloud_connected = 1;
    return 0;
}

/** recv property set request from cloud **/
static int user_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    EXAMPLE_TRACE("Property Set Received, Devid: %d, Request: %s", devid, request);

    res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                             (unsigned char *)request, request_len);
    EXAMPLE_TRACE("Post Property Message ID: %d", res);

    return 0;
}

/** **/
static int user_report_reply_event_handler(const int devid, const int msgid, const int code, const char *reply,
        const int reply_len)
{
    const char *reply_value = (reply == NULL) ? ("NULL") : (reply);
    const int reply_value_len = (reply_len == 0) ? (strlen("NULL")) : (reply_len);

    EXAMPLE_TRACE("Message Post Reply Received, Devid: %d, Message ID: %d, Code: %d, Reply: %.*s", devid, msgid, code,
                  reply_value_len,
                  reply_value);
    return 0;
}

/** porperty get **/
static int user_property_get_event_handler(const int devid, const char *request, const int request_len, char **response, int *response_len)
{
    uint32_t len = strlen("{\"test\": 12344}");
    char *rsp = HAL_Malloc(len);

    EXAMPLE_TRACE("Property Get Received, Devid: %d, Request: %s", devid, request);
    
    memset(rsp, 0, len);
    memcpy(rsp, "{\"test\": 12344}", len);

    *response = rsp;
    *response_len = len;

    /* return SUCCESS_RETURN; */
    return FAIL_RETURN;
}

/** service request **/
static int user_service_request_event_handler(const int devid, const char *serviceid, const int serviceid_len,
        const char *request, const int request_len,
        char **response, int *response_len)
{
    uint32_t len = strlen("{\"test\": 12344}");
    char *rsp = HAL_Malloc(len);

    EXAMPLE_TRACE("Service Request Received, Devid: %d, Service ID: %.*s, Payload: %s", devid, serviceid_len,
                  serviceid,
                  request);

    memset(rsp, 0, len);
    memcpy(rsp, "{\"Mode\": 1}", len);

    *response = rsp;
    *response_len = len;

    return 0;
}

/** downstream raw data **/
static int user_down_raw_data_arrived_event_handler(const int devid, const unsigned char *payload,
        const int payload_len)
{
    uint32_t i;
    EXAMPLE_TRACE("Down Raw Message, Devid: %d, Payload Length: %d", devid, payload_len);

    for (i = 0; i<payload_len; i++) {
        printf("0x%02x ", payload[i]);
    }
    printf("\r\n");

    return 0;
}

/** device inited successfully **/
static int user_initialized(const int devid)
{
    EXAMPLE_TRACE("Device Initialized, Devid: %d", devid);

    return 0;
}


void *example_yield_thread(void *args)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    while (user_example_ctx->thread_running) {
        IOT_Linkkit_Yield(200);
    }

    return NULL;
}


int main(int argc, char **argv)
{
    user_example_ctx_t *user_example_ctx;
    uint32_t cnt = 0;
    int res = FAIL_RETURN;
    static iotx_linkkit_dev_meta_info_t dev_info;

    /* get triple metadata from HAL */
    HAL_GetProductKey(dev_info.product_key);
    HAL_GetProductSecret(dev_info.product_secret);
    HAL_GetDeviceName(dev_info.device_name);
    HAL_GetDeviceSecret(dev_info.device_secret);

    printf("alink start\r\n");

    user_example_ctx = user_example_get_ctx();
    memset(user_example_ctx, 0, sizeof(user_example_ctx_t));
    user_example_ctx->thread_running = 1;

    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_GET, user_property_get_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, user_service_request_event_handler);
    IOT_RegisterCallback(ITE_RAWDATA_ARRIVED, user_down_raw_data_arrived_event_handler);    
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, user_initialized);

    /* Create Master Device Resources */
    user_example_ctx->master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &dev_info);
    if (user_example_ctx->master_devid < 0) {
        EXAMPLE_TRACE("IOT_Linkkit_Open Failed\n");
        return -1;
    }
    EXAMPLE_TRACE("IOT_Linkkit_Open Succeed\n");

    res = IOT_Linkkit_Connect(0);
    if (res < SUCCESS_RETURN) {
        return -1;
    }
    EXAMPLE_TRACE("IOT_Linkkit_Connect, res = %d", res);

    while (1) {

        IOT_Linkkit_Yield(2000);

        HAL_SleepMs(2000);

        res = IOT_Linkkit_Report(IOTX_LINKKIT_DEV_TYPE_MASTER, ITM_MSG_POST_PROPERTY, (uint8_t *)ALINK2_PROP_POST_DATA_TMP, strlen(ALINK2_PROP_POST_DATA_TMP));
        EXAMPLE_TRACE("post property, res = %d", res);

        res = IOT_Linkkit_TriggerEvent(IOTX_LINKKIT_DEV_TYPE_MASTER, "Empty", strlen("Empty"), ALINK2_EVENT_POST_EMPTY, strlen(ALINK2_EVENT_POST_EMPTY));
        EXAMPLE_TRACE("post event, res = %d", res);

        {
            uint8_t raw_data[] = "\x00\x00\x22\x33\x44\x12\x32\x01\x3f\xa0\x00\x00";
            res = IOT_Linkkit_Report(IOTX_LINKKIT_DEV_TYPE_MASTER, ITM_MSG_POST_RAW_DATA, raw_data, sizeof(raw_data) - 1);
            EXAMPLE_TRACE("post raw, res = %d", res);
        }

        res = IOT_Linkkit_Report(IOTX_LINKKIT_DEV_TYPE_MASTER, ITM_MSG_DEVICEINFO_UPDATE, (uint8_t *)ALINK2_DEVINFO_POST_DATA, strlen(ALINK2_DEVINFO_POST_DATA));
        EXAMPLE_TRACE("post devinfo, res = %d", res);    

        if (++cnt > 20) {
            user_example_ctx->thread_running = 0;
            HAL_SleepMs(1000);

            HAL_ThreadDelete(user_example_ctx->example_thread_yield);
            IOT_Linkkit_Close(IOTX_LINKKIT_DEV_TYPE_MASTER);
            break;
        }
    }

    printf("alink stop\r\n");
}



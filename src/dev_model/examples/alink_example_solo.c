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

#define EXAMPLE_RAWDATA_DEVICE      (0) /* setup to 1 to support rawdata device demo */
#define EXAMPLE_MASTER_DEVID        (0)

#define EXAMPLE_PROP_POST_DATA      "{\"WaterConsumption\": 100.123}"
#define EXAMPLE_EVENT_POST_DATA     "{\"ErrorCode\": 1}"
#define EXAMPLE_SERVICE_RSP_DATA    "{\"WaterConsumption\": 123.456}"
#define EXAMPLE_DEVINFO_POST_DATA   "[{\"attrKey\":\"devinfo_key\",\"attrValue\":\"devinfo_value\"}]"

/** type define **/
typedef struct {
    int master_devid;
    int cloud_connected;
    int master_initialized;
    int thread_running;
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

/** recv response message from cloud **/
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

/** recv service request from cloud **/
static int user_service_request_event_handler(const int devid, const char *serviceid, const int serviceid_len,
        const char *request, const int request_len,
        char **response, int *response_len)
{
    uint32_t len = strlen(EXAMPLE_SERVICE_RSP_DATA);
    char *rsp = HAL_Malloc(len);
    if (rsp == NULL) {
        return -1;
    }

    EXAMPLE_TRACE("Service Request Received, Devid: %d, Service ID: %.*s, Payload: %s", devid, serviceid_len,
                  serviceid,
                  request);

    memset(rsp, 0, len);
    memcpy(rsp, EXAMPLE_SERVICE_RSP_DATA, len);

    *response = rsp;
    *response_len = len;

    return 0;
}

#if EXAMPLE_RAWDATA_DEVICE
/** recv rawdata from cloud **/
static int user_raw_data_arrived_event_handler(int devid, const uint8_t *payload, uint32_t payload_len)
{
    uint32_t i;
    EXAMPLE_TRACE("Down Raw Message, Devid: %d, Payload Length: %d", devid, payload_len);

    for (i=0; i<payload_len; i++) {
        printf("0x%02x ", payload[i]);
    }
    printf("\n");

    return 0;
}
#endif

int main(int argc, char **argv)
{
    int res = FAIL_RETURN;
    user_example_ctx_t *user_example_ctx;
    static iotx_linkkit_dev_meta_info_t dev_info;
    uint32_t cnt = 0;

    /* get triple metadata from HAL */
    HAL_GetProductKey(dev_info.product_key);
    HAL_GetDeviceName(dev_info.device_name);
    HAL_GetDeviceSecret(dev_info.device_secret);

    printf("alink start\r\n");

    user_example_ctx = user_example_get_ctx();
    memset(user_example_ctx, 0, sizeof(user_example_ctx_t));
    user_example_ctx->thread_running = 1;

    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, user_service_request_event_handler);
#if EXAMPLE_RAWDATA_DEVICE
    IOT_RegisterCallback(ITE_RAWDATA_ARRIVED, user_raw_data_arrived_event_handler);
#endif

    /* init linkkit sdk and create device resource */
    res = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &dev_info);
    if (res < SUCCESS_RETURN) {
        EXAMPLE_TRACE("IOT_Linkkit_Open Failed, res = %d\n", res);
        return -1;
    }
    EXAMPLE_TRACE("IOT_Linkkit_Open Succeed");

    res = IOT_Linkkit_Connect(EXAMPLE_MASTER_DEVID);
    if (res < SUCCESS_RETURN) {
        EXAMPLE_TRACE("IOT_Linkkit_Connect Failed, res = %d\n", res);
        return -1;
    }
    EXAMPLE_TRACE("IOT_Linkkit_Connect Succeed");

    while (1) {

        IOT_Linkkit_Yield(2000);

#if (EXAMPLE_RAWDATA_DEVICE == 0)
        /* post property every odd number */
        if ((cnt % 2) != 0) {
            res = IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_POST_PROPERTY, (uint8_t *)EXAMPLE_PROP_POST_DATA, strlen(EXAMPLE_PROP_POST_DATA));
            EXAMPLE_TRACE("post property, res = %d", res);
        }

        /* post event every even number */
        if ((cnt % 2) == 0) {
            res = IOT_Linkkit_TriggerEvent(EXAMPLE_MASTER_DEVID, "Error", strlen("Error"), EXAMPLE_EVENT_POST_DATA, strlen(EXAMPLE_EVENT_POST_DATA));
            EXAMPLE_TRACE("post event, res = %d", res);
        }

#else
        /* post raw data to cloud */
        if ((cnt % 3) == 0) {
            res = IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_POST_RAW_DATA, (uint8_t *)"\x02", 1);
            EXAMPLE_TRACE("post rawdata, res = %d", res);
        }
#endif

        if (cnt == 10) {
            res = IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_DEVICEINFO_UPDATE, (uint8_t *)EXAMPLE_DEVINFO_POST_DATA, strlen(EXAMPLE_DEVINFO_POST_DATA));
            EXAMPLE_TRACE("post devinfo, res = %d", res);
        }

        if (++cnt > 100) {
            break;
        }
    }

    IOT_Linkkit_Close(EXAMPLE_MASTER_DEVID);
    printf("alink stop\r\n");

    return 0;
}


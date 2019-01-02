#include <stdio.h>
#include <string.h>

#include "alink_api.h"
#include "alink_wrapper.h"

extern char _product_key[IOTX_PRODUCT_KEY_LEN + 1];
extern char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1];
extern char _device_name[IOTX_DEVICE_NAME_LEN + 1];
extern char _device_secret[IOTX_DEVICE_SECRET_LEN + 1];


#define EXAMPLE_TRACE(...)                               \
    do {                                                     \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__);  \
        HAL_Printf(__VA_ARGS__);                                 \
        HAL_Printf("\033[0m\r\n");                                   \
    } while (0)


#define PROP_ALINK1_TEST   "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"test1\":1234},\"method\":\"thing.event.property.post\"}"

#define ALINK2_PROP_POST_DATA       "{\"Data\": \"1024\"}"
#define ALINK2_EVENT_POST_DATA      "{\"ErrorCode\": 400}"

/** type define **/
typedef struct {
    int master_devid;
    int cloud_connected;
    int master_initialized;
} user_example_ctx_t;


/** local variables define */
static user_example_ctx_t g_user_example_ctx;




/** internal test **/
extern int alink_downstream_invoke_mock(const char *uri_string);
extern void subdev_hash_iterator(void);
extern int subdev_hash_remove(uint32_t devid);
int alink_subdev_mgr_deinit(void);



static user_example_ctx_t *user_example_get_ctx(void)
{
    return &g_user_example_ctx;
}



/** recv property set req from cloud **/
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

/** **/
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

/** **/
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
    memcpy(rsp, "{\"test\": 12344}", len);

    *response = rsp;
    *response_len = len;

    return 0;
}

/** **/
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









/** permit join **/
int user_permit_join_event_handler(const char *product_key, const int time)
{
    /* user_example_ctx_t *user_example_ctx = user_example_get_ctx(); */

    EXAMPLE_TRACE("Product Key: %s, Time: %d", product_key, time);

    /* user_example_ctx->permit_join = 1; */

    return 0;
}











int main(int argc, char **argv)
{
    user_example_ctx_t *user_example_ctx;
    uint32_t cnt = 0;
    static iotx_dev_meta_info_t dev_info = {
        .product_key = "a1OFrRjV8nz",
        .product_secret = "EfFYTuX1GjMDvw6l",
        .device_name = "develop_01",
        .device_secret = "7dqP7Sg1C2mKjajtFCQjyrh9ziR3wOMC"
    };

    static iotx_dev_meta_info_t subdev_info = {
        .product_key = "a1OFrRjV8nz",
        .product_secret = "EfFYTuX1GjMDvw6l",
        .device_name = "develop_02",
        .device_secret = "\0"
    };

    printf("alink start\r\n");

    LITE_set_loglevel(5);

    user_example_ctx = user_example_get_ctx();
    memset(user_example_ctx, 0, sizeof(user_example_ctx_t));


    memset(_product_key,0,IOTX_PRODUCT_KEY_LEN + 1);
    memcpy(_product_key,dev_info.product_key,strlen(dev_info.product_key));

    memset(_product_secret,0,IOTX_PRODUCT_SECRET_LEN + 1);
    memcpy(_product_secret,dev_info.product_secret,strlen(dev_info.product_secret));

    memset(_device_name,0,IOTX_DEVICE_NAME_LEN + 1);
    memcpy(_device_name,dev_info.device_name,strlen(dev_info.device_name));

    memset(_device_secret,0,IOTX_DEVICE_SECRET_LEN + 1);
    memcpy(_device_secret,dev_info.device_secret,strlen(dev_info.device_secret));



    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_GET, user_property_get_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, user_service_request_event_handler);
    IOT_RegisterCallback(ITE_RAWDATA_ARRIVED, user_down_raw_data_arrived_event_handler);    


    IOT_RegisterCallback(ITE_PERMIT_JOIN, user_permit_join_event_handler);


    /* Create Master Device Resources */
    user_example_ctx->master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &dev_info);
    if (user_example_ctx->master_devid < 0) {
        EXAMPLE_TRACE("IOT_Linkkit_Open Failed\n");
        return -1;
    }

    IOT_Linkkit_Connect(0);

    {
        int res;
        int subdev_id = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_SLAVE, &subdev_info);
        printf("subdev id = %d\r\n", subdev_id);

        res = IOT_Linkkit_Connect(subdev_id);
        printf("subdev conn res = %d\r\n", res);
    }


    /*
    {
        uint32_t devid[160];
        int i = 0;
        for (i=0; i<160; i++) {
            dev_info.device_name[9]++;
            devid[i] = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_SLAVE, &dev_info);
            printf("open sudev, devid = %d", devid[i]);
        }
        subdev_hash_iterator();

        for (i=159; i>=0; --i) {
            printf("removed devid[%d] = %d", i, devid[i]);

            subdev_hash_remove(devid[i]);
        }

        subdev_hash_iterator();
    }
    */

    while (1) {
        IOT_Linkkit_Yield(200);

        HAL_SleepMs(2000);

        #if (0)
        alink_downstream_invoke_mock("rsp/sys/thing/property/post");
        alink_downstream_invoke_mock("req/sys/thing/property/put");
        alink_downstream_invoke_mock("req/sys/thing/property/get");
        alink_downstream_invoke_mock("rsp/sys/thing/event/post");
        alink_downstream_invoke_mock("req/sys/thing/service/put");
        alink_downstream_invoke_mock("rsp/sys/thing/raw/post");
        alink_downstream_invoke_mock("req/sys/thing/raw/put");
        alink_downstream_invoke_mock("rsp/sys/sub/register/post");
        alink_downstream_invoke_mock("rsp/sys/sub/register/delete");
        alink_downstream_invoke_mock("rsp/sys/sub/login/post");
        alink_downstream_invoke_mock("rsp/sys/sub/login/delete");
        alink_downstream_invoke_mock("rsp/sys/thing/topo/post");
        alink_downstream_invoke_mock("rsp/sys/thing/topo/delete");
        alink_downstream_invoke_mock("rsp/sys/thing/topo/get");
        alink_downstream_invoke_mock("rsp/sys/sub/list/post");
        alink_downstream_invoke_mock("rsp/sys/sub/list/put");
        alink_downstream_invoke_mock("req/sys/gw/permit/put");
        alink_downstream_invoke_mock("req/sys/gw/config/put");
        alink_downstream_invoke_mock("rsp/sys/thing/devinfo/post");
        alink_downstream_invoke_mock("rsp/sys/thing/devinfo/get");
        alink_downstream_invoke_mock("rsp/sys/thing/devinfo/delete");
        #endif

        IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)ALINK2_PROP_POST_DATA, strlen(ALINK2_PROP_POST_DATA));

        IOT_Linkkit_TriggerEvent(0, "Error", strlen("Error"), ALINK2_EVENT_POST_DATA, strlen(ALINK2_EVENT_POST_DATA));


        if (++cnt > 5) {
            IOT_Linkkit_Close(IOTX_LINKKIT_DEV_TYPE_MASTER);
            break;
        }
    }

    printf("alink stop\r\n");
}



#include <stdio.h>
#include <string.h>

#include "alink_api.h"
#include "alink_wrapper.h"

#include "iotx_alink_internal.h"        /* test used */

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

#define MOCK_FUNCTION               (0)


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


/** internal test **/
extern void subdev_hash_iterator(void);
extern int _subdev_hash_remove(uint32_t devid);


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
    memcpy(rsp, "{\"test\": 12344}", len);

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

#if 0
void *example_thread1(void *args)
{
    int res;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    while (user_example_ctx->thread_running) {
        HAL_SleepMs(200);
        res = IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)ALINK2_PROP_POST_DATA, strlen(ALINK2_PROP_POST_DATA));
        EXAMPLE_TRACE("post property, res = %d", res);

        _mock_property_post_rsp();
    }

    return NULL;
}

void *example_thread2(void *args)
{
    int res;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    while (user_example_ctx->thread_running) {
        HAL_SleepMs(200);
        res = IOT_Linkkit_TriggerEvent(0, "Error", strlen("Error"), ALINK2_EVENT_POST_DATA, strlen(ALINK2_EVENT_POST_DATA));
        EXAMPLE_TRACE("post event, res = %d", res);

        _mock_event_post_rsp();
    }

    return NULL; 
}
#endif


int main(int argc, char **argv)
{
    user_example_ctx_t *user_example_ctx;
    uint32_t cnt = 0;
    int res = FAIL_RETURN;
    #if 1
    static iotx_dev_meta_info_t dev_info = {
        .product_key = "a1cZIolx3YE",
        .product_secret = "EfFYTuX1GjMDvw6l",
        .device_name = "TestAlink2RowDevice001",
        .device_secret = "qoQjpFkhhZaUxxmaIofg2k4CmOi0jTdo"
    };

    #else
    static iotx_dev_meta_info_t dev_info = {
        .product_key = "a1Tp6krL0KH",
        .product_secret = "EfFYTuX1GjMDvw6l",
        .device_name = "TestAlink2Device001",
        .device_secret = "uX5I81Zws2g5eFbkV9SuzZYEfiXMHpno"
    };
    static iotx_dev_meta_info_t dev_info = {
        .product_key = "a1hKsL86jFv",
        .product_secret = "lkpH3KOFOJUqu5uXqCB3Q8htivToy2em",
        .device_name = "TestAlink2Gw001",
        .device_secret = "lkpH3KOFOJUqu5uXqCB3Q8htivToy2em"
    };
    static iotx_dev_meta_info_t dev_info = {
        .product_key = "a1EbseykAjo",
        .product_secret = "EfFYTuX1GjMDvw6l",
        .device_name = "gw_01",
        .device_secret = "DTDcrlJXEmVFFDR7eY5MyfsInniLXjre"
    };
    #endif

    printf("alink start\r\n");
    LITE_set_loglevel(LOG_DEBUG_LEVEL);

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
    EXAMPLE_TRACE("IOT_Linkkit_Connect, res = %d", res);


    /*** internel test start ***/
    #if MOCK_FUNCTION
    _mock_property_put_req();

    res = IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)ALINK2_PROP_POST_DATA, strlen(ALINK2_PROP_POST_DATA));
    EXAMPLE_TRACE("post property, res = %d", res);
    _mock_property_post_rsp();

    res = IOT_Linkkit_TriggerEvent(0, "Error", strlen("Error"), ALINK2_EVENT_POST_DATA, strlen(ALINK2_EVENT_POST_DATA));
    EXAMPLE_TRACE("post event, res = %d", res);
    _mock_event_post_rsp();

    _mock_service_invoke_req();

    res = IOT_Linkkit_Report(0, ITM_MSG_POST_RAW_DATA, (uint8_t *)"\x01\x02\x03\xFF", 4);
    EXAMPLE_TRACE("post raw, res = %d", res);
    _mock_raw_put_req();
    _mock_raw_post_rsp();

    res = IOT_Linkkit_Report(0, ITM_MSG_DEVICEINFO_UPDATE, (uint8_t *)ALINK2_DEVINFO_POST_DATA, strlen(ALINK2_DEVINFO_POST_DATA));
    EXAMPLE_TRACE("post devinfo, res = %d", res);

    {
        static iotx_dev_meta_info_t subdev_info = {
            .product_key = "a1Tp6krL0KH",
            .product_secret = "TestAlink2Sub001",
            .device_name = "TestAlink2Sub001",
            .device_secret = ""
        };

        uint32_t i;
        uint32_t devid[2];

        for (i=0; i<2; i++) {
            subdev_info.device_name[9]++;
            devid[i] = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_SLAVE, &subdev_info);
            EXAMPLE_TRACE("subdev open, id = %d", devid[i]);
            res = IOT_Linkkit_Connect(devid[i]);
            EXAMPLE_TRACE("subdev conn, res = %d", res);

            _mock_register_post_rsp();  /* check */
        }

        res = IOT_Linkkit_Report(0, ITM_MSG_LOGIN, (uint8_t *)devid, sizeof(devid));
        EXAMPLE_TRACE("subdev login, res = %d", res);

        _mock_login_post_rsp();  /* check */

        res = IOT_Linkkit_Report(0, ITM_MSG_LOGOUT, (uint8_t *)devid, sizeof(devid));
        EXAMPLE_TRACE("subdev login, res = %d", res);
    }

    _mock_subdev_property_put_req();
    /*** internel test end ***/

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

            _subdev_hash_remove(devid[i]);
        }

        subdev_hash_iterator();
    }


    res = HAL_ThreadCreate(&user_example_ctx->example_thread1, example_thread1, NULL, NULL, NULL);
    if (res < 0) {
        EXAMPLE_TRACE("HAL_ThreadCreate Failed\n");
        IOT_Linkkit_Close(user_example_ctx->master_devid);
        return -1;
    }
    #endif

    res = HAL_ThreadCreate(&user_example_ctx->example_thread_yield, example_yield_thread, NULL, NULL, NULL);
    if (res < 0) {
        EXAMPLE_TRACE("HAL_ThreadCreate Failed\n");
        IOT_Linkkit_Close(user_example_ctx->master_devid);
        return -1;
    }

    /*
    {
        static iotx_dev_meta_info_t subdev_info[2] = {
            {
                .product_key = "a1Tp6krL0KH",
                .product_secret = "TestAlink2Sub001",
                .device_name = "TestAlink2Sub001",
                .device_secret = ""
            },
            {
                .product_key = "a1Tp6krL0KH",
                .product_secret = "TestAlink2Sub001",
                .device_name = "TestAlink2Sub002",
                .device_secret = ""
            }
        };

        uint32_t i;
        uint32_t devid[2];

        for (i=0; i<2; i++) {
            devid[i] = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_SLAVE, &subdev_info[i]);
            EXAMPLE_TRACE("subdev open, id = %d", devid[i]);
            
            res = IOT_Linkkit_Connect(devid[i]);
            EXAMPLE_TRACE("subdev conn, res = %d", res);
            
        }
        {
        alink_subdev_id_list_t subdev_id_list;
        subdev_id_list.subdev_array = devid;
        subdev_id_list.subdev_num = 2; 
        res = alink_upstream_subdev_register_delete_req(&subdev_id_list);
        EXAMPLE_TRACE("subdev mass register, res = %d", res);
        }
    }
    */

    while (1) {
        uint8_t raw_data[] = "\x00\x00\x22\x33\x44\x12\x32\x01\x3f\xa0\x00\x00";

        IOT_Linkkit_Yield(2000);

        HAL_SleepMs(2000);

        IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)ALINK2_PROP_POST_DATA_TMP, strlen(ALINK2_PROP_POST_DATA_TMP));
        /*  
        IOT_Linkkit_TriggerEvent(0, "testEvent01", strlen("testEvent01"), ALINK2_EVENT_POST_DATA, strlen(ALINK2_EVENT_POST_DATA));
        
        res = IOT_Linkkit_Report(0, ITM_MSG_DEVICEINFO_UPDATE, (uint8_t *)ALINK2_DEVINFO_POST_DATA, strlen(ALINK2_DEVINFO_POST_DATA));
        EXAMPLE_TRACE("post devinfo, res = %d", res);

        */
        res = IOT_Linkkit_Report(0, ITM_MSG_POST_RAW_DATA, raw_data, sizeof(raw_data) - 1);

        EXAMPLE_TRACE("post raw, res = %d", res);
        if (++cnt > 30) {
            user_example_ctx->thread_running = 0;
            HAL_SleepMs(1000);

            HAL_ThreadDelete(user_example_ctx->example_thread_yield);
            #if 0
            HAL_ThreadDelete(user_example_ctx->example_thread1);
            #endif
            IOT_Linkkit_Close(IOTX_LINKKIT_DEV_TYPE_MASTER);
            break;
        }
    }

    printf("alink stop\r\n");
}



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
#define ALINK2_EVENT_POST_DATA      "{\"ErrorCode\": 400}"
#define ALINK2_DEVINFO_POST_DATA    "[{\"attrKey\":\"devinfo_k\",\"attrValue\":\"devinfo_v\"}]"


/** type define **/
typedef struct {
    int master_devid;
    int cloud_connected;
    int master_initialized;
} user_example_ctx_t;


/** local variables define */
static user_example_ctx_t g_user_example_ctx;


/** internal test **/
extern void alink_downstream_mock(const char *uri_string, const char *payload);
extern void subdev_hash_iterator(void);
extern int _subdev_hash_remove(uint32_t devid);

/** mock **/
static void _mock_property_put_req(void);
static void _mock_property_post_rsp(void);
static void _mock_event_post_rsp(void);
static void _mock_service_invoke_req(void);
static void _mock_raw_put_req(void);
static void _mock_raw_post_rsp(void);
static void _mock_register_post_rsp(void);
static void _mock_login_post_rsp(void);

static void _mock_subdev_property_put_req(void);



static user_example_ctx_t *user_example_get_ctx(void)
{
    return &g_user_example_ctx;
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
    int res = FAIL_RETURN;
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
        .device_secret = ""
    };

    printf("alink start\r\n");
    LITE_set_loglevel(LOG_DEBUG_LEVEL);

    user_example_ctx = user_example_get_ctx();
    memset(user_example_ctx, 0, sizeof(user_example_ctx_t));

    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_GET, user_property_get_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, user_service_request_event_handler);
    IOT_RegisterCallback(ITE_RAWDATA_ARRIVED, user_down_raw_data_arrived_event_handler);    
    IOT_RegisterCallback(ITE_PERMIT_JOIN, user_permit_join_event_handler);
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


    #if 0
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
    #endif


    while (1) {
        IOT_Linkkit_Yield(200);

        HAL_SleepMs(2000);

        IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)ALINK2_PROP_POST_DATA, strlen(ALINK2_PROP_POST_DATA));
        IOT_Linkkit_TriggerEvent(0, "Error", strlen("Error"), ALINK2_EVENT_POST_DATA, strlen(ALINK2_EVENT_POST_DATA));

        if (++cnt > 20) {
            IOT_Linkkit_Close(IOTX_LINKKIT_DEV_TYPE_MASTER);
            break;
        }
    }

    printf("alink stop\r\n");
}


static void _mock_property_put_req(void)
{
    char *topic = "/a1OFrRjV8nz/develop_01/req/sys/thing/property/put/?i=1234567&a=y";
    char *payload = "{\"p\":{\"test\": 123}}";

    alink_downstream_mock(topic, payload);
}

static void _mock_subdev_property_put_req(void)
{
    char *topic = "/a1OFrRjV8nz/develop_01/req/ext/proxy/a1OFrRjV8nz/develop_03/sys/thing/property/put/?i=1000&a=y";
    char *payload = "{\"p\":{\"test\": 123}}";

    alink_downstream_mock(topic, payload);    
}

static void _mock_property_post_rsp(void)
{
    char *topic = "/a1OFrRjV8nz/develop_01/rsp/sys/thing/property/post/?i=1&r=200";
    char *payload = "{}";

    alink_downstream_mock(topic, payload);  
}

static void _mock_event_post_rsp(void)
{
    char *topic = "/a1OFrRjV8nz/develop_01/rsp/sys/thing/event/post/?i=2&r=200";
    char *payload = "{}";

    alink_downstream_mock(topic, payload);  
}

static void _mock_service_invoke_req(void)
{
    char *topic = "/a1OFrRjV8nz/develop_01/req/sys/thing/service/put/?i=987654321&a=y";
    char *payload = "{\"id\":\"ACControl\",\"params\":{\"Action\":\"On\",\"FandSpeed\":123}}";

    alink_downstream_mock(topic, payload);
}

static void _mock_raw_put_req(void)
{
    char *topic = "/a1OFrRjV8nz/develop_01/req/sys/thing/raw/put/?i=123456789&f=b"; 
    char *payload = "\x01\x02\x03\x04\xFF\x04\xFF";

    alink_downstream_mock(topic, payload);
}

static void _mock_raw_post_rsp(void)
{
    char *topic = "/a1OFrRjV8nz/develop_01/req/sys/thing/raw/put/?i=3&f=b&r=200"; 
    char *payload = "\xFF\x02\x03\x04\xFF\x04\xFF";

    alink_downstream_mock(topic, payload);
}

static void _mock_register_post_rsp(void)
{
    char *topic = "/a1OFrRjV8nz/develop_01/rsp/sys/sub/register/post/?i=4&r=200"; 
    char *payload = "{\"subList\":[{\"pk\":\"a1OFrRjV8nz\",\"dn\":\"develop_03\",\"ds\":\"ds123\"},{\"pk\":\"a1OFrRjV8nz\",\"dn\":\"develop_04\",\"ds\":\"ds456\"}]}";

    alink_downstream_mock(topic, payload);
}

static void _mock_login_post_rsp(void)
{
    char *topic = "/a1OFrRjV8nz/develop_01/rsp/sys/sub/login/post/?i=5&r=200";
    char *payload = "{\"subList\":[{\"code\":200,\"data\":{\"pk\":\"a1OFrRjV8nz\",\"dn\":\"develop_03\"}},{\"code\":200,\"data\":{\"pk\":\"a1OFrRjV8nz\",\"dn\":\"develop_04\"}}]}";

    alink_downstream_mock(topic, payload);
}

#if 0
static void _mock_login_delete_rsp(void)
{
    char *topic = "/a1OFrRjV8nz/develop_01/rsp/sys/sub/login/delete/?i=6&r=200";
    char *payload = "{\"subList\":[{\"code\":200,\"data\":{\"pk\":\"a1OFrRjV8nz\",\"dn\":\"develop_03\"}},{\"code\":200,\"data\":{\"pk\":\"a1OFrRjV8nz\",\"dn\":\"develop_04\"}}]}";

    alink_downstream_mock(topic, payload);
}


static void _mock_register_delete_rsp(void)
{
    char *topic = "/a1OFrRjV8nz/develop_01/rsp/sys/sub/register/post/?i=4&r=200"; 
    char *payload = "{\"subList\":[{\"pk\":\"a1OFrRjV8nz\",\"dn\":\"develop_03\",\"ds\":\"ds123\"},{\"pk\":\"a1OFrRjV8nz\",\"dn\":\"develop_04\",\"ds\":\"ds456\"}]}";

    alink_downstream_mock(topic, payload);
}
#endif



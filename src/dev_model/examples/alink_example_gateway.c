/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

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

#define EXAMPLE_MASTER_DEVID            (0)
#define EXAMPLE_SUBDEV_OPERATOIN_NUM    (3)
#define EXAMPLE_RUNNING_SECONE_MAX      (60)

#define SUBDEV_PROP_POST_DATA           "{\"WaterConsumption\": 123.456}"

/* device metainfo of this demo
char _product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a11YTWo4pMl";
char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "h4I4dneEFp7EImTv";
char _device_name[IOTX_DEVICE_NAME_LEN + 1]       = "gw_01";
char _device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "bA3QQj5bfagHy4VpsC0HM3I82XO5w5Jc";
*/

/** type define **/
typedef struct {
    int subdev_devid;
    int thread_running;
    void *example_thread_yield;
} user_example_ctx_t;

/** local variables define **/
static user_example_ctx_t g_user_example_ctx;


/** get user context **/
static user_example_ctx_t *user_example_get_ctx(void)
{
    return &g_user_example_ctx;
}

/** device connected event **/
static int user_connected_event_handler(uint32_t devid)
{
    EXAMPLE_TRACE("device %d connected", devid);
    return 0;
}

/** recv property set request from cloud **/
static int user_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    int res = 0;
    EXAMPLE_TRACE("Property Set Received, Devid: %d, Request: %s", devid, request);

    /* echo the same property to cloud */
    res = IOT_Linkkit_Report(devid, ITM_MSG_POST_PROPERTY, (unsigned char *)request, request_len);
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
    uint32_t len = strlen("{\"test\": 12344}");
    char *rsp = HAL_Malloc(len);

    EXAMPLE_TRACE("Service Request Received, Devid: %d, Service ID: %.*s, Payload: %s", devid, serviceid_len,
                  serviceid,
                  request);

    /* malloc response data */
    memset(rsp, 0, len);
    memcpy(rsp, "{\"Mode\": 1}", len);

    *response = rsp;
    *response_len = len;

    return 0;
}

/** recv raw data from cloud **/
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

void *example_yield_thread(void *args)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    while (user_example_ctx->thread_running) {
        IOT_Linkkit_Yield(200);
    }

    return NULL;
}

/* mass subdev add and delete demo */
int example_mass_subdev_operation_demo()
{
    int res = FAIL_RETURN;
    uint8_t i;
    int subdev_id[EXAMPLE_SUBDEV_OPERATOIN_NUM];
    static iotx_linkkit_dev_meta_info_t subdev_info[EXAMPLE_SUBDEV_OPERATOIN_NUM] = {
        {
            "a1c5IFlLVP2",
            "",
            "sub_04",
            ""     /* dynamic register used, so deviceSecret is useless */
        },
        {
            "a1c5IFlLVP2",
            "",
            "sub_03",
            ""     /* dynamic register used, so deviceSecret is useless */
        },
        {
            "a1c5IFlLVP2",
            "",
            "sub_02",
            "ssi7q0iI0y7Re8YM5SY00VskHiLiBGpH"     /* dynamic register not used */
        }
    };

    /* connect all subdev first */
    for (i=0; i<EXAMPLE_SUBDEV_OPERATOIN_NUM; i++) {
        subdev_id[i] = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_SLAVE, &subdev_info[i]);
        EXAMPLE_TRACE("subdev open, devid = %d", subdev_id[i]);
        res = IOT_Linkkit_Connect(subdev_id[i]);
        if (res < SUCCESS_RETURN) {
            EXAMPLE_TRACE("subdev connect failed, res = %d", res);
            return -1;
        }
        EXAMPLE_TRACE("subdev connect succeed");
    }

    /* mass subdev login, this is a sync api */
    res = IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_LOGIN, (uint8_t *)subdev_id, sizeof(uint32_t) * EXAMPLE_SUBDEV_OPERATOIN_NUM);
    EXAMPLE_TRACE("subdev login, res = %d", res);

    /* subdev property post */
    res = IOT_Linkkit_Report(subdev_id[0], ITM_MSG_POST_PROPERTY, (uint8_t *)SUBDEV_PROP_POST_DATA, sizeof(SUBDEV_PROP_POST_DATA));
    EXAMPLE_TRACE("post property, res = %d", res);

    HAL_SleepMs(2000);

    /* mass subdev logout, this is a sync api */
    res = IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_LOGOUT, (uint8_t *)subdev_id, sizeof(uint32_t) * EXAMPLE_SUBDEV_OPERATOIN_NUM);
    EXAMPLE_TRACE("subdev logout, res = %d", res);

    /* invoke IOT_Linkkit_Close to release subdev resource if you won't used it any more */

    return 0;
}

int main(int argc, char **argv)
{
    int res = FAIL_RETURN;
    user_example_ctx_t *user_example_ctx;
    static iotx_linkkit_dev_meta_info_t dev_info;
    uint32_t running_cnt = 0;

    /* get triple metadata from HAL */
    HAL_GetProductKey(dev_info.product_key);
    HAL_GetDeviceName(dev_info.device_name);
    HAL_GetDeviceSecret(dev_info.device_secret);

    user_example_ctx = user_example_get_ctx();
    memset(user_example_ctx, 0, sizeof(user_example_ctx_t));
    user_example_ctx->thread_running = 1;

    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, user_service_request_event_handler);
    IOT_RegisterCallback(ITE_RAWDATA_ARRIVED, user_down_raw_data_arrived_event_handler);

    printf("alink start\r\n");

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

    /* create thread for yield first */
    res = HAL_ThreadCreate(&user_example_ctx->example_thread_yield, example_yield_thread, NULL, NULL, NULL);
    if (res < 0) {
        EXAMPLE_TRACE("HAL_ThreadCreate Failed\n");
        IOT_Linkkit_Close(EXAMPLE_MASTER_DEVID);
        return -1;
    }

    /* single subdevice add operation demo */
    {
        iotx_linkkit_dev_meta_info_t subdev_info = {
            "a1c5IFlLVP2",
            "",
            "sub_01",
            "pWxj7bpeKO8EaYrFcIXmE9zL3U2LgmsO"
        };

        /* open subdev */
        user_example_ctx->subdev_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_SLAVE, &subdev_info);
        EXAMPLE_TRACE("subdev opened, devid = %d", user_example_ctx->subdev_devid);

        /* connect subdev */
        res = IOT_Linkkit_Connect(user_example_ctx->subdev_devid);
        if (res < SUCCESS_RETURN) {
            EXAMPLE_TRACE("subdev connect failed, res = %d", res);
        }
        EXAMPLE_TRACE("subdev connect succeed");

        /* login single subdev, set parameter payload to NULL */
        res = IOT_Linkkit_Report(user_example_ctx->subdev_devid, ITM_MSG_LOGIN, NULL, 0);
        EXAMPLE_TRACE("subdev login, res = %d", res);
    }

    while (1) {

        /* just report propety, get more device model operation demo in alink_example_solo.c */
        res = IOT_Linkkit_Report(user_example_ctx->subdev_devid, ITM_MSG_POST_PROPERTY, (uint8_t *)SUBDEV_PROP_POST_DATA, strlen(SUBDEV_PROP_POST_DATA));
        EXAMPLE_TRACE("post property, res = %d", res);

        /* runing mass subdev operation demo after about 20 seconds later */
        if (running_cnt == 10) {
            example_mass_subdev_operation_demo();
        }

        HAL_SleepMs(2000);
        if (++running_cnt > EXAMPLE_RUNNING_SECONE_MAX) {
            break;
        }
    }

    /* logout single subdev */
    res = IOT_Linkkit_Report(user_example_ctx->subdev_devid, ITM_MSG_LOGOUT, NULL, 0);
    EXAMPLE_TRACE("subdev logout, res = %d", res);

    /* close subdev */
    IOT_Linkkit_Close(user_example_ctx->subdev_devid);

    user_example_ctx->thread_running = 0;
    HAL_SleepMs(1000);
    HAL_ThreadDelete(user_example_ctx->example_thread_yield);

    /* deinit the linkkit sdk */
    IOT_Linkkit_Close(IOTX_LINKKIT_DEV_TYPE_MASTER);
    printf("alink stop\r\n");
    return 0;
}


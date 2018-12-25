#include <stdio.h>
#include <string.h>

#include "infra_types.h"
#include "infra_defs.h"
#include "alink_api.h"
#include "infra_log.h"

#include "alink_wrapper.h"

extern char _product_key[IOTX_PRODUCT_KEY_LEN + 1];
extern char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1];
extern char _device_name[IOTX_DEVICE_NAME_LEN + 1];
extern char _device_secret[IOTX_DEVICE_SECRET_LEN + 1];

#define PROP_ALINK1_TEST   "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"test1\":1234},\"method\":\"thing.event.property.post\"}"

extern int alink_downstream_invoke_mock(const char *uri_string);


int main(int argc, char **argv)
{
    printf("alink start\r\n");


    LITE_set_loglevel(5);

    static iotx_dev_meta_info_t dev_info = {
        .product_key = "a1OFrRjV8nz",
        .product_secret = "EfFYTuX1GjMDvw6l",
        .device_name = "develop_01",
        .device_secret = "7dqP7Sg1C2mKjajtFCQjyrh9ziR3wOMC"
    };

    memset(_product_key,0,IOTX_PRODUCT_KEY_LEN + 1);
    memcpy(_product_key,dev_info.product_key,strlen(dev_info.product_key));

    memset(_product_secret,0,IOTX_PRODUCT_SECRET_LEN + 1);
    memcpy(_product_secret,dev_info.product_secret,strlen(dev_info.product_secret));

    memset(_device_name,0,IOTX_DEVICE_NAME_LEN + 1);
    memcpy(_device_name,dev_info.device_name,strlen(dev_info.device_name));

    memset(_device_secret,0,IOTX_DEVICE_SECRET_LEN + 1);
    memcpy(_device_secret,dev_info.device_secret,strlen(dev_info.device_secret));

    IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &dev_info);

    IOT_Linkkit_Connect(0);


    //IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)"{\"switch\": 1}", strlen("{\"switch\": 1}"));

    IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)PROP_ALINK1_TEST, strlen(PROP_ALINK1_TEST));


    while (1) {
        IOT_Linkkit_Yield(200);

        HAL_SleepMs(2000);

        /* test */
        alink_downstream_invoke_mock("rsp/sys/dt/property/post");
        alink_downstream_invoke_mock("req/sys/thing/property/post");
        alink_downstream_invoke_mock("req/sys/thing/property/get");
        alink_downstream_invoke_mock("rsp/sys/dt/event/post");
        alink_downstream_invoke_mock("req/sys/thing/service/post");
        alink_downstream_invoke_mock("rsp/sys/dt/raw/post");
        alink_downstream_invoke_mock("req/sys/thing/raw/post");
        alink_downstream_invoke_mock("rsp/sys/subdev/register/post");
        alink_downstream_invoke_mock("rsp/sys/subdev/register/delete");
        alink_downstream_invoke_mock("rsp/sys/dt/topo/post");
        alink_downstream_invoke_mock("rsp/sys/dt/topo/delete");
        alink_downstream_invoke_mock("rsp/sys/dt/topo/get");
        alink_downstream_invoke_mock("req/sys/subdev/topo/post");
        alink_downstream_invoke_mock("rsp/sys/subdev/login/post");
        alink_downstream_invoke_mock("rsp/sys/subdev/logout/post");
        alink_downstream_invoke_mock("rsp/sys/dt/list/post");
        alink_downstream_invoke_mock("req/sys/subdev/permit/post");
        alink_downstream_invoke_mock("req/sys/subdev/config/post");
        alink_downstream_invoke_mock("rsp/sys/dt/deviceinfo/post");
        alink_downstream_invoke_mock("rsp/sys/dt/deviceinfo/get");
        alink_downstream_invoke_mock("rsp/sys/dt/deviceinfo/delete");

        IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)PROP_ALINK1_TEST, strlen(PROP_ALINK1_TEST));
    }


    printf("alink stop\r\n");
}
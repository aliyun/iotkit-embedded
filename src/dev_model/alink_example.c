#include <stdio.h>
#include <string.h>

#include "infra_types.h"
#include "infra_defs.h"
#include "alink_api.h"
#include "infra_log.h"

#include "alink_wrapper.h"


#define PROP_ALINK1_TEST   "{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"test1\":1234},\"method\":\"thing.event.property.post\"}"


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

    HAL_SetProductKey(dev_info.product_key);
    HAL_SetProductSecret(dev_info.product_secret);
    HAL_SetDeviceName(dev_info.device_name);
    HAL_SetDeviceSecret(dev_info.device_secret);

    IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &dev_info);

    IOT_Linkkit_Connect(0);


    //IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)"{\"switch\": 1}", strlen("{\"switch\": 1}"));

    IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)PROP_ALINK1_TEST, strlen(PROP_ALINK1_TEST));


    while (1) {
        IOT_Linkkit_Yield(200);

        HAL_SleepMs(2000);

        IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)PROP_ALINK1_TEST, strlen(PROP_ALINK1_TEST));
    }


    printf("alink stop\r\n");
}
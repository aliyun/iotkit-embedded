#include <stdio.h>
#include <string.h>

#include "infra_types.h"
#include "infra_defs.h"
#include "alink_api.h"
#include "infra_log.h"


int main(int argc, char **argv)
{
    printf("alink start\r\n");


    LITE_set_loglevel(5);

    iotx_linkkit_dev_meta_info_t dev_info = {
        .product_key = "a1OFrRjV8nz",
        .product_secret = "EfFYTuX1GjMDvw6l",
        .device_name = "develop_01",
        .device_secret = "7dqP7Sg1C2mKjajtFCQjyrh9ziR3wOMC"
    };

    IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &dev_info);

    IOT_Linkkit_Connect(0);


    IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)"{\"switch\": 1}", strlen("{\"switch\": 1}"));



    printf("alink stop\r\n");
}
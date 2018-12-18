#include <stdio.h>
#include <string.h>

#include "infra_types.h"
#include "infra_defs.h"
#include "alink_api.h"


int main(int argc, char **argv)
{
    printf("alink start\r\n");

    iotx_linkkit_dev_meta_info_t dev_info = {
        .product_key = "124324",
        .product_secret = "124324",
        .device_name = "124324",
        .device_secret = "124324"
    };

    IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &dev_info);

    IOT_Linkkit_Connect(0);


    IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, (uint8_t *)"{\"switch\": 1}", strlen("{\"switch\": 1}"));



    printf("alink stop\r\n");
}
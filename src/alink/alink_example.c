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


    printf("alink stop\r\n");
}
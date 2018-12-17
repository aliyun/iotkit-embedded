#include <stdio.h>
#include <string.h>
#include "infra_types.h"
#include "infra_defs.h"
#include "dynreg_api.h"

#define EXAMPLE_PRODUCT_KEY     "a1ZETBPbycq"
#define EXAMPLE_PRODUCT_SECRET  "L68wCVXYUaNg1Ey9"
#define EXAMPLE_DEVICE_NAME     "example1"

void HAL_Printf(const char *fmt, ...);

int main(int argc, char *argv[])
{
    int32_t res = 0;
    iotx_dev_meta_info_t meta;
    iotx_http_region_types_t region = IOTX_HTTP_REGION_SHANGHAI;
    HAL_Printf("dynreg example\r\n");

    memset(&meta,0,sizeof(iotx_dev_meta_info_t));
    memcpy(meta.product_key,EXAMPLE_PRODUCT_KEY,strlen(EXAMPLE_PRODUCT_KEY));
    memcpy(meta.product_secret,EXAMPLE_PRODUCT_SECRET,strlen(EXAMPLE_PRODUCT_SECRET));
    memcpy(meta.device_name,EXAMPLE_DEVICE_NAME,strlen(EXAMPLE_DEVICE_NAME));

    res = IOT_Dynamic_Register(region, &meta);
    if (res < 0) {
        HAL_Printf("IOT_Dynamic_Register failed\r\n");
        return -1;
    }

    HAL_Printf("Device Secret: %s\r\n",meta.device_secret);

    return 0;
}
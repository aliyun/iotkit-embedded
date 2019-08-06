#include <stdio.h>
#include <string.h>
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "dynreg_api.h"
#include "wrappers.h"

char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1ZETBPbycq";
char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "L68wCVXYUaNg1Ey9";
char g_device_name[IOTX_DEVICE_NAME_LEN + 1]       = "example1";
char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "";

int main(int argc, char *argv[])
{
    int32_t res = 0;
    iotx_dev_meta_info_t meta;
    iotx_http_region_types_t region = IOTX_HTTP_REGION_SHANGHAI;
    
    HAL_Printf("dynreg example\n");

    memset(&meta, 0, sizeof(iotx_dev_meta_info_t));
    memcpy(meta.product_key, g_product_key, strlen(g_product_key));
    memcpy(meta.product_secret, g_product_secret, strlen(g_product_secret));
    memcpy(meta.device_name, g_device_name, strlen(g_device_name));

    res = IOT_Dynamic_Register(region, &meta);
    if (res < 0) {
        HAL_Printf("IOT_Dynamic_Register failed\n");
        return -1;
    }

    HAL_Printf("\nDevice Secret: %s\n\n", meta.device_secret);

    return 0;
}

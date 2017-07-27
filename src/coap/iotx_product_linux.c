#include <stdio.h>
#include <stdint.h>

#include "iot_export.h"

#define IOTX_PRODUCT_KEY         "trTceekBd1P"
#define IOTX_DEVICE_NAME         "KAW7ihRrroLevlHN1y21"
#define IOTX_DEVICE_SECRET       "0yscIv4r7cIc3aDu6kKGvyVVEWvobQF6"
#define IOTX_DEVICE_ID           "trTceekBd1P.KAW7ihRrroLevlHN1y21"


int HAL_GetDeviceInfo(iotx_deviceinfo_t *p_devinfo)
{
    if(NULL == p_devinfo){
        return IOTX_ERR_INVALID_PARAM;
    }

    memset(p_devinfo, 0x00, sizeof(iotx_deviceinfo_t));
    strncpy(p_devinfo->device_id,    IOTX_DEVICE_ID,   IOTX_DEVICE_ID_LEN);
    strncpy(p_devinfo->product_key,  IOTX_PRODUCT_KEY, IOTX_PRODUCT_KEY_LEN);
    strncpy(p_devinfo->device_secret,IOTX_DEVICE_SECRET, IOTX_DEVICE_SECRET_LEN);
    strncpy(p_devinfo->device_name,  IOTX_DEVICE_NAME, IOTX_DEVICE_NAME_LEN);

    return IOTX_SUCCESS;
}


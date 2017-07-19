
#include <string.h>

#include "lite/lite-log.h"
#include "utils_error.h"
#include "aliot_device.h"


static aliot_user_info_t aliot_user_info;
static aliot_device_info_t aliot_device_info;


int aliot_device_init(void)
{
    memset(&aliot_device_info, 0x0, sizeof(aliot_device_info_t));
    memset(&aliot_user_info, 0x0, sizeof(aliot_user_info_t));

    log_info("device init success!");
    return SUCCESS_RETURN;
}


int32_t aliot_set_device_info(
            const char *product_key,
            const char *device_name,
            const char *device_secret)
{
    int ret;
    log_debug("start to set device info!");
    memset(&aliot_device_info, 0x0, sizeof(aliot_device_info));

    strncpy(aliot_device_info.product_key, product_key, PRODUCT_KEY_LEN);
    strncpy(aliot_device_info.device_name, device_name, DEVICE_NAME_LEN);
    strncpy(aliot_device_info.device_secret, device_secret, DEVICE_SECRET_LEN);

    //construct device-id(@product_key+@device_name)
    ret = snprintf(aliot_device_info.device_id, DEVICE_ID_LEN, "%s.%s", product_key, device_name);
    if ((ret < 0) || (ret >= DEVICE_ID_LEN)) {
        log_err("set device info failed");
        return FAIL_RETURN;
    }

    log_debug("set device info successfully!");
    return SUCCESS_RETURN;
}

aliot_device_info_pt aliot_get_device_info(void)
{
    return &aliot_device_info;
}


aliot_user_info_pt aliot_get_user_info(void)
{
    return &aliot_user_info;
}



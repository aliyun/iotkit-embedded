
#include <string.h>

#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_device.h"



static aliot_user_info_t g_userInfo;
static aliot_device_info_t g_deviceInfo;



int aliyun_iot_device_init(void)
{
    aliyun_iot_common_log_set_level(ALIOT_LOG_LEVEL_DEBUG);

    memset(&g_deviceInfo, 0x0, sizeof(aliot_device_info_t));
    memset(&g_userInfo, 0x0, sizeof(aliot_user_info_t));

    ALIOT_LOG_INFO("device init success!");
    return SUCCESS_RETURN;
}



/***********************************************************
* 函数名称: aliyun_iot_set_device_info
* 描       述: 设置设备信息
* 输入参数: IOT_DEVICEINFO_SHADOW_S*deviceInfo
* 输出参数: VOID
* 返 回  值: 0：成功  -1：失败
* 说       明: 将在aliyun注册的设备信息设置到SDK中的设备变量中
************************************************************/
int32_t aliyun_iot_set_device_info(
            char *product_key,
            char *device_name,
            char *device_id,
            char *device_secret)
{
    ALIOT_LOG_DEBUG("start to set device info!");
    memset(&g_deviceInfo, 0x0, sizeof(g_deviceInfo));

    strncpy(g_deviceInfo.product_key, product_key, PRODUCT_KEY_LEN);
    strncpy(g_deviceInfo.device_name, device_name, DEVICE_NAME_LEN);
    strncpy(g_deviceInfo.device_id, device_id, DEVICE_ID_LEN);
    strncpy(g_deviceInfo.device_secret, device_secret, DEVICE_SECRET_LEN);

    ALIOT_LOG_DEBUG("set device info success!");

    return SUCCESS_RETURN;
}

aliot_device_info_pt aliyun_iot_get_device_info(void)
{
    return &g_deviceInfo;
}


aliot_user_info_pt aliyun_iot_get_user_info(void)
{
    return &g_userInfo;
}



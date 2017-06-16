
#ifndef ALIYUN_IOT_AUTH_H
#define ALIYUN_IOT_AUTH_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "aliot_platform.h"
#include "aliot_error.h"
#include "aliot_device.h"

#define ALIOT_AUTH_IOT_ID       (USER_NAME_LEN)
#define ALIOT_AUTH_IOT_TOKEN    (PASSWORD_LEN)



/***********************************************************
* 函数名称: aliot_auth
* 描       述: sdk用户鉴权函数
* 输入参数: SIGN_DATA_TYPE_E signDataType 签名类型
*          IOT_BOOL_VALUE_E haveFilesys 是否有文件系统
* 返 回  值: 0：成功  -1：失败
* 说       明: 鉴权得到公钥证书并生成用户信息
************************************************************/
int32_t aliot_auth(aliot_device_info_pt pdevice_info, aliot_user_info_pt puser_info);

#if defined(__cplusplus)
}
#endif


#endif

/*********************************************************************************
 * 文件名称: aliyun_iot_auth.h
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述: iot鉴权
 * 说       明: 此文件包含设备端IOTsdk的鉴权接口和数据相关内容
 * 历       史:
 **********************************************************************************/
#ifndef ALIYUN_IOT_AUTH_H
#define ALIYUN_IOT_AUTH_H

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_device.h"

#define ALIOT_AUTH_IOT_ID       (USER_NAME_LEN)
#define ALIOT_AUTH_IOT_TOKEN    (PASSWORD_LEN)

/***********************************************************
* 函数名称: aliyun_iot_auth_init
* 描       述: auth初始化函数
* 输入参数: VOID
* 输出参数: VOID
* 返 回  值: 0 成功，-1 失败
* 说       明: 初始化日志级别，设备信息，鉴权信息文件的保存路径
************************************************************/
int32_t aliyun_iot_auth_init();

/***********************************************************
* 函数名称: aliyun_iot_auth_release
* 描       述: auth释放函数
* 输入参数: VOID
* 输出参数: VOID
* 返 回  值: 0:成功 -1:失败
* 说      明: 释放authInfo内存
************************************************************/
int32_t aliyun_iot_auth_release();


/***********************************************************
* 函数名称: aliyun_iot_auth
* 描       述: sdk用户鉴权函数
* 输入参数: SIGN_DATA_TYPE_E signDataType 签名类型
*          IOT_BOOL_VALUE_E haveFilesys 是否有文件系统
* 返 回  值: 0：成功  -1：失败
* 说       明: 鉴权得到公钥证书并生成用户信息
************************************************************/
int32_t aliyun_iot_auth(aliot_device_info_pt pdevice_info, aliot_user_info_pt puser_info);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif


#endif

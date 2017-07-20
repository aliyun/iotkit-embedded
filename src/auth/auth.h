#ifndef _IOTX_AUTH_H_
#define _IOTX_AUTH_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "iot_import.h"
#include "device.h"     // for deviceInfo{} + userInfo{}

#define GUIDER_IOT_ID_LEN           (256)
#define GUIDER_IOT_TOKEN_LEN        (512)
#define GUIDER_DEFAULT_TS_STR       "2524608000000"

#define CONN_HOST_LEN               (128)
#define CONN_PORT_LEN               (8)
#define CONN_USR_LEN                (128)
#define CONN_PWD_LEN                (128)
#define CONN_CID_LEN                (256)
#define CONN_SECMODE_LEN            (32)

#define GUIDER_SIGN_LEN             (128)
#define GUIDER_PID_LEN              (64)
#define GUIDER_TS_LEN               (16)
#define GUIDER_URL_LEN              (256)

int32_t iotx_auth(iotx_device_info_pt pdevice_info, iotx_user_info_pt puser_info);

#if defined(__cplusplus)
}
#endif
#endif

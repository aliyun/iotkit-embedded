#ifndef _IOTX_AUTH_H_
#define _IOTX_AUTH_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "iot_import.h"
#include "device.h"     // for deviceInfo{} userInfo{}

#define GUIDER_IOT_ID_LEN       (256)
#define GUIDER_IOT_TOKEN_LEN    (512)

int32_t iotx_auth(iotx_device_info_pt pdevice_info, iotx_user_info_pt puser_info);


#if defined(__cplusplus)
}
#endif
#endif

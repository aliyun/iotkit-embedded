
#ifndef _IOTX_AUTH_H_
#define _IOTX_AUTH_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "iot_import.h"
#include "utils_error.h"
#include "device.h"

#define IOTX_AUTH_IOT_ID       (USER_NAME_LEN)
#define IOTX_AUTH_IOT_TOKEN    (PASSWORD_LEN)


int32_t iotx_auth(iotx_device_info_pt pdevice_info, iotx_user_info_pt puser_info);


#if defined(__cplusplus)
}
#endif


#endif

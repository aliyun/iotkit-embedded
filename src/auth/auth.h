
#ifndef _ALIOT_AUTH_H_
#define _ALIOT_AUTH_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "iot_import.h"
#include "utils_error.h"
#include "device.h"

#define ALIOT_AUTH_IOT_ID       (USER_NAME_LEN)
#define ALIOT_AUTH_IOT_TOKEN    (PASSWORD_LEN)


int32_t iotx_auth(iotx_device_info_pt pdevice_info, iotx_user_info_pt puser_info);


#if defined(__cplusplus)
}
#endif


#endif

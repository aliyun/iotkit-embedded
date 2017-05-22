
#ifndef _ALIOT_AUTH_H_
#define _ALIOT_AUTH_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "aliot_platform.h"
#include "aliot_error.h"
#include "aliot_device.h"

#define ALIOT_AUTH_IOT_ID       (USER_NAME_LEN)
#define ALIOT_AUTH_IOT_TOKEN    (PASSWORD_LEN)


int32_t aliot_auth(aliot_device_info_pt pdevice_info, aliot_user_info_pt puser_info);


#if defined(__cplusplus)
}
#endif


#endif

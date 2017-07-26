#ifndef _IOTX_AUTH_H_
#define _IOTX_AUTH_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "iot_import.h"
#include "device.h"     // for deviceInfo{} + userInfo{}

int32_t IOT_FetchConnInfo(iotx_device_info_pt pdevice_info, iotx_conn_info_pt puser_info);

#if defined(__cplusplus)
}
#endif
#endif

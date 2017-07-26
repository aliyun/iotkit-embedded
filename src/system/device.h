#ifndef _IOTX_DEVICE_H_
#define _IOTX_DEVICE_H_
#if defined(__cplusplus)
extern "C" {
#endif

#include "iot_import.h"

#define MQTT_SDK_VERSION  "2.0"

#define PRODUCT_KEY_LEN     (11)
#define DEVICE_NAME_LEN     (32)
#define DEVICE_ID_LEN       (64)
#define DEVICE_SECRET_LEN   (64)

#define MODULE_VENDOR_ID    (32) // Partner ID

#define HOST_ADDRESS_LEN    (128)
#define HOST_PORT_LEN       (8)
#define CLIENT_ID_LEN       (256)
#define USER_NAME_LEN       (128)
#define PASSWORD_LEN        (128)
#define AESKEY_STR_LEN      (32)

typedef struct {
    char        product_key[PRODUCT_KEY_LEN + 1];
    char        device_name[DEVICE_NAME_LEN + 1];
    char        device_id[DEVICE_ID_LEN + 1];
    char        device_secret[DEVICE_SECRET_LEN + 1];
    char        module_vendor_id[MODULE_VENDOR_ID + 1];
} iotx_device_info_t, *iotx_device_info_pt;

typedef struct {
    uint16_t        port;
    char            host_name[HOST_ADDRESS_LEN + 1];
    char            client_id[CLIENT_ID_LEN + 1];
    char            username[USER_NAME_LEN + 1];
    char            password[PASSWORD_LEN + 1];
    const char     *pub_key;
#ifdef ID2_AUTH
    char            aeskey_str[AESKEY_STR_LEN + 1];
#endif
} iotx_conn_info_t, *iotx_conn_info_pt;

int IOT_CreateDeviceInfo(void);

int32_t IOT_SetDeviceInfo(
            const char *product_key,
            const char *device_name,
            const char *device_secret);

iotx_device_info_pt IOT_GetDeviceInfo(void);

iotx_conn_info_pt IOT_GetConnInfo(void);

#if defined(__cplusplus)
}
#endif
#endif  /* #ifndef _IOTX_DEVICE_H_ */
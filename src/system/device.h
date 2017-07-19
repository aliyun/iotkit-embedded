
#ifndef _ALIOT_DEVICE_H_
#define _ALIOT_DEVICE_H_

#include "iot_import.h"

#define MQTT_SDK_VERSION  "1.0.0"

//SD, device system
#define PRODUCT_KEY_LEN     (11)
#define DEVICE_NAME_LEN     (32)
#define DEVICE_ID_LEN       (64)
#define DEVICE_SECRET_LEN   (64)

#define MODULE_VENDOR_ID    (32) //PID

#define HOST_ADDRESS_LEN    (128)
#define HOST_PORT_LEN       (8)
#define CLIENT_ID_LEN       (256)
#define USER_NAME_LEN       (128)
#define PASSWORD_LEN        (128)


typedef struct {
    char product_key[PRODUCT_KEY_LEN + 1];
    char device_name[DEVICE_NAME_LEN + 1];
    char device_id[DEVICE_ID_LEN + 1];
    char device_secret[DEVICE_SECRET_LEN + 1];
    char module_vendor_id[MODULE_VENDOR_ID + 1];
} aliot_device_info_t, *aliot_device_info_pt;


typedef struct {
    uint16_t port;
    char host_name[HOST_ADDRESS_LEN + 1];
    char client_id[CLIENT_ID_LEN + 1];
    char user_name[USER_NAME_LEN + 1];
    char password[PASSWORD_LEN + 1];
    const char *pubKey;
} aliot_user_info_t, *aliot_user_info_pt;


int aliot_device_init(void);

int32_t aliot_set_device_info(
            const char *product_key,
            const char *device_name,
            const char *device_secret);

aliot_device_info_pt aliot_get_device_info(void);

aliot_user_info_pt aliot_get_user_info(void);


#endif

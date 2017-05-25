
#ifndef _MQTT_ALIYUN_MQTT_DEVICE_H_
#define _MQTT_ALIYUN_MQTT_DEVICE_H_

#include "aliyun_iot_platform_datatype.h"

#define MQTT_SDK_VERSION  "1.0.0"

//SD, device system
#define PRODUCT_KEY_LEN     (11)
#define DEVICE_NAME_LEN     (32)
#define DEVICE_ID_LEN       (20)
#define DEVICE_SECRET_LEN   (32)

#define HOST_ADDRESS_LEN    (64)
#define HOST_PORT_LEN       (8)
#define CLIENT_ID_LEN       (128)
#define USER_NAME_LEN       (32)
#define PASSWORD_LEN        (32)


typedef struct IOT_DEVICE_INFO
{
    char product_key[PRODUCT_KEY_LEN + 1];
    char device_name[DEVICE_NAME_LEN + 1];
    char device_id[DEVICE_ID_LEN + 1];
    char device_secret[DEVICE_SECRET_LEN + 1];
}aliot_device_info_t, *aliot_device_info_pt;


typedef struct ALIYUN_IOT_USER_INFO
{
    uint16_t port;
    char port_str[6]; //todo remove it.
    char host_name[HOST_ADDRESS_LEN + 1];
    char client_id[CLIENT_ID_LEN + 1];
    char user_name[USER_NAME_LEN + 1];
    char password[PASSWORD_LEN + 1];
    const char *pubKey;
}aliot_user_info_t, *aliot_user_info_pt;


int aliyun_iot_device_init(void);

int32_t aliyun_iot_set_device_info(
            char *product_key,
            char *device_name,
            char *device_id,
            char *device_secret);

aliot_device_info_pt aliyun_iot_get_device_info(void);

aliot_user_info_pt aliyun_iot_get_user_info(void);


#endif /* _MQTT_ALIYUN_MQTT_DEVICE_H_ */

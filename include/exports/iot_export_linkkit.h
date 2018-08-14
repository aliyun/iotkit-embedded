
#include "iot_export.h"

typedef enum {
    IOTX_LINKKIT_DEV_TYPE_MASTER,
    IOTX_LINKKIT_DEV_TYPE_SLAVE
} iotx_linkkit_dev_type_t;

typedef struct {
    char product_key[PRODUCT_KEY_MAXLEN];
    char product_secret[PRODUCT_SECRET_MAXLEN];
    char device_name[DEVICE_NAME_MAXLEN];
    char device_secret[DEVICE_SECRET_MAXLEN];
} iotx_linkkit_dev_meta_info_t;

typedef enum {
    IOTX_LINKKIT_CMD_OPTION_POST_PROPERTY_REPLY,
    IOTX_LINKKIT_CMD_OPTION_POST_EVENT_REPLY
} iotx_linkkit_ioctl_cmd_t;

typedef struct {
    int (* connected)(void);
    int (* disconnected)(void);
    int (* down_raw)(const int devid, const unsigned char *payload, const int payload_len);
    int (* service_set)(const int devid, const char *service_id, const char *request, const int request_len,
                        char **response, int *response_len);
    int (* property_set)(const int devid, const char *payload, const int payload_len);
    int (* post_reply)(const int devid, const int msgid, const int code, const char *payload, const int payload_len);
    int (* permit_join)(void);
    int (* initialized)(const int devid);
} iotx_linkkit_event_handler_t;

typedef enum {
    /* post property value to cloud */
    IOTX_LINKKIT_MSG_POST_PROPERTY,

    /* post event output value to cloud , need fill event identifier in IOT_Linkkit_Post */
    IOTX_LINKKIT_MSG_POST_EVENT
} iotx_linkkit_msg_type_t;

/**
 * @brief create a new device
 *
 * @param dev_type. type of device which will be created. see iotx_dev_type_t
 * @param meta_info. The product key, product secret, device name and device secret of new device.
 *
 * @return success: device id (>=0), fail: -1.
 *
 */
int IOT_Linkkit_Open(iotx_linkkit_dev_type_t dev_type, iotx_linkkit_dev_meta_info_t meta_info);

/**
 * @brief configuration runtime parameter before network established.
 *
 * @param devid. device identifier.
 * @param cmd. command. see iotx_ioctl_cmd_t
 * @param arg. command argument.
 *
 * @return success: device id (>=0), fail: -1.
 *
 */
int IOT_Linkkit_Ioctl(int devid, iotx_linkkit_ioctl_cmd_t cmd, void *arg);

/**
 * @brief start device network connection.
 *        for master device, start to connect aliyun server.
 *        for slave device, send message to cloud for register new device and add topo with master device
 *
 * @param devid. device identifier.
 * @param iotx_linkkit_event_handler_t. event callback. see iotx_linkkit_event_handler_t
 *
 * @return success: device id (>=0), fail: -1.
 *
 */
int IOT_Linkkit_Start(int devid, iotx_linkkit_event_handler_t hdlrs);

/**
 * @brief try to receive message from cloud and dispatch these message to user event callback
 *
 * @param timeout_ms. timeout for waiting new message arrived
 *
 * @return void.
 *
 */
void IOT_Linkkit_Yield(int timeout_ms);

/**
 * @brief close device network connection and release resources.
 *        for master device, disconnect with aliyun server and release all local resources.
 *        for slave device, send message to cloud for delete topo with master device and unregister itself, then release device's resources.
 *
 * @param devid. device identifier.
 *
 * @return success: 0, fail: -1.
 *
 */
int IOT_Linkkit_Close(int devid);

/**
 * @brief slave device login to cloud. slave device can transfer message with cloud only when logined.
 *
 * @param devid. device identifier.
 *
 * @return success: 0, fail: -1.
 *
 */
int IOT_Linkkit_Login(int devid);

/**
 * @brief slave device logout from cloud
 *
 * @param devid. device identifier.
 *
 * @return success: 0, fail: -1.
 *
 */
int IOT_Linkkit_Logout(int devid);

/**
 * @brief post message to cloud
 *
 * @param devid. device identifier.
 * @param msg_type. message type. see iotx_linkkit_msg_type_t.
 * @param identifier. optional parameter accroding to iotx_linkkit_msg_type_t.
 * @param payload. message payload.
 * @param payload_len. message payload length.
 *
 * @return success: message id (>1), fail: -1.
 *
 */
int IOT_Linkkit_Post(int devid, int msg_type, const char *identifier, unsigned char *payload, int payload_len);
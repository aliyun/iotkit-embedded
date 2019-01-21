/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_API_H__
#define __ALINK_API_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "infra_types.h"
#include "infra_defs.h"

typedef enum {
    IOTX_LINKKIT_DEV_TYPE_MASTER,
    IOTX_LINKKIT_DEV_TYPE_SLAVE,
    IOTX_LINKKIT_DEV_TYPE_MAX
} iotx_linkkit_dev_type_t;

typedef struct _iotx_dev_meta_info iotx_linkkit_dev_meta_info_t;

typedef enum {
    /* post property value to cloud */
    ITM_MSG_POST_PROPERTY,

    /* post device info update message to cloud */
    ITM_MSG_DEVICEINFO_UPDATE,

    /* send a get device info request to cloud */
    ITM_MSG_DEVICEINFO_GET,

    /* post device info delete message to cloud */
    ITM_MSG_DEVICEINFO_DELETE,

    /* post raw data to cloud */
    ITM_MSG_POST_RAW_DATA,

    /* only for slave device, send login request to cloud */
    ITM_MSG_LOGIN,

    /* only for slave device, send logout request to cloud */
    ITM_MSG_LOGOUT,

    /* query ntp time from cloud */
    ITM_MSG_QUERY_TIMESTAMP,

    /* only for master device, query topo list */
    ITM_MSG_QUERY_TOPOLIST,

    /* only for master device, qurey firmware ota data */
    ITM_MSG_QUERY_FOTA_DATA,

    /* only for master device, qurey config ota data */
    ITM_MSG_QUERY_COTA_DATA,

    /* only for master device, request config ota data from cloud */
    ITM_MSG_REQUEST_COTA,

    /* only for master device, request fota image from cloud */
    ITM_MSG_REQUEST_FOTA_IMAGE,

    IOTX_LINKKIT_MSG_MAX
} iotx_linkkit_msg_type_t;

typedef enum {
    ITE_AWSS_STATUS = 0,        /* no implement this version */
    ITE_CONNECT_SUCC,
    ITE_CONNECT_FAIL,
    ITE_DISCONNECTED,
    ITE_RAWDATA_ARRIVED,
    ITE_SERVICE_REQUEST,
    ITE_PROPERTY_SET,
    ITE_PROPERTY_GET,           /* no implement this version */
    ITE_REPORT_REPLY,
    ITE_TRIGGER_EVENT_REPLY,    /* deprecated, use ITE_REPORT_REPLY event */
    ITE_TIMESTAMP_REPLY,        /* no implement this version */
    ITE_TOPOLIST_REPLY,         /* no implement this version */
    ITE_PERMIT_JOIN,            /* no implement this version */
    ITE_INITIALIZE_COMPLETED,
    ITE_FOTA,                   /* no implement this version */
    ITE_COTA,                   /* no implement this version */
    ITE_MQTT_CONNECT_SUCC,
    ITE_EVENT_NUM
} iotx_linkkit_event_type_t;

typedef int (*linkkit_event_cb_t)();

typedef int (*linkkit_awss_status_cb_t)(int);

typedef int (*linkkit_connect_success_cb_t)(uint32_t devid);        /* modified, add parameter devid */

typedef int (*linkkit_connect_fail_cb_t)(uint32_t devid);           /* modified, add parameter devid */

typedef int (*linkkit_disconnected_cb_t)(uint32_t devid);           /* modified, add parameter devid */

typedef int (*linkkit_rawdata_rx_cb_t)(int device_id, const uint8_t *payload, uint32_t payload_len);

typedef int (*linkkit_service_request_cb_t)(int device_id, const char *serviceid, uint32_t serviceid_len,
                                            const char *request, uint32_t request_len,
                                            char **response, uint32_t *response_len);

typedef int (*linkkit_property_set_cb_t)(int device_id, const char *request, uint32_t request_len);

typedef int (*linkkit_property_get_cb_t)(int device_id, const char *request, uint32_t request_len,
                                                        char **response, uint32_t *response_len);

typedef int (*linkkit_report_reply_cb_t)(int device_id, int msg_id, int code,
                                        const char *reply, uint32_t reply_len);

typedef int (*linkkit_timestamp_reply_cb_t)(const char *timestamp);

typedef int (*linkkit_topo_list_reply_cb_t)();

typedef int (*linkkit_permit_join_cb_t)(const char *productKey, const int timeout);

/* master device register to cloud successfuly, or subdevice login successfuly */
typedef int (*linkkit_inited_cb_t)(uint32_t devid);

typedef int (*linkkit_fata_event_cb_t)(int type, const char *version);

typedef int (*linkkit_cota_event_cb_t)(int type);

typedef int (*linkkit_mqtt_connected_cb_t)(void);


/**
 * @brief register event callback function for specific event id.
 *
 * @param event_id. type of event id. see iotx_linkkit_event_type_t.
 * @param callback. callback function for different event handle, check the function prototypes.
 *
 * @return success: device id (>=0), fail: -1.
 *
 */
int IOT_RegisterCallback(iotx_linkkit_event_type_t event_id, linkkit_event_cb_t callback);

/**
 * @brief create a new device
 *
 * @param dev_type. type of device which will be created. see iotx_linkkit_dev_type_t
 * @param meta_info. The product key, product secret, device name and device secret of new device.
 *
 * @return success: device id (>=0), fail: -1.
 *
 */
int IOT_Linkkit_Open(iotx_linkkit_dev_type_t dev_type, iotx_linkkit_dev_meta_info_t *meta_info);

/**
 * @brief start device network connection.
 *        for master device, start to connect aliyun server.
 *        for slave device, send message to cloud for register new device and add topo with master device
 *
 * @param devid. device identifier.
 *
 * @return success: device id (>=0), fail: -1.
 *
 */
int IOT_Linkkit_Connect(int devid);

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
 * @brief Report message to cloud
 *
 * @param devid. device identifier.
 * @param msg_type. message type. see iotx_linkkit_msg_type_t, as follows:
 *        ITM_MSG_POST_PROPERTY
 *        ITM_MSG_DEVICEINFO_UPDATE
 *        ITM_MSG_DEVICEINFO_DELETE
 *        ITM_MSG_POST_RAW_DATA
 *        ITM_MSG_LOGIN
 *        ITM_MSG_LOGOUT
 *
 * @param payload. message payload.
 * @param payload_len. message payload length.
 *
 * @return success: 0 or message id (>=1), fail: -1.
 *
 */
int IOT_Linkkit_Report(int devid, iotx_linkkit_msg_type_t msg_type, unsigned char *payload,
                                   uint32_t payload_len);

/**
 * @brief post message to cloud
 *
 * @param devid. device identifier.
 * @param msg_type. message type. see iotx_linkkit_msg_type_t, as follows:
 *        ITM_MSG_QUERY_TIMESTAMP
 *        ITM_MSG_QUERY_TOPOLIST
 *        ITM_MSG_QUERY_FOTA_DATA
 *        ITM_MSG_QUERY_COTA_DATA
 *        ITM_MSG_REQUEST_COTA
 *        ITM_MSG_REQUEST_FOTA_IMAGE
 *
 * @param payload. message payload.
 * @param payload_len. message payload length.
 *
 * @return success: 0 or message id (>=1), fail: -1.
 *
 */
int IOT_Linkkit_Query(int devid, iotx_linkkit_msg_type_t msg_type, unsigned char *payload, uint32_t payload_len);

/**
 * @brief post event to cloud
 *
 * @param devid. device identifier.
 * @param eventid. tsl event id.
 * @param eventid_len. length of tsl event id.
 * @param payload. event payload.
 * @param payload_len. event payload length.
 *
 * @return success: message id (>=1), fail: -1.
 *
 */
int IOT_Linkkit_TriggerEvent(int devid, char *eventid, uint32_t eventid_len, char *payload, uint32_t payload_len);


typedef enum {
    IOTX_CODE_PARAMS_INVALID        = -4001L,
    IOTX_CODE_STATUS_ERROR          = -4002L,
    IOTX_CODE_NETWORK_ERROR         = -4003L,
    IOTX_CODE_AUTH_ERROR            = -4004L,
    IOTX_CODE_GATEWAY_UNSUPPORTED   = -4005L,
    IOTX_CODE_UNKNOWN_MSG_TYPE      = -4006L,
    IOTX_CODE_MEMORY_NOT_ENOUGH     = -4007L,
    IOTX_CODE_ALREADY_OPENED        = -4008L,
    IOTX_CODE_TOO_MANY_SUBDEV       = -4009L,
    IOTX_CODE_MUTEX_CREATE_ERROR    = -4010L,
    IOTX_CODE_SUBDEV_NOT_EXIST      = -4011L,
    IOTX_CODE_CREATE_MUTEX_FAILED   = -4012L,
    IOTX_CODE_SUBDEV_IDX_OVERFLOW   = -4013L,
    IOTX_CODE_UNKNOWN_ERROR         = -4014L,   /* TOOD */
    IOTX_CODE_CLOUD_RETURN_ERROR    = -4015L,
    IOTX_CODE_WAIT_RSP_TIMEOUT      = -4016L,
    IOTX_CODE_LIST_INSERT_ERROR     = -4017L,
    IOTX_CODE_JSON_FORMAT_ERROR     = -4018L,
} iotx_linkkit_errorcode_t;



#if defined(__cplusplus)
}
#endif

#endif /* #ifndef __ALINK_API_H__ */




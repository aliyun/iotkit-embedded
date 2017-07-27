#ifndef __IOT_EXPORT_H__
#define __IOT_EXPORT_H__
#if defined(__cplusplus)
extern "C" {
#endif

typedef enum _IOT_LogLevel {
    IOT_LOG_EMERG = 0,
    IOT_LOG_CRIT,
    IOT_LOG_ERROR,
    IOT_LOG_WARNING,
    IOT_LOG_INFO,
    IOT_LOG_DEBUG,
} IOT_LogLevel;

void    IOT_OpenLog(const char *ident);
void    IOT_CloseLog(void);
void    IOT_SetLogLevel(IOT_LogLevel level);
void    IOT_DumpMemoryStats(IOT_LogLevel level);
int     IOT_SetupConnInfo(void);

/* From utils_error.h */
typedef enum IOT_RETURN_CODES {
    ERROR_DEVICE_NOT_EXSIT = -311,
    ERROR_NET_TIMEOUT = -310,
    ERROR_CERT_VERIFY_FAIL  = -309,
    ERROR_NET_SETOPT_TIMEOUT = -308,
    ERROR_NET_SOCKET = -307,
    ERROR_NET_CONNECT = -306,
    ERROR_NET_BIND = -305,
    ERROR_NET_LISTEN = -304,
    ERROR_NET_RECV = -303,
    ERROR_NET_SEND = -302,
    ERROR_NET_CONN = -301,
    ERROR_NET_UNKNOWN_HOST = -300,

    MQTT_SUB_INFO_NOT_FOUND_ERROR = -43,
    MQTT_PUSH_TO_LIST_ERROR = -42,
    MQTT_TOPIC_FORMAT_ERROR = -41,
    NETWORK_RECONNECT_TIMED_OUT_ERROR = -40,/** Returned when the Network is disconnected and the reconnect attempt has timed out */
    MQTT_CONNACK_UNKNOWN_ERROR = -39,/** Connect request failed with the server returning an unknown error */
    MQTT_CONANCK_UNACCEPTABLE_PROTOCOL_VERSION_ERROR = -38,/** Connect request failed with the server returning an unacceptable protocol version error */
    MQTT_CONNACK_IDENTIFIER_REJECTED_ERROR = -37,/** Connect request failed with the server returning an identifier rejected error */
    MQTT_CONNACK_SERVER_UNAVAILABLE_ERROR = -36,/** Connect request failed with the server returning an unavailable error */
    MQTT_CONNACK_BAD_USERDATA_ERROR = -35,/** Connect request failed with the server returning a bad userdata error */
    MQTT_CONNACK_NOT_AUTHORIZED_ERROR = -34,/** Connect request failed with the server failing to authenticate the request */
    MQTT_CONNECT_ERROR = -33,
    MQTT_CREATE_THREAD_ERROR = -32,
    MQTT_PING_PACKET_ERROR = -31,
    MQTT_CONNECT_PACKET_ERROR = -30,
    MQTT_CONNECT_ACK_PACKET_ERROR = -29,
    MQTT_NETWORK_CONNECT_ERROR = -28,
    MQTT_STATE_ERROR = -27,
    MQTT_SUBSCRIBE_PACKET_ERROR = -26,
    MQTT_SUBSCRIBE_ACK_PACKET_ERROR = -25,
    MQTT_SUBSCRIBE_ACK_FAILURE = -24,
    MQTT_SUBSCRIBE_QOS_ERROR = -23,
    MQTT_UNSUBSCRIBE_PACKET_ERROR = -22,
    MQTT_PUBLISH_PACKET_ERROR = -21,
    MQTT_PUBLISH_QOS_ERROR = -20,
    MQTT_PUBLISH_ACK_PACKET_ERROR = -19,
    MQTT_PUBLISH_COMP_PACKET_ERROR = -18,
    MQTT_PUBLISH_REC_PACKET_ERROR = -17,
    MQTT_PUBLISH_REL_PACKET_ERROR = -16,
    MQTT_UNSUBSCRIBE_ACK_PACKET_ERROR = -15,
    MQTT_NETWORK_ERROR = -14,
    MQTT_PUBLISH_ACK_TYPE_ERROR = -13,

    ERROR_SHADOW_NO_METHOD = -2008,
    ERROR_SHADOW_UNDEF_TYPE = -2007,
    ERROR_SHADOW_UPDATE_TIMEOUT = -2006,
    ERROR_SHADOW_UPDATE_NACK = -2005,         /**< Negative ACK  */
    ERROR_SHADOW_NO_ATTRIBUTE = -2004,
    ERROR_SHADOW_ATTR_NO_EXIST = -2003,       /**< NO such attribute */
    ERROR_SHADOW_ATTR_EXIST = -2002,          /**< attribute already exists   */
    ERROR_SHADOW_WAIT_LIST_OVERFLOW = -2001,
    ERROR_SHADOW_INVALID_STATE = -2000,

    ERROR_NO_MEM = -1016,
    ERROR_CERTIFICATE_EXPIRED = -1015,
    ERROR_MALLOC = -1014,
    ERROR_NO_ENOUGH_MEM = -1013,               /**< Writes more than size value. */

    ERROR_NO_SUPPORT = -12,
    ERROR_NO_PERSISTENCE = -11,
    ERROR_HTTP_BREAK = -10,
    ERROR_NULL_VALUE = -9,
    ERROR_HTTP_CONN = -8,                    /**< Connection failed. */
    ERROR_HTTP_PARSE = -7,                   /**< A URL parse error occurred. */
    ERROR_HTTP_UNRESOLVED_DNS = -6,           /**< Could not resolve the hostname. */
    ERROR_HTTP_PRTCL = -5,                   /**< A protocol error occurred. */
    ERROR_HTTP = -4,                         /**< An unknown error occurred. */
    ERROR_HTTP_CLOSED = -3,                  /**< Connection was closed by a remote host. */
    NULL_VALUE_ERROR = -2,

    FAIL_RETURN = -1,                        /**< generic error. */
    SUCCESS_RETURN = 0,


    /* @value > 0, reserved for other usage */

} iotx_err_t;
/* From utils_error.h */

/* From mqtt_client.h */
typedef enum {
    IOTX_MQTT_QOS0 = 0,
    IOTX_MQTT_QOS1,
    IOTX_MQTT_QOS2
} iotx_mqtt_qos_t;

typedef enum {

    /* Undefined event */
    IOTX_MQTT_EVENT_UNDEF = 0,

    /* MQTT disconnect event */
    IOTX_MQTT_EVENT_DISCONNECT = 1,

    /* MQTT reconnect event */
    IOTX_MQTT_EVENT_RECONNECT = 2,

    /* A ACK to the specific subscribe which specify by packet-id be received */
    IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS = 3,

    /* No ACK to the specific subscribe which specify by packet-id be received in timeout period */
    IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT = 4,

    /* A failed ACK to the specific subscribe which specify by packet-id be received*/
    IOTX_MQTT_EVENT_SUBCRIBE_NACK = 5,

    /* A ACK to the specific unsubscribe which specify by packet-id be received */
    IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS = 6,

    /* No ACK to the specific unsubscribe which specify by packet-id be received in timeout period */
    IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT = 7,

    /* A failed ACK to the specific unsubscribe which specify by packet-id be received*/
    IOTX_MQTT_EVENT_UNSUBCRIBE_NACK = 8,

    /* A ACK to the specific publish which specify by packet-id be received */
    IOTX_MQTT_EVENT_PUBLISH_SUCCESS = 9,

    /* No ACK to the specific publish which specify by packet-id be received in timeout period */
    IOTX_MQTT_EVENT_PUBLISH_TIMEOUT = 10,

    /* A failed ACK to the specific publish which specify by packet-id be received*/
    IOTX_MQTT_EVENT_PUBLISH_NACK = 11,

    /* MQTT packet published from MQTT remote broker be received */
    IOTX_MQTT_EVENT_PUBLISH_RECVEIVED = 12,

} iotx_mqtt_event_type_t;


/* topic information */
typedef struct {
    uint16_t packet_id;
    uint8_t qos;
    uint8_t dup;
    uint8_t retain;
    uint16_t topic_len;
    uint16_t payload_len;
    const char *ptopic;
    const char *payload;
} iotx_mqtt_topic_info_t, *iotx_mqtt_topic_info_pt;


typedef struct {

    /* Specify the event type */
    iotx_mqtt_event_type_t event_type;

    /*
     * Specify the detail event information. @msg means different to different event types:
     *
     * 1) IOTX_MQTT_EVENT_UNKNOWN,
     *    IOTX_MQTT_EVENT_DISCONNECT,
     *    IOTX_MQTT_EVENT_RECONNECT :
     *      Its data type is string and the value is detail information.
     *
     * 2) IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS,
     *    IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT,
     *    IOTX_MQTT_EVENT_SUBCRIBE_NACK,
     *    IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS,
     *    IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT,
     *    IOTX_MQTT_EVENT_UNSUBCRIBE_NACK
     *    IOTX_MQTT_EVENT_PUBLISH_SUCCESS,
     *    IOTX_MQTT_EVENT_PUBLISH_TIMEOUT,
     *    IOTX_MQTT_EVENT_PUBLISH_NACK :
     *      Its data type is @uint32_t and the value is MQTT packet identifier.
     *
     * 3) IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
     *      Its data type is @iotx_mqtt_packet_info_t and see detail at the declare of this type.
     *
     * */
    void *msg;
} iotx_mqtt_event_msg_t, *iotx_mqtt_event_msg_pt;


/**
 * @brief It define a datatype of function pointer.
 *        This type of function will be called when a related event occur.
 *
 * @param pcontext, the program context
 * @param pclient, the MQTT client
 * @param msg, the event message.
 *
 * @return none
 */
typedef void (*iotx_mqtt_event_handle_func_fpt)(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg);


/* The structure of MQTT event handle */
typedef struct {
    iotx_mqtt_event_handle_func_fpt h_fp;
    void *pcontext; //context pointer for handle
} iotx_mqtt_event_handle_t, *iotx_mqtt_event_handle_pt;


/* The structure of MQTT initial parameter */
typedef struct {

    uint16_t                    port;                   /* Specify MQTT broker port */
    const char                 *host;                   /* Specify MQTT broker host */
    const char                 *client_id;              /* Specify MQTT connection client id*/
    const char                 *user_name;              /* Specify MQTT user name */
    const char                 *password;               /* Specify MQTT password */

    /* Specify MQTT transport channel and key.
     * If the value is NULL, it means that use TCP channel,
     * If the value is NOT NULL, it means that use SSL/TLS channel and
     *   @pub_key point to the CA certification */
    const char                 *pub_key;

    uint8_t                     clean_session;            /* Specify MQTT clean session or not*/
    uint32_t                    request_timeout_ms;       /* Specify timeout of a MQTT request in millisecond */
    uint32_t                    keepalive_interval_ms;    /* Specify MQTT keep-alive interval in millisecond */

    char                       *pwrite_buf;               /* Specify write-buffer */
    uint32_t                    write_buf_size;           /* Specify size of write-buffer in byte */
    char                       *pread_buf;                /* Specify read-buffer */
    uint32_t                    read_buf_size;            /* Specify size of read-buffer in byte */

    iotx_mqtt_event_handle_t   handle_event;             /* Specify MQTT event handle */

} iotx_mqtt_param_t, *iotx_mqtt_param_pt;


/**
 * @brief Construct the MQTT client
 *        This function initialize the data structures, establish MQTT connection.
 *
 * @param pInitParams, specify the MQTT client parameter.
 *
 * @return NULL, construct failed; NOT NULL, the handle of MQTT client.
 */
void *IOT_MQTT_Construct(iotx_mqtt_param_t *pInitParams);


/**
 * @brief Deconstruct the MQTT client
 *        This function disconnect MQTT connection and release the related resource.
 *
 * @param handle, specify the MQTT client.
 *
 * @return 0, deconstruct success; -1, deconstruct failed.
 */
int IOT_MQTT_Destroy(void *handle);


/**
 * @brief Handle MQTT packet from remote server and process timeout request
 *        which include the MQTT subscribe, unsubscribe, publish(QOS >= 1), reconnect, etc..
 *
 * @param handle, specify the MQTT client.
 * @param timeout, specify the timeout in millisecond in this loop.
 *
 * @return none.
 */
void IOT_MQTT_Yield(void *handle, int timeout_ms);


/**
 * @brief check whether MQTT connection is established or not.
 *
 * @param handle, specify the MQTT client.
 *
 * @return true, MQTT in normal state; false, MQTT in abnormal state.
 */
bool IOT_MQTT_CheckStateNormal(void *handle);


/**
 * @brief Subscribe MQTT topic.
 *
 * @param handle, specify the MQTT client.
 * @param topic_filter, specify the topic filter.
 * @param qos, specify the MQTT Requested QoS.
 * @param topic_handle_func, specify the topic handle callback-function.
 * @param pcontext, specify context. When call @topic_handle_func, it will be passed back.
 *
 * @return
 * @verbatim
      -1, subscribe failed.
     >=0, subscribe successful.
          The value is a unique ID of this request.
          The ID will be passed back when callback @iotx_mqtt_param_t:handle_event.
   @endverbatim
 */
int32_t IOT_MQTT_Subscribe(void *handle,
                            const char *topic_filter,
                            iotx_mqtt_qos_t qos,
                            iotx_mqtt_event_handle_func_fpt topic_handle_func,
                            void *pcontext);


/**
 * @brief Unsubscribe MQTT topic.
 *
 * @param handle, specify the MQTT client.
 * @param topic_filter, specify the topic filter.
 *
 * @return
 * @verbatim
      -1, unsubscribe failed.
     >=0, unsubscribe successful.
          The value is a unique ID of this request.
          The ID will be passed back when callback @iotx_mqtt_param_t:handle_event.
   @endverbatim
 */
int32_t IOT_MQTT_Unsubscribe(void *handle, const char *topic_filter);


/**
 * @brief Publish message to specific topic.
 *
 * @param handle, specify the MQTT client.
 * @param topic_name, specify the topic name.
 * @param topic_msg, specify the topic message.
 *
 * @return
 * @verbatim
    -1, publish failed.
     0, publish successful, where QoS is 0.
    >0, publish successful, where QoS is >= 0.
        The value is a unique ID of this request.
        The ID will be passed back when callback @iotx_mqtt_param_t:handle_event.
 * @endverbatim
 *
 */
int32_t IOT_MQTT_Publish(void *handle, const char *topic_name, iotx_mqtt_topic_info_pt topic_msg);
/* From mqtt_client.h */

/* From device.h */
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
#define AESKEY_HEX_LEN      (128/8)

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
#ifdef MQTT_ID2_AUTH
    char            aeskey_str[AESKEY_STR_LEN];
    uint8_t         aeskey_hex[AESKEY_HEX_LEN];
#endif
} iotx_conn_info_t, *iotx_conn_info_pt;

int IOT_CreateDeviceInfo(void);

int32_t IOT_SetDeviceInfo(
            const char *product_key,
            const char *device_name,
            const char *device_secret);

iotx_device_info_pt IOT_GetDeviceInfo(void);

iotx_conn_info_pt IOT_GetConnInfo(void);
/* From device.h */

/* From shadow.h */
typedef enum {
    IOTX_SHADOW_ACK_TIMEOUT = -1,
    IOTX_SHADOW_ACK_NONE = 0,
    IOTX_SHADOW_ACK_SUCCESS = 200,
    IOTX_SHADOW_ACK_ERR_JSON_FMT_IS_INVALID = 400,
    IOTX_SHADOW_ACK_ERR_METHOD_IS_NULL = 401,
    IOTX_SHADOW_ACK_ERR_STATE_IS_NULL = 402,
    IOTX_SHADOW_ACK_ERR_VERSION_IS_INVALID = 403,
    IOTX_SHADOW_ACK_ERR_REPORTED_IS_NULL = 404,
    IOTX_SHADOW_ACK_ERR_REPORTED_ATTRIBUTE_IS_NULL = 405,
    IOTX_SHADOW_ACK_ERR_METHOD_IS_INVALID = 406,
    IOTX_SHADOW_ACK_ERR_SHADOW_DOCUMENT_IS_NULL = 407,
    IOTX_SHADOW_ACK_ERR_ATTRIBUTE_EXCEEDED = 408,
    IOTX_SHADOW_ACK_ERR_SERVER_FAILED = 500,
} iotx_shadow_ack_code_t;

typedef enum {
    IOTX_SHADOW_READONLY,
    IOTX_SHADOW_WRITEONLY,
    IOTX_SHADOW_RW
} iotx_shadow_datamode_t;

typedef enum {
    IOTX_SHADOW_NULL,
    IOTX_SHADOW_INT32,
    IOTX_SHADOW_STRING,
} iotx_shadow_attr_datatype_t;

typedef struct {
    bool flag_new;
    uint32_t buf_size;
    uint32_t offset;
    char *buf;
} format_data_t, *format_data_pt;

typedef struct {
    uint32_t base_system_time; //in millisecond
    uint32_t epoch_time;
} iotx_shadow_time_t, *iotx_shadow_time_pt;

typedef void (*iotx_push_cb_fpt)(
            void *pcontext,
            iotx_shadow_ack_code_t ack_code,
            const char *ack_msg, // NOTE: NOT a string.
            uint32_t ack_msg_len);

struct iotx_shadow_attr_st;

typedef void (*iotx_shadow_attr_cb_t)(struct iotx_shadow_attr_st *pattr);
typedef struct iotx_shadow_attr_st {
    iotx_shadow_datamode_t mode;       ///< data mode
    const char *pattr_name;             ///< attribute name
    void *pattr_data;                   ///< pointer to the attribute data
    iotx_shadow_attr_datatype_t attr_type;    ///< data type
    uint32_t timestamp;                 ///timestamp in Epoch(Unix) format
    iotx_shadow_attr_cb_t callback;    ///< callback when related control message come.
} iotx_shadow_attr_t, *iotx_shadow_attr_pt;

typedef struct {
    iotx_mqtt_param_t mqtt;
} iotx_shadow_para_t, *iotx_shadow_para_pt;

/**
 * @brief Construct the Device Shadow
 *        This function initialize the data structures, establish MQTT connection
 *        and subscribe the topic: "/shadow/get/${product_key}/${device_name}"
 *
 * @param pClient, A device shadow client data structure.
 * @param pParams, The specific initial parameter.
 *
 * @return NULL, construct shadow failed; NOT NULL, deconstruct failed.
 */
void *IOT_Shadow_Construct(iotx_shadow_para_pt pparam);


/* Deconstruct the specific device shadow */
iotx_err_t IOT_Shadow_Destroy(void *handle);


/* Handle MQTT packet from cloud and wait list */
void IOT_Shadow_Yield(void *handle, uint32_t timeout);


/* Register the specific attribute */
iotx_err_t IOT_Shadow_RegisterAttribute(void *handle, iotx_shadow_attr_pt pattr);


/* Delete the specific attribute */
iotx_err_t IOT_Shadow_DeleteAttribute(void *handle, iotx_shadow_attr_pt pattr);


/* Format the attribute name and value for update */
iotx_err_t IOT_Shadow_PushFormat_Init(
            void *handle,
            format_data_pt pformat,
            char *buf,
            uint16_t size);

iotx_err_t IOT_Shadow_PushFormat_Add(
            void *handle,
            format_data_pt pformat,
            iotx_shadow_attr_pt pattr);

iotx_err_t IOT_Shadow_PushFormat_Finalize(void *handle, format_data_pt pformat);


/* Update data to Cloud. It is a synchronous interface. */
iotx_err_t IOT_Shadow_Push(
            void *handle,
            char *data,
            uint32_t data_len,
            uint16_t timeout_s);


/* Update data to Cloud. It is a asynchronous interface.
 * The result of this update will be informed by calling the callback function @cb_fpt */
iotx_err_t IOT_Shadow_Push_Async(
            void *handle,
            char *data,
            size_t data_len,
            uint16_t timeout_s,
            iotx_push_cb_fpt cb_fpt,
            void *pcontext);


/* Synchronize device shadow data from cloud. It is a synchronous interface. */
iotx_err_t IOT_Shadow_Pull(void *handle);
/* From shadow.h */

#if defined(__cplusplus)
}
#endif
#endif  /* __IOT_EXPORT_H__ */

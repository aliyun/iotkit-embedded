

#ifndef _ALIOT_MQTT_CLIENT_H_
#define _ALIOT_MQTT_CLIENT_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aliot_platform.h"


typedef enum {
    ALIOT_MQTT_QOS0 = 0,
    ALIOT_MQTT_QOS1,
    ALIOT_MQTT_QOS2
}aliot_mqtt_qos_t;


typedef enum {

    /* Undefined event */
    ALIOT_MQTT_EVENT_UNDEF = 0,

    /* MQTT disconnect event */
    ALIOT_MQTT_EVENT_DISCONNECT = 1,

    /* MQTT reconnect event */
    ALIOT_MQTT_EVENT_RECONNECT = 2,

    /* A ACK to the specific subscribe which specify by packet-id be received */
    ALIOT_MQTT_EVENT_SUBCRIBE_SUCCESS = 3,

    /* No ACK to the specific subscribe which specify by packet-id be received in timeout period */
    ALIOT_MQTT_EVENT_SUBCRIBE_TIMEOUT = 4,

    /* A failed ACK to the specific subscribe which specify by packet-id be received*/
    ALIOT_MQTT_EVENT_SUBCRIBE_NACK = 5,

    /* A ACK to the specific unsubscribe which specify by packet-id be received */
    ALIOT_MQTT_EVENT_UNSUBCRIBE_SUCCESS = 6,

    /* No ACK to the specific unsubscribe which specify by packet-id be received in timeout period */
    ALIOT_MQTT_EVENT_UNSUBCRIBE_TIMEOUT = 7,

    /* A failed ACK to the specific unsubscribe which specify by packet-id be received*/
    ALIOT_MQTT_EVENT_UNSUBCRIBE_NACK = 8,

    /* A ACK to the specific publish which specify by packet-id be received */
    ALIOT_MQTT_EVENT_PUBLISH_SUCCESS = 9,

    /* No ACK to the specific publish which specify by packet-id be received in timeout period */
    ALIOT_MQTT_EVENT_PUBLISH_TIMEOUT = 10,

    /* A failed ACK to the specific publish which specify by packet-id be received*/
    ALIOT_MQTT_EVENT_PUBLISH_NACK = 11,

    /* MQTT packet published from MQTT remote broker be received */
    ALIOT_MQTT_EVENT_PUBLISH_RECVEIVED = 12,

} aliot_mqtt_event_type_t;


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
} aliot_mqtt_topic_info_t, *aliot_mqtt_topic_info_pt;


typedef struct {

    /* Specify the event type */
    aliot_mqtt_event_type_t event_type;

    /*
     * Specify the detail event information. @msg means different to different event types:
     *
     * 1) ALIOT_MQTT_EVENT_UNKNOWN,
     *    ALIOT_MQTT_EVENT_DISCONNECT,
     *    ALIOT_MQTT_EVENT_RECONNECT :
     *      Its data type is string and the value is detail information.
     *
     * 2) ALIOT_MQTT_EVENT_SUBCRIBE_SUCCESS,
     *    ALIOT_MQTT_EVENT_SUBCRIBE_TIMEOUT,
     *    ALIOT_MQTT_EVENT_SUBCRIBE_NACK,
     *    ALIOT_MQTT_EVENT_UNSUBCRIBE_SUCCESS,
     *    ALIOT_MQTT_EVENT_UNSUBCRIBE_TIMEOUT,
     *    ALIOT_MQTT_EVENT_UNSUBCRIBE_NACK
     *    ALIOT_MQTT_EVENT_PUBLISH_SUCCESS,
     *    ALIOT_MQTT_EVENT_PUBLISH_TIMEOUT,
     *    ALIOT_MQTT_EVENT_PUBLISH_NACK :
     *      Its data type is @uint32_t and the value is MQTT packet identifier.
     *
     * 3) ALIOT_MQTT_EVENT_PUBLISH_RECVEIVED:
     *      Its data type is @aliot_mqtt_packet_info_t and see detail at the declare of this type.
     *
     * */
    void *msg;
}aliot_mqtt_event_msg_t, *aliot_mqtt_event_msg_pt;


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
typedef void (*aliot_mqtt_event_handle_func_fpt)(void *pcontext, void *pclient, aliot_mqtt_event_msg_pt msg);


/* The structure of MQTT event handle */
typedef struct {
    aliot_mqtt_event_handle_func_fpt h_fp;
    void *pcontext; //context pointer for handle
} aliot_mqtt_event_handle_t, *aliot_mqtt_event_handle_pt;


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

    aliot_mqtt_event_handle_t   handle_event;             /* Specify MQTT event handle */

} aliot_mqtt_param_t, *aliot_mqtt_param_pt;


/**
 * @brief Construct the MQTT client
 *        This function initialize the data structures, establish MQTT connection.
 *
 * @param pInitParams, specify the MQTT client parameter.
 *
 * @return NULL, construct failed; NOT NULL, the handle of MQTT client.
 */
void *aliot_mqtt_construct(aliot_mqtt_param_t *pInitParams);


/**
 * @brief Deconstruct the MQTT client
 *        This function disconnect MQTT connection and release the related resource.
 *
 * @param handle, specify the MQTT client.
 *
 * @return 0, deconstruct success; -1, deconstruct failed.
 */
int aliot_mqtt_deconstruct(void *handle);


/**
 * @brief Handle MQTT packet from remote server and process timeout request
 *        which include the MQTT subscribe, unsubscribe, publish(QOS >= 1), reconnect, etc..
 *
 * @param handle, specify the MQTT client.
 * @param timeout, specify the timeout in millisecond in this loop.
 *
 * @return none.
 */
void aliot_mqtt_yield(void *handle, int timeout_ms);


/**
 * @brief check whether MQTT connection is established or not.
 *
 * @param handle, specify the MQTT client.
 *
 * @return true, MQTT in normal state; false, MQTT in abnormal state.
 */
bool aliot_mqtt_check_state_normal(void *handle);


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
          The ID will be passed back when callback @aliot_mqtt_param_t:handle_event.
   @endverbatim
 */
int32_t aliot_mqtt_subscribe(void *handle,
                const char *topic_filter,
                aliot_mqtt_qos_t qos,
                aliot_mqtt_event_handle_func_fpt topic_handle_func,
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
          The ID will be passed back when callback @aliot_mqtt_param_t:handle_event.
   @endverbatim
 */
int32_t aliot_mqtt_unsubscribe(void *handle, const char *topic_filter);


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
        The ID will be passed back when callback @aliot_mqtt_param_t:handle_event.
 * @endverbatim
 *
 */
int32_t aliot_mqtt_publish(void *handle, const char *topic_name, aliot_mqtt_topic_info_pt topic_msg);



#if defined(__cplusplus)
}
#endif

#endif

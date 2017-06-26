
#ifndef _ALIOT_SHADOW_H_
#define _ALIOT_SHADOW_H_

#include "aliot_platform.h"
#include "aliot_error.h"
#include "aliot_mqtt_client.h"


//ads is an abbreviation for aliot device shadow.


typedef enum {
    ALIOT_SHADOW_ACK_TIMEOUT = -1,
    ALIOT_SHADOW_ACK_NONE = 0,
    ALIOT_SHADOW_ACK_SUCCESS = 200,
    ALIOT_SHADOW_ACK_ERR_JSON_FMT_IS_INVALID = 400,
    ALIOT_SHADOW_ACK_ERR_METHOD_IS_NULL = 401,
    ALIOT_SHADOW_ACK_ERR_STATE_IS_NULL = 402,
    ALIOT_SHADOW_ACK_ERR_VERSION_IS_INVALID = 403,
    ALIOT_SHADOW_ACK_ERR_REPORTED_IS_NULL = 404,
    ALIOT_SHADOW_ACK_ERR_REPORTED_ATTRIBUTE_IS_NULL = 405,
    ALIOT_SHADOW_ACK_ERR_METHOD_IS_INVALID = 406,
    ALIOT_SHADOW_ACK_ERR_SHADOW_DOCUMENT_IS_NULL = 407,
    ALIOT_SHADOW_ACK_ERR_ATTRIBUTE_EXCEEDED = 408,
    ALIOT_SHADOW_ACK_ERR_SERVER_FAILED = 500,
} aliot_shadow_ack_code_t;


typedef enum {
    ALIOT_SHADOW_READONLY,
    ALIOT_SHADOW_WRITEONLY,
    ALIOT_SHADOW_RW
} aliot_shadow_datamode_t;


typedef enum {
    ALIOT_SHADOW_NULL,
    ALIOT_SHADOW_INT32,
    ALIOT_SHADOW_STRING,
} aliot_shadow_attr_datatype_t;


typedef struct {
    bool flag_new;
    uint32_t buf_size;
    uint32_t offset;
    char *buf;
} format_data_t, *format_data_pt;


typedef struct {
    uint32_t base_system_time; //in millisecond
    uint32_t epoch_time;
} aliot_shadow_time_t, *aliot_shadow_time_pt;


typedef void (*aliot_update_cb_fpt)(
                void *pcontext,
                aliot_shadow_ack_code_t ack_code,
                const char *ack_msg, //NOTE: NOT a string.
                uint32_t ack_msg_len);


struct aliot_shadow_attr_st;

typedef void (*aliot_shadow_attr_cb_t)(struct aliot_shadow_attr_st *pattr);

typedef struct aliot_shadow_attr_st {
    aliot_shadow_datamode_t mode;       ///< data mode
    const char *pattr_name;             ///< attribute name
    void *pattr_data;                   ///< pointer to the attribute data
    aliot_shadow_attr_datatype_t attr_type;    ///< data type
    uint32_t timestamp;                 ///timestamp in Epoch(Unix) format
    aliot_shadow_attr_cb_t callback;    ///< callback when related control message come.
} aliot_shadow_attr_t, *aliot_shadow_attr_pt;


typedef struct {
    aliot_mqtt_param_t mqtt;
} aliot_shadow_para_t, *aliot_shadow_para_pt;


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
void *aliot_shadow_construct(aliot_shadow_para_pt pparam);


/* Deconstruct the specific device shadow */
aliot_err_t aliot_shadow_deconstruct(void *handle);


/* Handle MQTT packet from cloud and wait list */
void aliot_shadow_yield(void *handle, uint32_t timeout);


/* Register the specific attribute */
aliot_err_t aliot_shadow_register_attribute(void *handle, aliot_shadow_attr_pt pattr);


/* Delete the specific attribute */
aliot_err_t aliot_shadow_delete_attribute(void *handle, aliot_shadow_attr_pt pattr);


/* Format the attribute name and value for update */
aliot_err_t aliot_shadow_update_format_init(
                void *handle,
                format_data_pt pformat,
                char *buf,
                uint16_t size);

aliot_err_t aliot_shadow_update_format_add(
                void *handle,
                format_data_pt pformat,
                aliot_shadow_attr_pt pattr);

aliot_err_t aliot_shadow_update_format_finalize(void *handle, format_data_pt pformat);


/* Update data to Cloud. It is a synchronous interface. */
aliot_err_t aliot_shadow_update(
                void *handle,
                char *data,
                uint32_t data_len,
                uint16_t timeout_s);


/* Update data to Cloud. It is a asynchronous interface.
 * The result of this update will be informed by calling the callback function @cb_fpt */
aliot_err_t aliot_shadow_update_asyn(
                void *handle,
                char *data,
                size_t data_len,
                uint16_t timeout_s,
                aliot_update_cb_fpt cb_fpt,
                void *pcontext);


/* Synchronize device shadow data from cloud. It is a synchronous interface. */
aliot_err_t aliot_shadow_sync(void *handle);


#endif /* _ALIOT_SHADOW_H_ */

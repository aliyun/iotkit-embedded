#include "iotx_cm.h"
#include "iotx_cm_internal.h"
#include "iotx_cm_mqtt.h"

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"
#include "dev_sign_api.h"

#if defined(MQTT_COMM_ENABLED) || defined(MAL_ENABLED)
/** CM default parameters define **/
#define CM_MQTT_IS_CLEAN_SESSION            (0)
#define CM_MQTT_REQUEST_TIMEOUT_MS          (2000)
#define CM_MQTT_KEEPALIVE_INTERVAL_MS       (60000)
#define CM_MQTT_RX_MAXLEN                   (CONFIG_MQTT_RX_MAXLEN)
#define CM_MQTT_TX_MAXLEN                   (CONFIG_MQTT_TX_MAXLEN)


static iotx_cm_connection_t *_mqtt_conncection = NULL;
static iotx_sign_mqtt_t g_mqtt_sign;
static void iotx_cloud_conn_mqtt_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg);
static int  _mqtt_connect(uint32_t timeout);
static int _mqtt_publish(iotx_cm_ext_params_t *params, const char *topic, const char *payload,
                         unsigned int payload_len);
static int _mqtt_sub(iotx_cm_ext_params_t *params, const char *topic,
                     iotx_cm_data_handle_cb topic_handle_func, void *pcontext);
static iotx_mqtt_qos_t _get_mqtt_qos(iotx_cm_ack_types_t ack_type);
static int _mqtt_unsub(const char *topic);
static int _mqtt_close(void);
static void _set_common_handlers(void);

iotx_cm_connection_t *iotx_cm_open_mqtt(iotx_cm_init_param_t *params)
{
    ALINK_ASSERT_DEBUG(params != NULL);

    if (_mqtt_conncection != NULL) {
        return _mqtt_conncection;
    }

    _mqtt_conncection = (iotx_cm_connection_t *)cm_malloc(sizeof(iotx_cm_connection_t));
    if (_mqtt_conncection == NULL) {
        cm_err("_mqtt_conncection malloc failed!");
        goto failed;
    }

    _mqtt_conncection->open_params = NULL;
    _mqtt_conncection->event_handler = params->handle_event;
    _mqtt_conncection->cb_data = params->context;
    _set_common_handlers();

    return _mqtt_conncection;
failed:

    if (_mqtt_conncection != NULL) {
        cm_free(_mqtt_conncection);
        _mqtt_conncection = NULL;
    }

    return NULL;
}


static void iotx_cloud_conn_mqtt_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;

    if (_mqtt_conncection == NULL) {
        return;
    }

    switch (msg->event_type) {

        case IOTX_MQTT_EVENT_DISCONNECT: {
            iotx_cm_event_msg_t event;
            cm_info("disconnected,fd = %d", _mqtt_conncection->fd);
            event.type = IOTX_CM_EVENT_CLOUD_DISCONNECT;
            event.msg = NULL;
            if (_mqtt_conncection->event_handler) {
                _mqtt_conncection->event_handler(_mqtt_conncection->fd, &event, _mqtt_conncection->cb_data);
            }
        }
        break;

        case IOTX_MQTT_EVENT_RECONNECT: {
            iotx_cm_event_msg_t event;
            cm_info("connected,fd = %d", _mqtt_conncection->fd);
            event.type = IOTX_CM_EVENT_CLOUD_CONNECTED;
            event.msg = NULL;
            /* cm_info(cm_log_info_MQTT_reconnect); */

            if (_mqtt_conncection->event_handler) {
                _mqtt_conncection->event_handler(_mqtt_conncection->fd, &event, _mqtt_conncection->cb_data);
            }
        }
        break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS: {
            iotx_cm_event_msg_t event;
            event.type = IOTX_CM_EVENT_SUBCRIBE_SUCCESS;
            event.msg = (void *)packet_id;

            if (_mqtt_conncection->event_handler) {
                _mqtt_conncection->event_handler(_mqtt_conncection->fd, &event, _mqtt_conncection->cb_data);
            }
        }
        break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT: {
            iotx_cm_event_msg_t event;
            event.type = IOTX_CM_EVENT_SUBCRIBE_FAILED;
            event.msg = (void *)packet_id;

            if (_mqtt_conncection->event_handler) {
                _mqtt_conncection->event_handler(_mqtt_conncection->fd, &event, _mqtt_conncection->cb_data);
            }
        }
        break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS: {
            iotx_cm_event_msg_t event;
            event.type = IOTX_CM_EVENT_UNSUB_SUCCESS;
            event.msg = (void *)packet_id;

            if (_mqtt_conncection->event_handler) {
                _mqtt_conncection->event_handler(_mqtt_conncection->fd, &event, _mqtt_conncection->cb_data);
            }
        }
        break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT: {
            iotx_cm_event_msg_t event;
            event.type = IOTX_CM_EVENT_UNSUB_FAILED;
            event.msg = (void *)packet_id;

            if (_mqtt_conncection->event_handler) {
                _mqtt_conncection->event_handler(_mqtt_conncection->fd, &event, _mqtt_conncection->cb_data);
            }
        }
        break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS: {
            iotx_cm_event_msg_t event;
            event.type = IOTX_CM_EVENT_PUBLISH_SUCCESS;
            event.msg = (void *)packet_id;

            if (_mqtt_conncection->event_handler) {
                _mqtt_conncection->event_handler(_mqtt_conncection->fd, &event, _mqtt_conncection->cb_data);
            }
        }
        break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT: {
            iotx_cm_event_msg_t event;
            event.type = IOTX_CM_EVENT_PUBLISH_FAILED;
            event.msg = (void *)packet_id;

            if (_mqtt_conncection->event_handler) {
                _mqtt_conncection->event_handler(_mqtt_conncection->fd, &event, _mqtt_conncection->cb_data);
            }
        }
        break;

        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED: {
            iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;
            iotx_cm_data_handle_cb topic_handle_func = (iotx_cm_data_handle_cb)pcontext;

            if (topic_handle_func == NULL) {    
                cm_err("sub handle is null!");
                return;
            }

            topic_handle_func(_mqtt_conncection->fd, topic_info->ptopic, topic_info->topic_len, topic_info->payload, topic_info->payload_len, NULL);
        }
        break;

        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
            cm_warning("buffer overflow", msg->msg);
            break;

        default:
            cm_warning("msg type unkown, type = %d", msg->event_type);
            break;
    }
}


static int  _mqtt_connect(uint32_t timeout)
{
    void *pclient;
    iotx_time_t timer;
    iotx_mqtt_param_t mqtt_param;
    iotx_dev_meta_info_t dev_info;

    if (_mqtt_conncection == NULL) {
        return FAIL_RETURN;
    }

    HAL_GetProductKey(dev_info.product_key);
    HAL_GetProductSecret(dev_info.product_secret);
    HAL_GetDeviceName(dev_info.device_name);
    HAL_GetDeviceSecret(dev_info.device_secret);

    /* Device AUTH */
    if (SUCCESS_RETURN != IOT_Sign_MQTT(IOTX_CLOUD_REGION_SHANGHAI, &dev_info, &g_mqtt_sign)) {         /* TODO: can't get region params!!! */
        cm_err("sign failed");
        return FAIL_RETURN;
    }

    mqtt_param.port = g_mqtt_sign.port;
    mqtt_param.host = g_mqtt_sign.hostname;
    mqtt_param.client_id = g_mqtt_sign.clientid;
    mqtt_param.username = g_mqtt_sign.username;
    mqtt_param.password = g_mqtt_sign.password;

    mqtt_param.pub_key = NULL;
    mqtt_param.clean_session = CM_MQTT_IS_CLEAN_SESSION;
    mqtt_param.request_timeout_ms = CM_MQTT_REQUEST_TIMEOUT_MS;
    mqtt_param.keepalive_interval_ms = CM_MQTT_KEEPALIVE_INTERVAL_MS;
    mqtt_param.read_buf_size = CM_MQTT_RX_MAXLEN;
    mqtt_param.write_buf_size = CM_MQTT_TX_MAXLEN;
    mqtt_param.handle_event.pcontext = NULL;
    mqtt_param.handle_event.h_fp = iotx_cloud_conn_mqtt_event_handle;

    iotx_time_init(&timer);
    utils_time_countdown_ms(&timer, timeout);

    do {
        pclient = IOT_MQTT_Construct((iotx_mqtt_param_t *)&mqtt_param);

        if (pclient != NULL) {
            _mqtt_conncection->context = pclient;
            return 0;
        }
    } while (!utils_time_is_expired(&timer));

    cm_err("mqtt connect failed");

    return -1;
}

static int _mqtt_publish(iotx_cm_ext_params_t *ext, const char *topic, const char *payload, unsigned int payload_len)
{
    int qos = 0;

    if (_mqtt_conncection == NULL) {
        return NULL_VALUE_ERROR;
    }

    if (ext != NULL) {
        qos = (int)_get_mqtt_qos(ext->ack_type);
    }
    return IOT_MQTT_Publish_Simple(_mqtt_conncection->context, topic, qos, (void *)payload, payload_len);
}

static int _mqtt_yield(uint32_t timeout)
{
    if (_mqtt_conncection == NULL) {
        return NULL_VALUE_ERROR;
    }

    return IOT_MQTT_Yield(_mqtt_conncection->context, timeout);
}

static int _mqtt_sub(iotx_cm_ext_params_t *ext, const char *topic,
                     iotx_cm_data_handle_cb topic_handle_func, void *pcontext)
{
    int sync = 0;
    int qos = 0;
    int timeout;
    int ret;

    if (_mqtt_conncection == NULL || topic == NULL || topic_handle_func == NULL) {
        return NULL_VALUE_ERROR;
    }

    if (ext != NULL) {
        if (ext->sync_mode == IOTX_CM_ASYNC) {
            sync = 0;
        } else {
            sync = 1;
            timeout = ext->sync_timeout;
        }
        qos = (int)_get_mqtt_qos(ext->ack_type);
    }

    if (sync != 0) {
        ret = IOT_MQTT_Subscribe_Sync(_mqtt_conncection->context,
                                      topic,
                                      (iotx_mqtt_qos_t)qos,
                                      iotx_cloud_conn_mqtt_event_handle,
                                      (void *)topic_handle_func,
                                      timeout);
    } else {
        ret = IOT_MQTT_Subscribe(_mqtt_conncection->context,
                                 topic,
                                 (iotx_mqtt_qos_t)qos,
                                 iotx_cloud_conn_mqtt_event_handle,
                                 (void *)topic_handle_func);
    }

    return ret;
}

static int _mqtt_unsub(const char *topic)
{
    int ret;

    if (_mqtt_conncection == NULL) {
        return NULL_VALUE_ERROR;
    }

    ret = IOT_MQTT_Unsubscribe(_mqtt_conncection->context, topic);

    if (ret < 0) {
        return -1;
    }

    return ret;
}

static int _mqtt_close(void)
{
    if (_mqtt_conncection == NULL) {
        return NULL_VALUE_ERROR;
    }

    cm_free(_mqtt_conncection->open_params);
    IOT_MQTT_Destroy(&_mqtt_conncection->context);
    cm_free(_mqtt_conncection);
    _mqtt_conncection = NULL;
    return 0;
}

static iotx_mqtt_qos_t _get_mqtt_qos(iotx_cm_ack_types_t ack_type)
{
    switch (ack_type) {
        case IOTX_CM_MESSAGE_NO_ACK:
            return IOTX_MQTT_QOS0;

        case IOTX_CM_MESSAGE_NEED_ACK:
            return IOTX_MQTT_QOS1;

        default:
            return IOTX_MQTT_QOS0;
    }
}


static void _set_common_handlers(void)
{
    if (_mqtt_conncection != NULL) {
        _mqtt_conncection->connect_func = _mqtt_connect;
        _mqtt_conncection->sub_func = _mqtt_sub;
        _mqtt_conncection->unsub_func = _mqtt_unsub;
        _mqtt_conncection->pub_func = _mqtt_publish;
        _mqtt_conncection->yield_func = (iotx_cm_yield_fp)_mqtt_yield;
        _mqtt_conncection->close_func = _mqtt_close;
    }
}

#endif


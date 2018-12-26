/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_alink_internal.h"
#include "alink_wrapper.h"

#include "mqtt_api.h"
#include "dev_sign_api.h"
#include "infra_list.h"


/** **/
#define ALINK_BEARER_MQTT_REQUEST_TIMEOUT_MS        (2000)
/** **/
#define ALINK_BEARER_MQTT_IS_CLEAN_SESSION          (0)
/** **/
#define ALINK_BEARER_MQTT_KEEPALIVE_INTERVAL_MS     (60000)
/** **/
#define ALINK_BEARER_MQTT_TX_MAXLEN                 CONFIG_MQTT_TX_MAXLEN
/** **/
#define ALINK_BEARER_MQTT_RX_MAXLEN                 CONFIG_MQTT_RX_MAXLEN


/**
 * local functions prototype
 */
static int _mqtt_connect(alink_bearer_node_t *p_bearer_ctx, uint32_t timeout);
static int _mqtt_close(alink_bearer_node_t *p_bearer_ctx);
static int _mqtt_yield(alink_bearer_node_t *p_bearer_ctx, uint32_t timeout);
static int _mqtt_sub(alink_bearer_node_t *p_bearer_ctx, const char *topic, alink_bearer_rx_cb_t topic_handle_func, uint8_t qos, uint32_t timeout);
static int _mqtt_unsub(alink_bearer_node_t *p_bearer_ctx, const char *topic);
static int _mqtt_publish(alink_bearer_node_t *p_bearer_ctx, const char *topic, const uint8_t *payload, uint32_t payload_len, uint8_t qos);

/**
 * 
 */
static alink_bearer_mqtt_ctx_t *p_alink_bearer_mqtt_ctx;


/**
 * add mqtt bearer, TODO, used the complete mqtt bearer context!!!!
 */
int alink_bearer_mqtt_open(alink_bearer_mqtt_ctx_t *p_bearer_mqtt_ctx)
{
    ALINK_ASSERT_DEBUG(p_bearer_mqtt_ctx != NULL);

    p_bearer_mqtt_ctx->bearer.p_api.bearer_connect  = _mqtt_connect;
    p_bearer_mqtt_ctx->bearer.p_api.bearer_close    = _mqtt_close;
    p_bearer_mqtt_ctx->bearer.p_api.bearer_yield    = _mqtt_yield;
    p_bearer_mqtt_ctx->bearer.p_api.bearer_sub      = _mqtt_sub;
    p_bearer_mqtt_ctx->bearer.p_api.bearer_unsub    = _mqtt_unsub;
    p_bearer_mqtt_ctx->bearer.p_api.bearer_pub      = _mqtt_publish;

    /* hehe, todo */
    p_alink_bearer_mqtt_ctx = p_bearer_mqtt_ctx;

    return SUCCESS_RETURN;
}

/**
 * set qos ? TODO
 */
int alink_bearer_mqtt_set_qos(alink_bearer_node_t *p_bearer_ctx, uint8_t qos)
{
    ALINK_ASSERT_DEBUG(p_bearer_ctx != NULL);
    (void)qos;

    int res = FAIL_RETURN;

    return res;
}

/**
 * copy from cm
 */
static void alink_bearer_mqtt_general_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    alink_bearer_node_t *p_bearer_ctx = pcontext;

    if (p_bearer_ctx->p_handle == NULL) {
        return;
    }

    if (p_bearer_ctx->p_cb.bearer_event_cb == NULL) {
        return;
    }

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_DISCONNECT:
        case IOTX_MQTT_EVENT_RECONNECT:
        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
        case IOTX_MQTT_EVENT_PUBLISH_NACK:
        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
        default: break;
    }
}

/**
 * 
 */
void alink_bearer_mqtt_rx_evnet_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;
    alink_bearer_rx_cb_t topic_handle_func = pcontext;

    /* only process publish rx event */
    if (IOTX_MQTT_EVENT_PUBLISH_RECEIVED != msg->event_type) {
        return;
    }

    if (topic_handle_func != NULL) {

        #ifdef TEST_MOCK
        (void)topic_info;

        // char *topic = "/a1OFrRjV8nz/develop_01/req/sys/thing/property/post/?i=898789&c=b&r=400&a=y&c=n";
        // char *topic = "/gw_pk/gw_dn/req/proxy/a1OFrRjV8nz/develop_01/sys/thing/property/post/?i=898789&c=b&a=y&r=400&c=n";
        // char *topic = "/a1OFrRjV8nz/develop_01/req/sys/thing/property/put/?i=898789&c=b&a=y&c=n"; char *payload = "{\"test\": 12344}";
        // char *topic = "/a1OFrRjV8nz/develop_01/rsp/sys/thing/property/post/?i=898789&c=b&a=y&c=n"; char *payload = "{\"code\": 200}";
        // char *topic = "/a1OFrRjV8nz/develop_01/req/sys/thing/property/get/?i=898789&c=b&a=y&c=n"; char *payload = "{\"params\":[\"test\", \"alink\"]}";
        // char *topic = "/a1OFrRjV8nz/develop_01/rsp/sys/thing/event/post/?i=898789&c=b&a=y&c=n"; char *payload = "{\"code\": 200}";
        char *topic = "/a1OFrRjV8nz/develop_01/req/sys/thing/service/put/?i=898789&c=b&a=y&c=n"; char *payload = "{\"serviceId\":\"ACControl\",\"params\":{\"Action\":\"On\",\"FandSpeed\":123}}";


        uint32_t topic_len = strlen(topic);
        uint32_t payload_len = strlen(payload);

        topic_handle_func(NULL, topic, topic_len, payload, payload_len);
        #else
        topic_handle_func(NULL, topic_info->ptopic, topic_info->topic_len, topic_info->payload, topic_info->payload_len);    // TODO
        #endif
    }
}

static int _mqtt_connect(alink_bearer_node_t *p_bearer_ctx, uint32_t timeout)
{
    ALINK_ASSERT_DEBUG(p_bearer_ctx != NULL);

    alink_bearer_mqtt_ctx_t *p_mqtt_ctx;
    (void)timeout;      // TODO

    int res = FAIL_RETURN;
    iotx_sign_mqtt_t sign_mqtt;
    iotx_mqtt_param_t params_mqtt;


    p_mqtt_ctx = container_of(p_bearer_ctx, alink_bearer_mqtt_ctx_t, bearer);

    alink_info("%d", p_mqtt_ctx->region);

    res = IOT_Sign_MQTT(p_mqtt_ctx->region, p_mqtt_ctx->dev_info, &sign_mqtt);

    if (res == FAIL_RETURN) {
        alink_err("sign fail");
        return res;
    }

    params_mqtt.port = sign_mqtt.port;
    params_mqtt.host = sign_mqtt.hostname;
    params_mqtt.client_id = sign_mqtt.clientid;
    params_mqtt.username = sign_mqtt.username;
    params_mqtt.password = sign_mqtt.password;

    params_mqtt.pub_key = NULL;                 // TODO
    params_mqtt.clean_session = ALINK_BEARER_MQTT_IS_CLEAN_SESSION;
    params_mqtt.request_timeout_ms = ALINK_BEARER_MQTT_REQUEST_TIMEOUT_MS;
    params_mqtt.keepalive_interval_ms = ALINK_BEARER_MQTT_KEEPALIVE_INTERVAL_MS;
    params_mqtt.read_buf_size = ALINK_BEARER_MQTT_RX_MAXLEN;
    params_mqtt.write_buf_size = ALINK_BEARER_MQTT_TX_MAXLEN;
    params_mqtt.handle_event.pcontext = p_bearer_ctx;                               // TODO
    params_mqtt.handle_event.h_fp = alink_bearer_mqtt_general_event_handle;         // TODO

    p_bearer_ctx->p_handle = IOT_MQTT_Construct(&params_mqtt);
    if (p_bearer_ctx->p_handle == NULL) {
        res = FAIL_RETURN;
        alink_info("mqtt construct fail");
    }
    else {
        res = SUCCESS_RETURN;
        alink_info("mqtt construct success");
    }

    HAL_Free(sign_mqtt.hostname);
    HAL_Free(sign_mqtt.username);
    HAL_Free(sign_mqtt.password);
    HAL_Free(sign_mqtt.clientid);

    // TODO connection event
    
    return res;
}

static int _mqtt_close(alink_bearer_node_t *p_bearer_ctx)
{
    ALINK_ASSERT_DEBUG(p_bearer_ctx != NULL);

    p_bearer_ctx->p_handle = NULL;
    // TODO, update state
    IOT_MQTT_Destroy(&p_bearer_ctx->p_handle);
    return SUCCESS_RETURN;
}

static int _mqtt_yield(alink_bearer_node_t *p_bearer_ctx, uint32_t timeout)
{
    ALINK_ASSERT_DEBUG(p_bearer_ctx != NULL);
    ALINK_ASSERT_DEBUG(timeout > 0);

    /* IOT_MQTT_Yield always return SUCCESS_RETURN now */
    return IOT_MQTT_Yield(p_bearer_ctx->p_handle, timeout);   
}

/**
 * copy from cm, todo
 */
static int _mqtt_sub(alink_bearer_node_t *p_bearer_ctx, const char *topic, alink_bearer_rx_cb_t topic_handle_func, uint8_t qos, uint32_t timeout)
{
    ALINK_ASSERT_DEBUG(p_bearer_ctx != NULL);
    ALINK_ASSERT_DEBUG(topic != NULL);
    ALINK_ASSERT_DEBUG(topic_handle_func != NULL);

    int res = FAIL_RETURN;

    if (timeout == 0) {
        res = IOT_MQTT_Subscribe(p_bearer_ctx->p_handle,
                                 topic,
                                 qos,
                                 alink_bearer_mqtt_rx_evnet_handle,
                                 topic_handle_func);
    } 
    else {
        res = IOT_MQTT_Subscribe_Sync(p_bearer_ctx->p_handle,
                                      topic,
                                      qos,
                                      alink_bearer_mqtt_rx_evnet_handle,
                                      topic_handle_func,
                                      timeout);
    }

    return res;
}

static int _mqtt_unsub(alink_bearer_node_t *p_bearer_ctx, const char *topic) 
{
    ALINK_ASSERT_DEBUG(p_bearer_ctx != NULL);

    int res = FAIL_RETURN;
    res = IOT_MQTT_Unsubscribe(p_bearer_ctx->p_handle, topic);

    if (res < 0) {
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

static int _mqtt_publish(alink_bearer_node_t *p_bearer_ctx, const char *topic, const uint8_t *payload, uint32_t payload_len, uint8_t qos)
{
    ALINK_ASSERT_DEBUG(p_bearer_ctx != NULL);

    int res = FAIL_RETURN;

    res = IOT_MQTT_Publish_Simple(p_bearer_ctx->p_handle, topic, qos, (void *)payload, payload_len);

    if (res == FAIL_RETURN) {
        alink_err("mqtt pub fail");
    }
    else {
        alink_info("mqtt pub success, msgId = %d", res);
    }

    return res;
}


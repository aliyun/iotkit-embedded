
#include "iot_import.h"
#include "lite/lite-log.h"
#include "lite/lite-utils.h"
#include "mqtt_client.h"
#include "auth.h"
#include "device.h"

//The product and device information from IOT console
//#define PRODUCT_KEY         "6RcIOUafDOm"
//#define DEVICE_NAME         "sh_pre_sample_mqtt"
//#define DEVICE_SECRET       "R0OTtD46DSalSpGW7SFzFDIA6fksTC2c"

#if !defined(DIRECT_MQTT)
    #define PRODUCT_KEY         "OvNmiEYRDSY"
    #define DEVICE_NAME         "sh_online_sample_mqtt"
    #define DEVICE_SECRET       "v9mqGzepKEphLhXmAoiaUIR2HZ7XwTky"
#else
    #define PRODUCT_KEY         "jRCMjOhnScj"
    #define DEVICE_NAME         "dns_test"
    #define DEVICE_SECRET       "OJurfzWl9SsyL6eaxBkMvmHW15KMyn3C"
#endif

//This is the pre-defined topic
#define TOPIC_UPDATE         "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR          "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET            "/"PRODUCT_KEY"/"DEVICE_NAME"/get"
#define TOPIC_DATA           "/"PRODUCT_KEY"/"DEVICE_NAME"/data"

#define MSG_LEN_MAX         (1024)


void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uint32_t packet_id = (uint32_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            log_info("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            log_info("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            log_info("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            log_info("subscribe success, packet-id=%u", packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            log_info("subscribe wait ack timeout, packet-id=%u", packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            log_info("subscribe nack, packet-id=%u", packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            log_info("unsubscribe success, packet-id=%u", packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            log_info("unsubscribe timeout, packet-id=%u", packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            log_info("unsubscribe nack, packet-id=%u", packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            log_info("publish success, packet-id=%u", packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            log_info("publish timeout, packet-id=%u", packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            log_info("publish nack, packet-id=%u", packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
            log_info("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                     topic_info->topic_len,
                     topic_info->ptopic,
                     topic_info->payload_len,
                     topic_info->payload);
            break;

        default:
            log_info("Should NOT arrive here.");
            break;
    }
}


void iotx_mqtt_msg_arrived(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    // print topic name and topic message
    log_info("----");
    log_info("Topic: '%.*s' (Length: %d)",
             ptopic_info->topic_len,
             ptopic_info->ptopic,
             ptopic_info->topic_len);
    log_info("Payload: '%.*s' (Length: %d)",
             ptopic_info->payload_len,
             ptopic_info->payload,
             ptopic_info->payload_len);
    log_info("----");
}

int mqtt_client(void)
{
    int rc = 0, msg_len, cnt = 0;
    void *pclient;
    iotx_user_info_pt puser_info;
    iotx_mqtt_param_t mqtt_params;
    iotx_mqtt_topic_info_t topic_msg;
    char msg_pub[128];
    char *msg_buf = NULL, *msg_readbuf = NULL;

    if (NULL == (msg_buf = (char *)LITE_malloc(MSG_LEN_MAX))) {
        log_debug("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)LITE_malloc(MSG_LEN_MAX))) {
        log_debug("not enough memory");
        rc = -1;
        goto do_exit;
    }

    /* Initialize device info */
    iotx_device_init();

    if (0 != iotx_set_device_info(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET)) {
        log_debug("set device info failed!");
        rc = -1;
        goto do_exit;
    }

    /* Device AUTH */
    if (0 != iotx_auth(iotx_get_device_info(), iotx_get_user_info())) {
        log_debug("AUTH request failed!");
        rc = -1;
        goto do_exit;
    }
    puser_info = iotx_get_user_info();

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = puser_info->port;
    mqtt_params.host = puser_info->host_name;
    mqtt_params.client_id = puser_info->client_id;
    mqtt_params.user_name = puser_info->user_name;
    mqtt_params.password = puser_info->password;
    mqtt_params.pub_key = puser_info->pubKey;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MSG_LEN_MAX;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MSG_LEN_MAX;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;


    /* Construct a MQTT client with specify parameter */
    pclient = iotx_mqtt_construct(&mqtt_params);
    if (NULL == pclient) {
        log_debug("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }

    /* Subscribe the specific topic */
    rc = iotx_mqtt_subscribe(pclient, TOPIC_DATA, IOTX_MQTT_QOS1, iotx_mqtt_msg_arrived, NULL);
    if (rc < 0) {
        iotx_mqtt_deconstruct(pclient);
        log_debug("ali_iot_mqtt_subscribe failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }

    HAL_SleepMs(1000);

    /* Initialize topic information */
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    strcpy(msg_pub, "message: hello! start!");

    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    do {
        /* Generate topic message */
        cnt++;
        msg_len = snprintf(msg_pub, sizeof(msg_pub), "{\"attr_name\":\"temperature\", \"attr_value\":\"%d\"}", cnt);
        if (msg_len < 0) {
            log_debug("Error occur! Exit program");
            rc = -1;
            break;
        }

        topic_msg.payload = (void *)msg_pub;
        topic_msg.payload_len = msg_len;

        rc = iotx_mqtt_publish(pclient, TOPIC_DATA, &topic_msg);
        if (rc < 0) {
            log_debug("error occur when publish");
            rc = -1;
            break;
        }
        log_debug("packet-id=%u, publish topic msg=%s", (uint32_t)rc, msg_pub);

        /* handle the MQTT packet received from TCP or SSL connection */
        iotx_mqtt_yield(pclient, 200);

        //HAL_SleepMs(1000);

    } while (cnt < 1);

    iotx_mqtt_unsubscribe(pclient, TOPIC_DATA);

    HAL_SleepMs(200);

    iotx_mqtt_deconstruct(pclient);


do_exit:
    if (NULL != msg_buf) {
        LITE_free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        LITE_free(msg_readbuf);
    }

    return rc;
}



int main()
{
    LITE_openlog("mqtt");
    LITE_set_loglevel(LOG_DEBUG_LEVEL);

    mqtt_client();

    unittest_json_parser();

    log_debug("out of sample!");
    LITE_dump_malloc_free_stats(LOG_DEBUG_LEVEL);
    LITE_closelog();
    return 0;
}


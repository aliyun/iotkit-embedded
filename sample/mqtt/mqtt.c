
#include "aliot_platform.h"
#include "aliot_log.h"
#include "aliot_mqtt_client.h"
#include "aliot_auth.h"
#include "aliot_device.h"

//The product and device information from IOT console
#define PRODUCT_KEY         "6RcIOUafDOm"
#define DEVICE_NAME         "sh_pre_sample_mqtt"
#define DEVICE_ID           "SWJgsyw28undlXwyFK4J"
#define DEVICE_SECRET       "R0OTtD46DSalSpGW7SFzFDIA6fksTC2c"


//This is the pre-defined topic
#define TOPIC_UPDATE         "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR          "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET            "/"PRODUCT_KEY"/"DEVICE_NAME"/get"
#define TOPIC_DATA           "/"PRODUCT_KEY"/"DEVICE_NAME"/data"

#define MSG_LEN_MAX         (1024)


void event_handle(void *pcontext, void *pclient, aliot_mqtt_event_msg_pt msg)
{
    uint32_t packet_id = (uint32_t)msg->msg;
    aliot_mqtt_topic_info_pt topic_info = (aliot_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type)
    {
    case ALIOT_MQTT_EVENT_UNDEF:
        ALIOT_LOG_INFO("undefined event occur.");
        break;

    case ALIOT_MQTT_EVENT_DISCONNECT:
        ALIOT_LOG_INFO("MQTT disconnect.");
        break;

    case ALIOT_MQTT_EVENT_RECONNECT:
        ALIOT_LOG_INFO("MQTT reconnect.");
        break;

    case ALIOT_MQTT_EVENT_SUBCRIBE_SUCCESS:
        ALIOT_LOG_INFO("subscribe success, packet-id=%u", packet_id);
        break;

    case ALIOT_MQTT_EVENT_SUBCRIBE_TIMEOUT:
        ALIOT_LOG_INFO("subscribe wait ack timeout, packet-id=%u", packet_id);
        break;

    case ALIOT_MQTT_EVENT_SUBCRIBE_NACK:
        ALIOT_LOG_INFO("subscribe nack, packet-id=%u", packet_id);
        break;

    case ALIOT_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
        ALIOT_LOG_INFO("unsubscribe success, packet-id=%u", packet_id);
        break;

    case ALIOT_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
        ALIOT_LOG_INFO("unsubscribe timeout, packet-id=%u", packet_id);
        break;

    case ALIOT_MQTT_EVENT_UNSUBCRIBE_NACK:
        ALIOT_LOG_INFO("unsubscribe nack, packet-id=%u", packet_id);
        break;

    case ALIOT_MQTT_EVENT_PUBLISH_SUCCESS:
        ALIOT_LOG_INFO("publish success, packet-id=%u", packet_id);
        break;

    case ALIOT_MQTT_EVENT_PUBLISH_TIMEOUT:
        ALIOT_LOG_INFO("publish timeout, packet-id=%u", packet_id);
        break;

    case ALIOT_MQTT_EVENT_PUBLISH_NACK:
        ALIOT_LOG_INFO("publish nack, packet-id=%u", packet_id);
        break;

    case ALIOT_MQTT_EVENT_PUBLISH_RECVEIVED:
        ALIOT_LOG_INFO("Should NOT arrive here, topic=%.*s, topic_msg=%.*s",
                topic_info->topic_len,
                topic_info->ptopic,
                topic_info->payload_len,
                topic_info->payload);
        break;

    default:
        ALIOT_LOG_INFO("Should NOT arrive here.");
        break;
    }
}


void aliot_mqtt_msg_arrived(void *pcontext, void *pclient, aliot_mqtt_event_msg_pt msg)
{
    aliot_mqtt_topic_info_pt ptopic_info = (aliot_mqtt_topic_info_pt) msg->msg;

    //print topic name and topic message
    ALIOT_LOG_DEBUG("topic=%.*s, topic_msg=%.*s",
            ptopic_info->topic_len,
            ptopic_info->ptopic,
            ptopic_info->payload_len,
            ptopic_info->payload);
}

int mqtt_client(void)
{
    int rc = 0, ch = 0, msg_len, cnt = 0;
    void *pclient;
    aliot_user_info_pt puser_info;
    aliot_mqtt_param_t mqtt_params;
    aliot_mqtt_topic_info_t topic_msg;
    char msg_pub[128];
    char *msg_buf = NULL, *msg_readbuf = NULL;

    if (NULL == (msg_buf = (char *)aliot_platform_malloc(MSG_LEN_MAX))) {
        ALIOT_LOG_DEBUG("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)aliot_platform_malloc(MSG_LEN_MAX))) {
        ALIOT_LOG_DEBUG("not enough memory");
        rc = -1;
        goto do_exit;
    }

    /* Initialize device info */
    aliot_device_init();

    if (0 != aliot_set_device_info(PRODUCT_KEY, DEVICE_NAME, DEVICE_ID, DEVICE_SECRET)) {
        ALIOT_LOG_DEBUG("set device info failed!");
        rc = -1;
        goto do_exit;
    }

    /* Device AUTH */
    if (0 != aliot_auth(aliot_get_device_info(), aliot_get_user_info())) {
        ALIOT_LOG_DEBUG("AUTH request failed!");
        rc = -1;
        goto do_exit;
    }
    puser_info = aliot_get_user_info();

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
    mqtt_params.keepalive_interval = 120;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MSG_LEN_MAX;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MSG_LEN_MAX;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;


    /* Construct a MQTT client with specify parameter */
    pclient = aliot_mqtt_construct(&mqtt_params);
    if (NULL == pclient) {
        ALIOT_LOG_DEBUG("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }

    /* Subscribe the specific topic */
    rc = aliot_mqtt_subscribe(pclient, TOPIC_DATA, ALIOT_MQTT_QOS1, aliot_mqtt_msg_arrived, NULL);
    if (rc < 0) {
        aliot_mqtt_deconstruct(pclient);
        ALIOT_LOG_DEBUG("ali_iot_mqtt_subscribe failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }

    aliot_platform_msleep(1000);


    /* Initialize topic information */
    memset(&topic_msg, 0x0, sizeof(aliot_mqtt_topic_info_t));
    strcpy(msg_pub, "message: hello! start!");

    topic_msg.qos = ALIOT_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    do {
        /* Generate topic message */
        cnt++;
        msg_len = snprintf(msg_pub, sizeof(msg_pub), "message: hello, %d!", cnt);
        if (msg_len < 0) {
            ALIOT_LOG_DEBUG("Error occur! Exit program");
            rc = -1;
            break;
        }

        topic_msg.payload = (void *)msg_pub;
        topic_msg.payload_len = msg_len;

        rc = aliot_mqtt_publish(pclient, TOPIC_DATA, &topic_msg);
        if (rc < 0) {
            ALIOT_LOG_DEBUG("error occur when publish");
            rc = -1;
            break;
        } else {
            ALIOT_LOG_DEBUG("publish packet-id = %u", rc);
        }

        /* handle the MQTT packet received from TCP or SSL connection */
        aliot_mqtt_yield(pclient, 100);

        aliot_platform_msleep(1000);

    } while (ch != 'Q' && ch != 'q');

    aliot_mqtt_unsubscribe(pclient, TOPIC_DATA);

    aliot_platform_msleep(200);

    aliot_mqtt_deconstruct(pclient);


do_exit:
    if (NULL != msg_buf) {
        aliot_platform_free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        aliot_platform_free(msg_readbuf);
    }

    return rc;
}



int main()
{
    mqtt_client();

    ALIOT_LOG_DEBUG("out of sample!");

    return 0;
}


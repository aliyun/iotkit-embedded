
#include "aliot_platform.h"
#include "aliot_log.h"
#include "aliot_mqtt_client.h"
#include "aliot_auth.h"
#include "aliot_device.h"

//The product and device information from IOT console
/* online */
//#define PRODUCT_KEY         "OvNmiEYRDSY"
//#define DEVICE_NAME         "sh_online_sample_shadow"
//#define DEVICE_ID           "TyyB3SYLhpSaUKkWj33h"
//#define DEVICE_SECRET       "RcS3af0lHnpzNkfcVB1RKc4kSoR84D2n"

/* pre */
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


static char my_msg[MSG_LEN_MAX];

/**
 * @brief This is a callback function.
 *
 * @param [in] md: message pointer
 * @return none
 * @see none.
 * @note none.
 */
void event_handle(void *pcontext, void *pclient, aliot_mqtt_event_msg_pt msg)
{


    switch (msg->event_type)
    {
    case ALIOT_MQTT_EVENT_UNKNOWN:
        ALIOT_LOG_INFO("event unkown.");
        break;

    case ALIOT_MQTT_EVENT_DISCONNECT:
        ALIOT_LOG_INFO("disconnet.");
        break;

    case ALIOT_MQTT_EVENT_RECONNECT:
        ALIOT_LOG_INFO("reconnect.");
        break;

    case ALIOT_MQTT_EVENT_SUBCRIBE_SUCCESS:
        ALIOT_LOG_INFO("subscribe success.");
        break;

    case ALIOT_MQTT_EVENT_SUBCRIBE_TIMEOUT:
        ALIOT_LOG_INFO("subscribe wait ack timeout.");
        break;

    case ALIOT_MQTT_EVENT_SUBCRIBE_NACK:
        ALIOT_LOG_INFO("subscribe nack.");
        break;

    case ALIOT_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
        ALIOT_LOG_INFO("unsubscribe success.");
        break;

    case ALIOT_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
        ALIOT_LOG_INFO("unsubscribe timeout.");
        break;

    case ALIOT_MQTT_EVENT_UNSUBCRIBE_NACK:
        ALIOT_LOG_INFO("unsubscribe nack.");
        break;

    case ALIOT_MQTT_EVENT_PUBLISH_SUCCESS:
        ALIOT_LOG_INFO("publish success.");
        break;

    case ALIOT_MQTT_EVENT_PUBLISH_TIMEOUT:
        ALIOT_LOG_INFO("publish timeout.");
        break;

    case ALIOT_MQTT_EVENT_PUBLISH_NACK:
        ALIOT_LOG_INFO("publish nack.");
        break;

    case ALIOT_MQTT_EVENT_PUBLISH_RECVEIVED:
        ALIOT_LOG_INFO("Should NOT arrive here.");
        break;

    default:
        ALIOT_LOG_INFO("Should NOT arrive here.");
        break;
    }
}


void aliot_mqtt_msg_arrived(void *pcontext, void *pclient, aliot_mqtt_event_msg_pt msg)
{
    uint32_t msg_len;
    aliot_mqtt_topic_info_pt ppacket = (aliot_mqtt_topic_info_pt) msg->msg;

    if (ppacket->payload_len < MSG_LEN_MAX - 1) {
        msg_len = ppacket->payload_len;
    } else {
        ALIOT_LOG_INFO("message is too long to be stored, truncate it");
        msg_len = MSG_LEN_MAX - 1;
    }

    //copy the message to your buffer
    memcpy(my_msg, ppacket->payload, msg_len);
    //print the message
    ALIOT_LOG_DEBUG("msg = %s", my_msg);
}

int mqtt_client(char *msg_buf, char *msg_readbuf)
{
    int rc = 0, ch = 0, msg_len, cnt;
    void *pclient;
    aliot_user_info_pt puser_info;
    aliot_mqtt_param_t mqtt_params;
    char msg_pub[128];


    /* initialize device info */
    aliot_device_init();

    if (0 != aliot_set_device_info(PRODUCT_KEY, DEVICE_NAME, DEVICE_ID, DEVICE_SECRET)) {
        ALIOT_LOG_DEBUG("run aliot_set_device_info() error!");
        return -1;
    }

    /* Device AUTH */
    if (0 != aliot_auth(aliot_get_device_info(), aliot_get_user_info())) {
        ALIOT_LOG_DEBUG("run aliot_auth() error!");
        return -1;
    }

    puser_info = aliot_get_user_info();

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

    pclient = aliot_mqtt_construct(&mqtt_params);
    if (NULL == pclient) {
        ALIOT_LOG_DEBUG("MQTT construct failed");
        return rc;
    }

    rc = aliot_mqtt_subscribe(pclient, TOPIC_DATA, ALIOT_MQTT_QOS1, aliot_mqtt_msg_arrived, NULL);
    if (rc < 0) {
        aliot_mqtt_deconstruct(pclient);
        ALIOT_LOG_DEBUG("ali_iot_mqtt_subscribe failed, rc = %d", rc);
        return rc;
    }

    aliot_platform_msleep(1000);


    aliot_mqtt_topic_info_t topic_msg;
    memset(&topic_msg, 0x0, sizeof(aliot_mqtt_topic_info_t));
    strcpy(msg_pub, "message: hello! start!");

    topic_msg.qos = ALIOT_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    rc = aliot_mqtt_publish(pclient, TOPIC_DATA, &topic_msg);
    if (rc < 0) {
        aliot_mqtt_deconstruct(pclient);
        ALIOT_LOG_DEBUG("aliot_mqtt_publish failed ret = %d", rc);
        return rc;
    }

    cnt = 0;
    do {
        ++cnt;
        msg_len = snprintf(msg_pub, sizeof(msg_pub), "message: hello, %d!", cnt);
        if (msg_len < 0) {
            ALIOT_LOG_DEBUG("Error occur! Exit program");
            break;
        }

        topic_msg.payload = (void *)msg_pub;
        topic_msg.payload_len = msg_len;

        //handle the MQTT packet received from TCP or SSL connection
        aliot_mqtt_yield(pclient, 500);
        rc = aliot_mqtt_publish(pclient, TOPIC_DATA, &topic_msg);
    } while (ch != 'Q' && ch != 'q');

    aliot_mqtt_deconstruct(pclient);
    aliot_platform_msleep(200);
    return 0;
}



int main()
{
    char *msg_buf = (char *)aliot_platform_malloc(MSG_LEN_MAX);
    char *msg_readbuf = (char *)aliot_platform_malloc(MSG_LEN_MAX);

    mqtt_client(msg_buf, msg_readbuf);

    aliot_platform_free(msg_buf);
    aliot_platform_free(msg_readbuf);

    ALIOT_LOG_DEBUG("out of demo!");

    return 0;
}


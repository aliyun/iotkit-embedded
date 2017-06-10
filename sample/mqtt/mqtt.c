
#include "aliot_platform.h"
#include "aliot_log.h"
#include "aliot_mqtt_client.h"
#include "aliot_auth.h"
#include "aliot_device.h"

//The product and device information from IOT console
#define PRODUCT_KEY         "G6xVlhtyQFW"
#define DEVICE_NAME         "sh_xk_device_sample_mqtt"
#define DEVICE_ID           "UVUKrHPwSF0N6LfKvjo5"
#define DEVICE_SECRET       "65YgEXvgtXzLPnbW9xgTiCBddXGTkWlW"


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
static void messageArrived(MessageData *md)
{
    uint32_t msg_len;
    MQTTMessage *message = md->message;

    if (message->payloadlen < MSG_LEN_MAX - 1) {
        msg_len = message->payloadlen;
    } else {
        ALIOT_LOG_INFO("message is too long to be stored, truncate it");
        msg_len = MSG_LEN_MAX - 1;
    }

    //copy the message to your buffer
    memcpy(my_msg, message->payload, msg_len);



    /*
     * *********** Your Code ***********
     */



    //print the message
    ALIOT_LOG_DEBUG("msg = %s", my_msg);
}

int mqtt_client(unsigned char *msg_buf, unsigned char *msg_readbuf)
{
    int rc = 0, ch = 0, msg_len, cnt;
    MQTTClient_t client;
    aliot_mqtt_param_t initParams;
    char msg_pub[128];


    /* initialize device info */
    aliyun_iot_device_init();

    if (0 != aliyun_iot_set_device_info(PRODUCT_KEY, DEVICE_NAME, DEVICE_ID, DEVICE_SECRET)) {
        ALIOT_LOG_DEBUG("run aliyun_iot_set_device_info() error!");
        return -1;
    }


    /* Device AUTH */
    if (0 != aliyun_iot_auth(aliyun_iot_get_device_info(), aliyun_iot_get_user_info())) {
        ALIOT_LOG_DEBUG("run aliyun_iot_auth() error!");
        return -1;
    }


    memset(&client, 0x0, sizeof(client));
    memset(&initParams, 0x0, sizeof(initParams));

    initParams.mqttCommandTimeout_ms = 2000;
    initParams.pReadBuf = msg_readbuf;
    initParams.readBufSize = MSG_LEN_MAX;
    initParams.pWriteBuf = msg_buf;
    initParams.writeBufSize = MSG_LEN_MAX;
    initParams.disconnectHandler = NULL;
    initParams.disconnectHandlerData = (void *) &client;
    initParams.deliveryCompleteFun = NULL;
    initParams.subAckTimeOutFun = NULL;

    initParams.cleansession      = 0;
    initParams.MQTTVersion       = 4;
    initParams.keepAliveInterval = 180;
    initParams.willFlag          = 0;

    rc = aliyun_iot_mqtt_init(&client, &initParams, aliyun_iot_get_user_info());
    if (0 != rc) {
        ALIOT_LOG_DEBUG("aliyun_iot_mqtt_init failed ret = %d", rc);
        return rc;
    }

    rc = aliyun_iot_mqtt_connect(&client);
    if (0 != rc) {
        aliyun_iot_mqtt_release(&client);
        ALIOT_LOG_DEBUG("ali_iot_mqtt_connect failed ret = %d", rc);
        return rc;
    }

    rc = aliyun_iot_mqtt_subscribe(&client, TOPIC_DATA, QOS1, messageArrived);
    if (0 != rc) {
        aliyun_iot_mqtt_release(&client);
        ALIOT_LOG_DEBUG("ali_iot_mqtt_subscribe failed ret = %d", rc);
        return rc;
    }


    MQTTMessage message;
    memset(&message, 0x0, sizeof(message));
    strcpy(msg_pub, "message: hello! start!");

    message.qos        = QOS1;
    message.retained   = 0;
    message.dup        = 0;
    message.payload    = (void *)msg_pub;
    message.payloadlen = strlen(msg_pub);
    message.id         = 0;

    rc = aliyun_iot_mqtt_publish(&client, TOPIC_DATA, &message);
    if (SUCCESS_RETURN != rc) {
        aliyun_iot_mqtt_release(&client);
        ALIOT_LOG_DEBUG("aliyun_iot_mqtt_publish failed ret = %d", rc);
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

        message.payload = (void *)msg_pub;
        message.payloadlen = msg_len;

        //handle the MQTT packet received from TCP or SSL connection
        aliyun_iot_mqtt_yield(&client, 500);
        rc = aliyun_iot_mqtt_publish(&client, TOPIC_DATA, &message);
    } while (ch != 'Q' && ch != 'q');

    aliyun_iot_mqtt_release(&client);
    aliot_platform_msleep(200);
    return 0;
}



int main()
{
    unsigned char *msg_buf = (unsigned char *)aliot_platform_malloc(MSG_LEN_MAX);
    unsigned char *msg_readbuf = (unsigned char *)aliot_platform_malloc(MSG_LEN_MAX);

    mqtt_client(msg_buf, msg_readbuf);

    aliot_platform_free(msg_buf);
    aliot_platform_free(msg_readbuf);

    ALIOT_LOG_DEBUG("out of demo!");

    return 0;
}


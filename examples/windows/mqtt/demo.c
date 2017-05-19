#include "aliyun_iot_mqtt_client.h"
#include "aliyun_iot_auth.h"
#include "aliyun_iot_device.h"
#include "aliyun_iot_common_httpclient.h"
#include <windows.h>
#include <pthread.h>

#define HOST_NAME      "iot-as.aliyuncs.com"

//用户需要根据设备信息完善以下宏定义中的四元组内容
#define PRODUCT_KEY         "4eViBFJ2QGH"
#define DEVICE_NAME         "device_xikan_1"
#define DEVICE_ID           "mwYSQBQq0svKOUb1MZCx"
#define DEVICE_SECRET       "GkQjaEAZh7jZW4XIQ6LOT2yM9aOWAdZP"

//以下三个TOPIC的宏定义不需要用户修改，可以直接使用
//IOT HUB为设备建立三个TOPIC：update用于设备发布消息，error用于设备发布错误，get用于订阅消息
#define TOPIC_UPDATE         "/4eViBFJ2QGH/device_xikan_1/update"
#define TOPIC_ERROR          "/4eViBFJ2QGH/device_xikan_1/update/error"
#define TOPIC_GET            "/4eViBFJ2QGH/device_xikan_1/get"

#define MSG_LEN_MAX     (256)

/**********************************************************************
 *            接收消息的回调函数
 *
 * 说明：当其它设备的消息到达时，此函数将被执行。
 * 注意：此回调函数中用户在做业务处理时不要使用耗时操作，否则会阻塞接收通道
 **********************************************************************/
static void messageArrived(MessageData *md)
{
    //接收消息缓存
    char msg[MSG_LEN_MAX] = {0};

    MQTTMessage *message = md->message;
    if(message->payloadlen > MSG_LEN_MAX - 1)
    {
        printf("process part of receive message\n");
        message->payloadlen = MSG_LEN_MAX - 1;
    }

    //复制接收消息到本地缓存
    memcpy(msg,message->payload,message->payloadlen);

    //to-do此处可以增加用户自己业务逻辑，例如：开关灯等操作

    //打印接收消息
    printf("Message : %s\n", msg);
}

static void publishComplete(void* context, unsigned int msgId)
{
    printf("publish message is arrived,id = %d\n",msgId);
}

static void subAckTimeout(SUBSCRIBE_INFO_S *subInfo)
{
    printf("msgId = %d,sub ack is timeout\n",subInfo->msgId);
}


int singleThreadDemo(unsigned char *msg_buf,unsigned char *msg_readbuf)
{
    int rc = 0;
    char buf[MSG_LEN_MAX] = { 0 };


    aliyun_iot_device_init();

    if (0 != aliyun_iot_set_device_info(PRODUCT_KEY, DEVICE_NAME, DEVICE_ID, DEVICE_SECRET))
    {
        printf("run aliyun_iot_set_device_info() error!\n");
        return -1;
    }

    if (0 != aliyun_iot_auth(aliyun_iot_get_device_info(), aliyun_iot_get_user_info()))
    {
        printf("run aliyun_iot_auth() error!\n");
        return -1;
    }

    MQTTClient_t client;
    memset(&client,0x0,sizeof(client));
    aliot_mqtt_param_t initParams;
    memset(&initParams,0x0,sizeof(initParams));

    initParams.mqttCommandTimeout_ms = 2000;
    initParams.pReadBuf = msg_readbuf;
    initParams.readBufSize = MSG_LEN_MAX;
    initParams.pWriteBuf = msg_buf;
    initParams.writeBufSize = MSG_LEN_MAX;
    initParams.disconnectHandler = NULL;
    initParams.disconnectHandlerData = (void*) &client;
    initParams.deliveryCompleteFun = publishComplete;
    initParams.subAckTimeOutFun = subAckTimeout;

    initParams.cleansession      = 0;
    initParams.MQTTVersion       = 4;
    initParams.keepAliveInterval = 180;
    initParams.willFlag          = 0;

    rc = aliyun_iot_mqtt_init(&client, &initParams, aliyun_iot_get_user_info());
    if (0 != rc)
    {
        printf("aliyun_iot_mqtt_init failed ret = %d\n", rc);
        return rc;
    }

    rc = aliyun_iot_mqtt_connect(&client);
    if (0 != rc)
    {
        aliyun_iot_mqtt_release(&client);
        printf("ali_iot_mqtt_connect failed ret = %d\n", rc);
        return rc;
    }

    rc = aliyun_iot_mqtt_subscribe(&client, TOPIC_GET, QOS1, messageArrived);
    if (0 != rc)
    {
        aliyun_iot_mqtt_release(&client);
        printf("ali_iot_mqtt_subscribe failed ret = %d\n", rc);
        return rc;
    }

    do
    {
        Sleep(200);
        rc = aliyun_iot_mqtt_suback_sync(&client, TOPIC_GET, messageArrived);
    }while(rc != SUCCESS_RETURN);

    MQTTMessage message;
    memset(&message,0x0,sizeof(message));

    //发送字符串消息的内容
    //sprintf_s(buf, MSG_LEN_MAX,"{\"message\":\"Hello World\"}");
    strcpy(buf, "message: hello!");

    message.qos        = QOS1;
    message.retained   = FALSE_IOT;
    message.dup        = FALSE_IOT;
    message.payload    = (void *) buf;   //此处使用的负载是字符串文本数据，如果使用规则引擎请使用JSON格式的字符串
    message.payloadlen = strlen(buf);    //此处使用的负载长度是字符串长度且不包含结束标志'\0',如果负载是二进制数据，则负载长度需要用户准确指定
    message.id         = 0;

    //发送消息
    rc = aliyun_iot_mqtt_publish(&client, TOPIC_GET, &message);
    if (SUCCESS_RETURN != rc)
    {
        aliyun_iot_mqtt_release(&client);
        printf("aliyun_iot_mqtt_publish failed ret = %d\n", rc);
        return rc;
    }

    int ch = 0;
    do
    {
        //ch = getchar();
        Sleep(500);
        rc = aliyun_iot_mqtt_publish(&client, TOPIC_GET, &message);
    } while (ch != 'Q' && ch != 'q');

    aliyun_iot_mqtt_release(&client);
	Sleep(200);
    return 0;
}


//#define IOT_MQTT_SERVER_PUB_KEY

const char iot_auth_ca_crt[] = {
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n"
    "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n"
    "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n"
    "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n"
    "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n"
    "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n"
    "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n"
    "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n"
    "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n"
    "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n"
    "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n"
    "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n"
    "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n"
    "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n"
    "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n"
    "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n"
    "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n"
    "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n"
    "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n"
    "-----END CERTIFICATE-----"
};


int main()
{
//    httpclient_t client;
//    httpclient_data_t client_data;
//    memset(&client, 0, sizeof(httpclient_t));
//    memset(&client_data, 0, sizeof(httpclient_data_t));
//
//    char *response_buf = (char *) aliyun_iot_memory_malloc(HTTP_RESP_MAX_LEN);
//    if (NULL == response_buf) {
//        WRITE_IOT_ERROR_LOG("malloc http response buf failed!");
//        return CCP_MALLOC_ERROR;
//    }
//    memset(response_buf, 0, HTTP_RESP_MAX_LEN);
//
//    client_data.response_buf = response_buf;
//    client_data.response_buf_len = HTTP_RESP_MAX_LEN;
//
//    printf("start demo!\n");

    //aliyun_iot_common_post(&client, "https://iot-auth.alibaba.net/", 443, iot_auth_ca_crt, &client_data);
    //aliyun_iot_common_post(&client, "http://100.69.166.91/login", 8080, NULL, &client_data);

    //aliyun_iot_common_post(&client, "http://iot-auth-pre.cn-shanghai.aliyuncs.com/", 80, NULL, &client_data);




    unsigned char *msg_buf = (unsigned char *)malloc(MSG_LEN_MAX);
    unsigned char *msg_readbuf = (unsigned char *)malloc(MSG_LEN_MAX);

    //sdk初始化,初始化日志等
    if (0 != aliyun_iot_auth_init())
    {
        printf("run aliyun_iot_auth_init error!\n");
        return -1;
    }

    singleThreadDemo(msg_buf,msg_readbuf);

    free(msg_buf);
    free(msg_readbuf);

    (void) aliyun_iot_auth_release();

    printf("out of demo!\n");

    return 0;
}


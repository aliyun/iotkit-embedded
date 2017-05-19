
#include "aliyun_iot_mqtt_client.h"
#include "aliyun_iot_auth.h"
#include "threadx/tx_api.h"
#include "qcom/qcom_mem.h"

#define HOST_NAME      "iot-as.aliyuncs.com"

//用户需要根据设备信息完善以下宏定义中的四元组内容
#define PRODUCT_KEY    ""
#define PRODUCT_SECRET ""
#define DEVICE_NAME    ""
#define DEVICE_SECRET  ""

//以下三个TOPIC的宏定义不需要用户修改，可以直接使用
//IOT HUB为设备建立三个TOPIC：update用于设备发布消息，error用于设备发布错误，get用于订阅消息
#define TOPIC_UPDATE         "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR          "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET            "/"PRODUCT_KEY"/"DEVICE_NAME"/get"

#define MSG_LEN_MAX 100

/**Number of publish thread*/
#define MAX_PUBLISH_THREAD_COUNT 2


TX_THREAD publishThread[MAX_PUBLISH_THREAD_COUNT];


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

void * pubThread(void*param)
{
    char buf[MSG_LEN_MAX] = { 0 };
    static int num = 0;
    int rc = 0 ;
    MQTTClient_t * client = (MQTTClient_t*)param;
    MQTTMessage message;

    int msgId[5] = {0};

    static int threadID = 0;
    int id = threadID++;

    printf("start pub thread,id = %d\n",id);
    for(;;)
    {
        int i = 0;
        for(i = 0;i<5;i++)
        {
            memset(&message,0x0,sizeof(message));
            sprintf(buf, "{\"message\":\"Hello World! threadId = %d, num = %d\"}",id,num++);
            message.qos = QOS1;
            message.retained = FALSE_IOT;
            message.dup = FALSE_IOT;
            message.payload = (void *) buf;
            message.payloadlen = strlen(buf);
            message.id = 0;
            rc = aliyun_iot_mqtt_publish(client, TOPIC_UPDATE, &message);
            if (SUCCESS_RETURN != rc)
            {
                printf("ali_iot_mqtt_publish failed ret = %d\n", rc);
            }
            else
            {
                msgId[i] = message.id;
            }

            tx_thread_sleep(5000);
        }

        tx_thread_sleep(1000);
    }

    return NULL;
}

int multiThreadDemo(unsigned char *msg_buf,unsigned char *msg_readbuf)
{
    int rc = 0;
    memset(msg_buf,0x0,MSG_LEN_MAX);
    memset(msg_readbuf,0x0,MSG_LEN_MAX);

    //初始化设备参数
    IOT_DEVICEINFO_SHADOW_S deviceInfo;
    memset(&deviceInfo, 0x0, sizeof(deviceInfo));

    //设备信息初始化
    deviceInfo.productKey = PRODUCT_KEY;
    deviceInfo.productSecret = PRODUCT_SECRET;
    deviceInfo.deviceName = DEVICE_NAME;
    deviceInfo.deviceSecret = DEVICE_SECRET;
    deviceInfo.hostName = HOST_NAME;
    if (0 != aliyun_iot_set_device_info(&deviceInfo))
    {
        printf("run aliyun_iot_set_device_info() error!\n");
        return -1;
    }

    if (SUCCESS_RETURN != aliyun_iot_auth(MD5_SIGN_TYPE))
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
    rc = aliyun_iot_mqtt_init(&client, &initParams);
    if (0 != rc)
    {
        printf("ali_iot_mqtt_init failed ret = %d\n", rc);
        return rc;
    }

    MQTTPacket_connectData connectParam;
    memset(&connectParam,0x0,sizeof(connectParam));
    connectParam.cleansession = 0;
    connectParam.MQTTVersion = 4;
    connectParam.keepAliveInterval = 180;
    connectParam.willFlag = 0;
    rc = aliyun_iot_mqtt_connect(&client, &connectParam);
    if (0 != rc)
    {
        printf("ali_iot_mqtt_connect failed ret = %d\n", rc);
        return rc;
    }

    rc = aliyun_iot_mqtt_subscribe(&client, TOPIC_GET, QOS1, messageArrived);
    if (0 != rc)
    {
        printf("ali_iot_mqtt_subscribe failed ret = %d\n", rc);
        return rc;
    }

    do
    {
        tx_thread_sleep(10000);
        rc = aliyun_iot_mqtt_suback_sync(&client, TOPIC_GET, messageArrived);
    }while(rc != SUCCESS_RETURN);

	unsigned int iter = 0;
	for (iter = 0; iter < MAX_PUBLISH_THREAD_COUNT; iter++)
	{
		rc = tx_thread_create(&publishThread[iter], "threadx", pubThread, (ULONG)&client, 0, 2048, 16, 16, 4, TX_AUTO_START);
		if(0 != rc)
		{
			printf("create publish thread success failed");
		}
		else
		{
			printf("create publish thread success ");
		}
	}
	tx_thread_sleep(500000);

	int i = 0;
	for (i = 0; i < MAX_PUBLISH_THREAD_COUNT; i++)
	{
		tx_thread_delete(&publishThread[i]);
	}

	aliyun_iot_mqtt_release(&client);

    return 0;
}

int singleThreadDemo(unsigned char *msg_buf,unsigned char *msg_readbuf)
{
    int rc = 0;
    char buf[MSG_LEN_MAX] = { 0 };

    //初始化设备参数
    IOT_DEVICEINFO_SHADOW_S deviceInfo;
    memset(&deviceInfo, 0x0, sizeof(deviceInfo));

    //设备信息初始化
    deviceInfo.productKey = PRODUCT_KEY;
    deviceInfo.productSecret = PRODUCT_SECRET;
    deviceInfo.deviceName = DEVICE_NAME;
    deviceInfo.deviceSecret = DEVICE_SECRET;
    deviceInfo.hostName = HOST_NAME;

    //配置设备信息
    if (0 != aliyun_iot_set_device_info(&deviceInfo))
    {
        printf("run aliyun_iot_set_device_info() error!\n");
        return -1;
    }

    if (0 != aliyun_iot_auth(MD5_SIGN_TYPE))
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
    rc = aliyun_iot_mqtt_init(&client, &initParams);
    if (0 != rc)
    {
        printf("aliyun_iot_mqtt_init failed ret = %d\n", rc);
        return rc;
    }

    MQTTPacket_connectData connectParam;
    memset(&connectParam,0x0,sizeof(connectParam));
    connectParam.cleansession      = 0;
    connectParam.MQTTVersion       = 4;
    connectParam.keepAliveInterval = 180;
    connectParam.willFlag          = 0;

    rc = aliyun_iot_mqtt_connect(&client, &connectParam);
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
        tx_thread_sleep(2000);
        rc = aliyun_iot_mqtt_suback_sync(&client, TOPIC_GET, messageArrived);
    }while(rc != SUCCESS_RETURN);

    MQTTMessage message;
    memset(&message,0x0,sizeof(message));

    //发送字符串消息的内容
    sprintf(buf, "{\"message\":\"Hello World\"}");

    message.qos        = QOS1;
    message.retained   = FALSE_IOT;
    message.dup        = FALSE_IOT;
    message.payload    = (void *) buf;   //此处使用的负载是字符串文本数据，如果使用规则引擎请使用JSON格式的字符串
    message.payloadlen = strlen(buf);    //此处使用的负载长度是字符串长度且不包含结束标志'\0',如果负载是二进制数据，则负载长度需要用户准确指定
    message.id         = 0;

    //发送消息
    rc = aliyun_iot_mqtt_publish(&client, TOPIC_UPDATE, &message);
    if (SUCCESS_RETURN != rc)
    {
        aliyun_iot_mqtt_release(&client);
        printf("ali_iot_mqtt_publish failed ret = %d\n", rc);
        return rc;
    }

	tx_thread_sleep(500000);

    aliyun_iot_mqtt_release(&client);
    return 0;
}

void mqtt_client_demo(uint32_t param)
{
    printf("start demo!\n");

    unsigned char *msg_buf = (unsigned char *)mem_alloc(MSG_LEN_MAX);
    unsigned char *msg_readbuf = (unsigned char *)mem_alloc(MSG_LEN_MAX);

    //sdk初始化,初始化日志等
    if (0 != aliyun_iot_auth_init())
    {
        printf("run aliyun_iot_auth_init error!\n");
        return;
    }

    singleThreadDemo(msg_buf,msg_readbuf);
//    multiThreadDemo(msg_buf,msg_readbuf);

    mem_free(msg_buf);
    mem_free(msg_readbuf);

    aliyun_iot_auth_release();

    printf("out of demo!\n");
}

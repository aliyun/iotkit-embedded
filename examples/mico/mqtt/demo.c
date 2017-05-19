#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_auth.h"
#include "aliyun_iot_mqtt_client.h"
#include "mico_rtos.h"

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

/**Number of publish thread*/
#define MAX_PUBLISH_THREAD_COUNT 2

#define MSG_LEN_MAX 100
static int is_network_connected = 1;

//callback of publish
static void messageArrived(MessageData *md)
{
    char msg[MSG_LEN_MAX] = {0};

    MQTTMessage *message = md->message;
    if(message->payloadlen > MSG_LEN_MAX - 1)
    {
        WRITE_IOT_NOTICE_LOG("process part of receive message\n");
        message->payloadlen = MSG_LEN_MAX - 1;
    }

	memcpy(msg,message->payload,message->payloadlen);

	WRITE_IOT_NOTICE_LOG("Message : %s\n", msg);
}

static void publishComplete(void* context, unsigned int msgId)
{
    WRITE_IOT_NOTICE_LOG("publish message is arrived,id = %d\n",msgId);
}

static void subAckTimeout(SUBSCRIBE_INFO_S *subInfo)
{
    WRITE_IOT_NOTICE_LOG("msgId = %d,sub ack is timeout\n",subInfo->msgId);
}

static void MqttDisconnectHandler(MQTTClient_t *pClient, void *data)
{
	is_network_connected = 0;
	WRITE_IOT_NOTICE_LOG("mqtt network disconnected");
}

static void MqttReconnectHandler(MQTTClient_t *pClient, void *data)
{
	is_network_connected = 1;
	WRITE_IOT_NOTICE_LOG("mqtt network reconnected");
}

#ifndef MICO_SDK_V3
void  pubThread(void*param)
#else
void  pubThread(unsigned int param)
#endif

{
    char buf[100] = { 0 };
    static int num = 0;
    int rc = 0 ;
    MQTTClient_t * client = (MQTTClient_t*)param;
    MQTTMessage message;

    int msgId[5] = {0};

    static int threadID = 0;
    int id = threadID++;

    for(;;)
    {
        int i = 0;
        for(i = 0; i < 5; i++)
        {
        	if(is_network_connected)
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
	                WRITE_IOT_ERROR_LOG("ali_iot_mqtt_publish failed ret = %d\n", rc);
	            }
	            else
	            {
	                msgId[i] = message.id;
	            }
        	}
			else
        	{
        		WRITE_IOT_ERROR_LOG("network disconnected, stop publish");
        	}

            mico_thread_sleep(5);
        }

        mico_thread_sleep(1);
    }

    return;
}

int multiThreadDemo(unsigned char *msg_buf,unsigned char *msg_readbuf)
{
    int rc = 0;
    memset(msg_buf,0x0,100);
    memset(msg_readbuf,0x0,100);

    IOT_DEVICEINFO_SHADOW_S deviceInfo;
    memset(&deviceInfo, 0x0, sizeof(deviceInfo));

    deviceInfo.productKey = PRODUCT_KEY;
    deviceInfo.productSecret = PRODUCT_SECRET;
    deviceInfo.deviceName = DEVICE_NAME;
    deviceInfo.deviceSecret = DEVICE_SECRET;
    deviceInfo.hostName = HOST_NAME;
    if (0 != aliyun_iot_set_device_info(&deviceInfo))
    {
        WRITE_IOT_ERROR_LOG("run aliyun_iot_set_device_info() error!\n");
        return -1;
    }

    if (0 != aliyun_iot_auth(MD5_SIGN_TYPE))
    {
        WRITE_IOT_ERROR_LOG("run aliyun_iot_auth() error!\n");
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
    initParams.disconnectHandler = MqttDisconnectHandler;
    initParams.disconnectHandlerData = (void*) &client;
	initParams.reconnectHandler = MqttReconnectHandler;
	initParams.reconnectHandlerData = (void*) &client;
    initParams.deliveryCompleteFun = publishComplete;
    initParams.subAckTimeOutFun = subAckTimeout;
    rc = aliyun_iot_mqtt_init(&client, &initParams);
    if (0 != rc)
    {
        WRITE_IOT_ERROR_LOG("ali_iot_mqtt_init failed ret = %d\n", rc);
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
        WRITE_IOT_ERROR_LOG("ali_iot_mqtt_connect failed ret = %d\n", rc);
        return rc;
    }

    rc = aliyun_iot_mqtt_subscribe(&client, TOPIC_GET, QOS1, messageArrived);
    if (0 != rc)
    {
        WRITE_IOT_ERROR_LOG("ali_iot_mqtt_subscribe failed ret = %d\n", rc);
        return rc;
    }

    do
    {
        aliyun_iot_pthread_taskdelay(100);
        rc = aliyun_iot_mqtt_suback_sync(&client, TOPIC_GET, messageArrived);
    }while(rc != SUCCESS_RETURN);

	ALIYUN_IOT_PTHREAD_S publishThread[MAX_PUBLISH_THREAD_COUNT];

	unsigned iter = 0;
	for (iter = 0; iter < MAX_PUBLISH_THREAD_COUNT; iter++)
	{
	#ifndef MICO_SDK_V3
		rc = mico_rtos_create_thread(&publishThread[iter].micoThread, 1, "pub_thread", pubThread, 4096, &client);
	#else
		rc = mico_rtos_create_thread(&publishThread[iter].micoThread, 1, "pub_thread", pubThread, 4096, (unsigned int)&client);
	#endif
		if(0 != rc)
		{
			WRITE_IOT_ERROR_LOG("create publish thread failed ");
		}
		else
		{
			WRITE_IOT_NOTICE_LOG("create publish thread success id:%d", iter);
		}
	}


	int loopNum = 0;
	do
    {
    	mico_thread_sleep(5);
		loopNum ++ ;
    }while (loopNum < 50);

	int i = 0;
	for (i = 0; i < MAX_PUBLISH_THREAD_COUNT; i++)
	{
		mico_rtos_delete_thread(&publishThread[i].micoThread);
	}
	
    aliyun_iot_mqtt_release(&client);

    return 0;
}

int singleThread(unsigned char *msg_buf,unsigned char *msg_readbuf)
{
    int rc = 0;
    char buf[100] = { 0 };                        //application payload

    IOT_DEVICEINFO_SHADOW_S deviceInfo;
    memset(&deviceInfo, 0x0, sizeof(deviceInfo));

    deviceInfo.productKey = PRODUCT_KEY;
    deviceInfo.productSecret = PRODUCT_SECRET;
    deviceInfo.deviceName = DEVICE_NAME;
    deviceInfo.deviceSecret = DEVICE_SECRET;
    deviceInfo.hostName = HOST_NAME;
    if (0 != aliyun_iot_set_device_info(&deviceInfo))
    {
        WRITE_IOT_ERROR_LOG("run aliyun_iot_set_device_info() error!\n");
        return -1;
    }

    if (0 != aliyun_iot_auth(MD5_SIGN_TYPE))
    {
        WRITE_IOT_ERROR_LOG("run aliyun_iot_auth() error!\n");
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
    initParams.disconnectHandler = MqttDisconnectHandler;
    initParams.disconnectHandlerData = (void*) &client;
	initParams.reconnectHandler = MqttReconnectHandler;
	initParams.reconnectHandlerData = (void*) &client;
    initParams.deliveryCompleteFun = publishComplete;
    initParams.subAckTimeOutFun = subAckTimeout;
    rc = aliyun_iot_mqtt_init(&client, &initParams);
    if (0 != rc)
    {
        WRITE_IOT_ERROR_LOG("aliyun_iot_mqtt_init failed ret = %d\n", rc);
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
        WRITE_IOT_ERROR_LOG("ali_iot_mqtt_connect failed ret = %d\n", rc);
        return rc;
    }
	WRITE_IOT_NOTICE_LOG("mqtt connect success");

    rc = aliyun_iot_mqtt_subscribe(&client, TOPIC_GET, QOS1, messageArrived);
    if (0 != rc)
    {
        WRITE_IOT_ERROR_LOG("ali_iot_mqtt_subscribe failed ret = %d\n", rc);
        return rc;
    }
	WRITE_IOT_NOTICE_LOG("mqtt subscribe success");

    do
    {
        aliyun_iot_pthread_taskdelay(100);
        rc = aliyun_iot_mqtt_suback_sync(&client, TOPIC_GET, messageArrived);
    }while(rc != SUCCESS_RETURN);

    MQTTMessage message;
    memset(&message,0x0,sizeof(message));
    sprintf(buf, "{\"message\":\"Hello World\"}");
    message.qos = QOS1;
    message.retained = FALSE_IOT;
    message.dup = FALSE_IOT;
    message.payload = (void *) buf;
    message.payloadlen = strlen(buf);
    message.id = 0;
    rc = aliyun_iot_mqtt_publish(&client, TOPIC_UPDATE, &message);
    if (SUCCESS_RETURN != rc)
    {
        WRITE_IOT_ERROR_LOG("ali_iot_mqtt_publish failed ret = %d\n", rc);
        return rc;
    }
	WRITE_IOT_NOTICE_LOG("mqtt publish success");

	int loopNum = 0;
	do
    {
    	mico_thread_sleep(5);
		loopNum ++ ;
    }while (loopNum < 50);

    aliyun_iot_mqtt_release(&client);
	
	WRITE_IOT_NOTICE_LOG("mqtt release success");

    return 0;
}

int mqtt_client_demo()
{
    WRITE_IOT_NOTICE_LOG("start demo!\n");

    if (0 != aliyun_iot_auth_init())
    {
        WRITE_IOT_ERROR_LOG("run aliyun_iot_auth_init error!\n");
        return -1;
    }

    unsigned char *msg_buf = (unsigned char *)malloc(MSG_LEN_MAX);
    unsigned char *msg_readbuf = (unsigned char *)malloc(MSG_LEN_MAX);

    singleThread(msg_buf,msg_readbuf);
    //multiThreadDemo(msg_buf,msg_readbuf);

    free(msg_buf);
    free(msg_readbuf);

    (void) aliyun_iot_auth_release();

    WRITE_IOT_NOTICE_LOG("out of demo!\n");

    return 0;
}


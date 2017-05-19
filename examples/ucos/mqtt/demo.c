#include <stdio.h>

#include "../../../platform/os/mico/inc/aliyun_iot_platform_memory.h"
#include "../../../platform/os/mico/inc/aliyun_iot_platform_pthread.h"
#include "aliyun_iot_mqtt_common.h"
#include "aliyun_iot_mqtt_client.h"
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_auth.h"

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


//callback of publish
static void messageArrived(MessageData *md)
{
    char msg[MSG_LEN_MAX] = {0};

    MQTTMessage *message = md->message;
    if(message->payloadlen > MSG_LEN_MAX - 1)
    {
        printf("process part of receive message\n");
        message->payloadlen = MSG_LEN_MAX - 1;
    }

	memcpy(msg,message->payload,message->payloadlen);
	
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

    for(;;)
    {
        int i = 0;
        for(i = 0; i < 5; i++)
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
            if (0 != rc)
            {
                printf("ali_iot_mqtt_publish failed ret = %d\n", rc);
            }
            else
            {
                msgId[i] = message.id;
            }

            aliyun_iot_pthread_taskdelay(5000);
        }

        aliyun_iot_pthread_taskdelay(1000);
    }

    return NULL;
}

int multiThreadDemo(unsigned char *msg_buf,unsigned char *msg_readbuf)
{
    int rc = 0;
    memset(msg_buf,0x0,MSG_LEN_MAX);
    memset(msg_readbuf,0x0,MSG_LEN_MAX);

    IOT_DEVICEINFO_SHADOW_S deviceInfo;
    memset(&deviceInfo, 0x0, sizeof(deviceInfo));

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
        aliyun_iot_pthread_taskdelay(100);
        rc = aliyun_iot_mqtt_suback_sync(&client, TOPIC_GET, messageArrived);
    }while(rc != SUCCESS_RETURN);

	ALIYUN_IOT_PTHREAD_S publishThread[MAX_PUBLISH_THREAD_COUNT];
	unsigned iter = 0;
	for (iter = 0; iter < MAX_PUBLISH_THREAD_COUNT; iter++)
	{
		rc = aliyun_iot_pthread_create(&publishThread[iter], pubThread, &client, NULL);
		if(1 == rc)
		{
			printf("create publish thread success ");
		}
		else
		{
			printf("create publish thread success failed");
		}
	}

    int32_t ch;
    do
    {
        ch = getchar();
        aliyun_iot_pthread_taskdelay(100);
    } while (ch != 'Q' && ch != 'q');

	int i = 0;
	for (i = 0; i < MAX_PUBLISH_THREAD_COUNT; i++)
	{
		aliyun_iot_pthread_cancel(&publishThread[i]);
	}

    aliyun_iot_mqtt_release(&client);

    return 0;
}

int singleThreadDemo(unsigned char *msg_buf,unsigned char *msg_readbuf)
{
    int rc = 0;
    char buf[MSG_LEN_MAX] = { 0 };

    IOT_DEVICEINFO_SHADOW_S deviceInfo;
    memset(&deviceInfo, 0x0, sizeof(deviceInfo));

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
    connectParam.cleansession = 0;
    connectParam.MQTTVersion = 4;
    connectParam.keepAliveInterval = 180;
    connectParam.willFlag = 0;

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
    if (0 != rc)
    {
        aliyun_iot_mqtt_release(&client);
        printf("ali_iot_mqtt_publish failed ret = %d\n", rc);
        return rc;
    }

    int32_t ch;
    do
    {
        ch = getchar();
        aliyun_iot_pthread_taskdelay(100);
    } while (ch != 'Q' && ch != 'q');

    aliyun_iot_mqtt_release(&client);

    aliyun_iot_pthread_taskdelay(10000);
    return 0;
}

int mqtt_client_demo()
{
    printf("start demo!\n");

    unsigned char *msg_buf = (unsigned char *)aliyun_iot_memory_malloc(MSG_LEN_MAX);
    unsigned char *msg_readbuf = (unsigned char *)aliyun_iot_memory_malloc(MSG_LEN_MAX);

    if (0 != aliyun_iot_auth_init())
    {
        printf("run aliyun_iot_auth_init error!\n");
        return -1;
    }

    singleThreadDemo(msg_buf,msg_readbuf);
//    multiThreadDemo(msg_buf,msg_readbuf);

    aliyun_iot_memory_free(msg_buf);
    aliyun_iot_memory_free(msg_readbuf);

    (void) aliyun_iot_auth_release();

    printf("out of demo!\n");

    return 0;
}


/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/
/*********************************************************************************
 * 文件名称: aliyun_iot_mqtt_client.h
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 说       明: iot sdk中mqtt协议的用户接口
 * 历       史:
 **********************************************************************************/
#ifndef ALIYUN_IOT_MQTT_CLIENT_H
#define ALIYUN_IOT_MQTT_CLIENT_H

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

#include <stdio.h>

#include "aliyun_iot_common_net.h"
#include "aliyun_iot_common_list.h"
#include "aliyun_iot_common_timer.h"
#include "aliyun_iot_platform_pthread.h"
#include "aliyun_iot_platform_sem.h"
#include "aliyun_iot_device.h"
#include "MQTTPacket/MQTTPacket.h"

#define TOPIC_NAME_MAX_LEN 64

#define MAX_PACKET_ID 65535
#define MAX_MESSAGE_HANDLERS 5
#define MQTT_TOPIC_LEN_MAX 128
#define MQTT_REPUB_NUM_MAX 20
#define MQTT_SUB_NUM_MAX   10

#define PINGRSP_TIMEOUT_MS           5000
#define MQTT_RECV_THREAD_STACK_SIZE         8192 //unit Kbyte
#define MQTT_KEEPALIVE_THREAD_STACK_SIZE    4096 //unit Kbyte
#define MQTT_RETRANS_THREAD_STACK_SIZE      4096 //unit Kbyte

/*******************************************
 * MQTT自动重连的最小周期时间，单位ms
*******************************************/
#define ALI_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL 1000

/*******************************************
 * MQTT自动重连的最大周期时间，单位ms
*******************************************/
#define ALI_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL 60000


/*******************************************
 * MQTT消息发送最小超时时间，单位ms
*******************************************/
#define ALI_IOT_MQTT_MIN_COMMAND_TIMEOUT    500

/*******************************************
 * MQTT消息发送最大超时时间，单位ms
*******************************************/
#define ALI_IOT_MQTT_MAX_COMMAND_TIMEOUT    5000

/*******************************************
 * MQTT消息发送最大超时时间，单位ms
*******************************************/
#define ALI_IOT_MQTT_DEFAULT_COMMAND_TIMEOUT 2000

/*******************************************
 * mqtt消息的QOS的类型
*******************************************/
enum QoS { QOS0, QOS1, QOS2 };


typedef struct Client MQTTClient_t;
typedef struct SUBSCRIBE_INFO SUBSCRIBE_INFO_S;

typedef struct Timer {
    aliot_time_t end_time;
}aliot_timer_t, *aliot_timer_pt;


/*******************************************
 * 节点状态标记
*******************************************/
typedef enum MQTT_NODE_STATE
{
    NODE_NORMANL_STATE = 0,
    NODE_INVALID_STATE,
}MQTT_NODE_STATE_E;

/*******************************************
 * mqtt消息结构体
*******************************************/
typedef struct MQTTMessage
{
    enum QoS qos;          //QOS类型，即mqtt服务等级，当前支持QOS0和QOS1
    char retained;         //publish报文保留标志
    char dup;              //控制报文重复分发标志
    unsigned short id;     //报文标识符
    void *payload;         //报文负载数据,注意：负载数据可以使用二进制数据或字符文本数据
    size_t payloadlen;     //报文负载数据长度,注意：使用字符文本数据时payloadlen是字符串长度不包含结束符'\0'标志，二进制数据的长度需要用户准确指定.
}MQTTMessage;


/*******************************************
 * 消息处理回调函数数据结构
*******************************************/
typedef struct MessageData
{
    MQTTMessage* message;   //mqtt消息结构体
    MQTTString* topicName;  //订阅的主题topic
}MessageData;

/*******************************************
 * mqtt消息处理回调函数
*******************************************/
typedef void (*messageHandler)(MessageData*);

/*******************************************
 * mqtt client状态类型
*******************************************/
typedef enum
{
	CLIENT_STATE_INVALID = 0,                    //client无效状态
	CLIENT_STATE_INITIALIZED = 1,                //client初始化状态
	CLIENT_STATE_CONNECTED = 2,                  //client已经连接状态
	CLIENT_STATE_DISCONNECTED = 3,               //client连接丢失状态
	CLIENT_STATE_DISCONNECTED_RECONNECTING = 4,  //client正在重连状态
} MQTTClientState;

/*******************************************
 * 网络事件类型（用于网络底层事件回调）
*******************************************/
typedef enum
{
    ALI_IOT_NETWORK_CONNECTED = 0,          //网络连接
    ALI_IOT_NETWORK_DISCONNECTED = 1,       //网络失去连接
    ALI_IOT_NETWORK_LINK_LOSS = 2,		    //网络连接丢失
    ALI_IOT_NETWORK_MAX_NUMBER              //网络连接最大数
} ALIYUN_IOT_NETWORK_E;

/*******************************************
 * 网络连接丢失回调处理函数
*******************************************/
typedef void (*iot_disconnect_handler)(MQTTClient_t *, void *);

/*******************************************
 * 网络连接丢失回调处理函数
*******************************************/
typedef void (*iot_reconnect_handler)(MQTTClient_t *, void *);

/*******************************************
 * 消息发送完成回调函数（收到消息ACK）
*******************************************/
typedef void DeliveryComplete(void* context, unsigned int msgId);

/*******************************************
 * 订阅消息超时回调函数（没有收到消息ACK）
*******************************************/
typedef void SubAckTimeoutHandler(SUBSCRIBE_INFO_S *);


/*******************************************
 * mqtt client的初始化数据
*******************************************/
typedef struct
{
	unsigned int           mqttCommandTimeout_ms;	//MQTT消息传输超时时间，时间内阻塞传输。单位：毫秒
	iot_disconnect_handler disconnectHandler;	    //网络连接丢失回调处理函数，不使用置NULL
	iot_reconnect_handler  reconnectHandler;        //网络连接恢复回调处理函数，不使用置NULL
	void                   *disconnectHandlerData;	//网络连接丢失回调处理函数的入参
	void                   *reconnectHandlerData;   //网络连接恢复回调处理函数的入参
	unsigned char          *pWriteBuf;		        //MQTT发送buffer
	unsigned int           writeBufSize;            //MQTT发送buffer的长度,
	unsigned char          *pReadBuf; 	            //MQTT接收buffer
	unsigned int           readBufSize;             //MQTT接收buffer的长度
	DeliveryComplete       *deliveryCompleteFun;    //MQTT传输完成回调函数，不使用置NULL
	SubAckTimeoutHandler   *subAckTimeOutFun;       //MQTTSub和unSub消息ACK超时的回调

	unsigned char MQTTVersion;
    unsigned short keepAliveInterval; //单位s
    unsigned char cleansession;
    unsigned char willFlag;
    MQTTPacket_willOptions will;
} aliot_mqtt_param_t, *aliot_mqtt_param_pt;


/*******************************************
 * mqtt client网络重连参数
*******************************************/
typedef struct
{
	aliot_timer_t          reconnectDelayTimer;	            //重连定时器，判断是否到重连时间
	iot_disconnect_handler disconnectHandler;               //网络连接丢失回调处理函数
	iot_reconnect_handler  reconnectHandler;                //网络连接恢复回调处理函数
	void                   *disconnectHandlerData;	        //网络连接丢失回调处理函数的入参
	void                   *reconnectHandlerData;           //网络连接恢复回调处理函数的入参
	bool                   isAutoReconnectEnabled;	        //自动重连标志
	unsigned int           currentReconnectWaitInterval;	//网络重连时间周期，单位ms
}aliot_mqtt_reconnect_param_t;



/*******************************************
 * 订阅主题对应的消息处理结构
*******************************************/
typedef struct MessageHandlers
{
    const char* topicFilter;      //pub消息对应的主题
    void (*fp) (MessageData*);    //pub消息处理函数
}MessageHandlers;

/*******************************************
 *         sub消息的信息记录
*******************************************/
struct SUBSCRIBE_INFO
{
    enum msgTypes    type;                               //sub消息类型（sub or unsub）
    unsigned int     msgId;                              //sub报文标识符
    aliot_timer_t    subTime;                            //sub消息时间
    MQTT_NODE_STATE_E nodeState;                         //node状态
    MessageHandlers  handler;
    int              len;                                //sub消息长度
    unsigned char*   buf;                                //sub消息体
};

/*******************************************
 * 网络恢复信号
*******************************************/
typedef struct NETWORK_RECOVER_CALLBACK_SIGNAL
{
    int                   signal;      //信号标志
    ALIYUN_IOT_MUTEX_S    signalLock;  //信号锁
}NETWORK_RECOVER_CALLBACK_SIGNAL_S;

/*******************************************
 * MQTT CLIENT数据结构
*******************************************/
struct Client
{
    unsigned int                      next_packetid;                           //MQTT报文标识符
    ALIYUN_IOT_MUTEX_S                idLock;                                  //MQTT报文标志符锁
    unsigned int                      command_timeout_ms;                      //MQTT消息传输超时时间，时间内阻塞传输。单位：毫秒
    int                               threadRunning;                           //client的线程运行标志
    size_t                            buf_size, readbuf_size;                  //MQTT消息发送接收buffer的大小
    unsigned char                     *buf;                                    //MQTT消息发送buffer
    unsigned char                     *readbuf;                                //MQTT消息接收buffer
    MessageHandlers                   messageHandlers[MAX_MESSAGE_HANDLERS];   //订阅主题对应的消息处理结构数组
    void (*defaultMessageHandler)     (MessageData*);                          //主题默认消息处理结构
    DeliveryComplete                  *deliveryCompleteFun;                    //MQTT消息传输完成回调函数（收到消息ACK）
    pNetwork_t                        ipstack;                                 //MQTT使用的网络参数
    aliot_timer_t                             ping_timer;                              //MQTT保活定时器，时间未到不做保活包发送
    int                               pingMark;                                //ping消息发送标志
    ALIYUN_IOT_MUTEX_S                pingMarkLock;                            //ping消息发送标志锁
    MQTTClientState                   clientState;                             //MQTT client状态
    ALIYUN_IOT_MUTEX_S                stateLock;                               //MQTT client状态锁
    aliot_mqtt_reconnect_param_t       reconnectparams;                         //MQTT client重连参数
	MQTTPacket_connectData            connectdata;                             //MQTT重连报文数据
	list_t *                          pubInfoList;                             //publish消息信息链表
	list_t *                          subInfoList;                             //subscribe、unsubscribe消息信息链表
	ALIYUN_IOT_MUTEX_S                pubInfoLock;                             //publish消息信息数组锁
	ALIYUN_IOT_MUTEX_S                subInfoLock;                             //subscribe消息信息数组锁
	ALIYUN_IOT_PTHREAD_S              recieveThread;                           //MQTT接收消息线程
	ALIYUN_IOT_PTHREAD_S              keepaliveThread;                         //MQTT client保活线程
	ALIYUN_IOT_PTHREAD_S              retransThread;                           //MQTT pub消息重发线程
	ALIYUN_IOT_MUTEX_S                writebufLock;                            //MQTT消息发送buffer锁
	NETWORK_RECOVER_CALLBACK_SIGNAL_S networkRecoverSignal;                    //网络恢复信号
	aliot_platform_sem_t                  semaphore;                               //接收消息同步信号
	SubAckTimeoutHandler              *subAckTimeOutFun;                       //sub或unsub消息ack超时回调
};

/***********************************************************
* 函数名称: aliyun_iot_mqtt_init
* 描       述: mqtt初始化
* 输入参数: Client *pClient
*          IOT_CLIENT_INIT_PARAMS *pInitParams
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: 用户创建Client结构，此接口根据用户的初始化参数初始化Client结构
************************************************************/
int aliyun_iot_mqtt_init(MQTTClient_t *pClient,
        aliot_mqtt_param_t *pInitParams,
        aliot_user_info_pt puser_info);

/***********************************************************
* 函数名称: aliyun_iot_mqtt_release
* 描       述: mqtt释放
* 输入参数: Client *pClient
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: 释放mqtt初始化时创建的资源
************************************************************/
int aliyun_iot_mqtt_release(MQTTClient_t *pClient);

/***********************************************************
* 函数名称: aliyun_iot_mqtt_connect
* 描       述: 连接mqtt协议
* 输入参数: Client *pClient
*           MQTTPacket_connectData* pConnectParams 连接报文参数
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: 实现网络connect和mqtt协议的connect
*          此接口是同步CONNACK的阻塞接口，超时退出
************************************************************/
int aliyun_iot_mqtt_connect(MQTTClient_t* pClient);

/***********************************************************
* 函数名称: aliyun_iot_mqtt_disconnect
* 描       述: 断开mqtt协议连接
* 输入参数: Client *pClient
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: mqtt协议的disconnect和网络的disconnect
*          注意执行阶段调用此接口会触发自动重连
************************************************************/
int aliyun_iot_mqtt_disconnect(MQTTClient_t* pClient);

/***********************************************************
* 函数名称: aliyun_iot_mqtt_subscribe
* 描       述: mqtt订阅消息
* 输入参数: Client* c  mqtt客户端
*          char* topicFilter 订阅的主题规则
*          enum QoS qos 消息服务类型
*          messageHandler messageHandler 消息处理结构
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: MQTT订阅操作，订阅成功，即收到sub ack
************************************************************/
int aliyun_iot_mqtt_subscribe(MQTTClient_t* c, const char* topicFilter, enum QoS qos, messageHandler messageHandler);

/***********************************************************
* 函数名称: aliyun_iot_mqtt_unsubscribe
* 描       述: mqtt取消订阅
* 输入参数: Client* c  mqtt客户端
*          char* topicFilter 订阅的主题规则
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: 同subscribe
************************************************************/
int aliyun_iot_mqtt_unsubscribe(MQTTClient_t* c, const char* topicFilter);

/***********************************************************
* 函数名称: aliyun_iot_mqtt_sub_sync
* 描       述: mqtt同步订阅ACK
* 输入参数: Client* c  mqtt客户端
*          char* topicFilter 订阅的主题规则
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: 同subscribe
************************************************************/
int aliyun_iot_mqtt_suback_sync(MQTTClient_t* c,const char* topicFilter,messageHandler messageHandler);

/***********************************************************
* 函数名称: aliyun_iot_mqtt_publish
* 描       述: mqtt发布消息
* 输入参数: Client* c  mqtt客户端
*          char* topicName 发布消息的主题
*          MQTTMessage* message 发布的消息体
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: MQTT发布消息操作，发布成功，即收到pub ack
************************************************************/
aliot_err_t aliyun_iot_mqtt_publish(MQTTClient_t* c, const char* topicName, MQTTMessage* message);


void aliyun_iot_mqtt_yield(MQTTClient_t* pClient, int timeout_ms);

//check whether MQTT connection is established or not.
bool aliyun_iot_mqtt_is_connected(MQTTClient_t *pClient);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif

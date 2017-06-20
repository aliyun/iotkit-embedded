
#include <stdlib.h>
#include <stddef.h>
#include "aliot_platform.h"
#include "aliot_auth.h"
#include "aliot_error.h"
#include "aliot_log.h"
#include "aliot_net.h"
#include "aliot_list.h"
#include "aliot_timer.h"

#include "MQTTPacket/MQTTPacket.h"
#include "aliot_mqtt_client.h"


//amc, aliot MQTT client

#define AMC_MQTT_VERSION        (4)
#define TOPIC_NAME_MAX_LEN      (64)

#define MAX_PACKET_ID           (65535)
#define MAX_MESSAGE_HANDLERS    (5)
#define MQTT_TOPIC_LEN_MAX      (128)
#define MQTT_REPUB_NUM_MAX      (20)
#define MQTT_SUB_NUM_MAX        (10)

#define PINGRSP_TIMEOUT_MS      (5000)

/*******************************************
 * MQTT自动重连的最小周期时间，单位ms
*******************************************/
#define ALIOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL  (1000)

/*******************************************
 * MQTT自动重连的最大周期时间，单位ms
*******************************************/
#define ALIOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL  (60000)


/*******************************************
 * MQTT消息发送最小超时时间，单位ms
*******************************************/
#define ALIOT_MQTT_MIN_COMMAND_TIMEOUT          (500)

/*******************************************
 * MQTT消息发送最大超时时间，单位ms
*******************************************/
#define ALIOT_MQTT_MAX_COMMAND_TIMEOUT          (5000)

/*******************************************
 * MQTT消息发送最大超时时间，单位ms
*******************************************/
#define ALIOT_MQTT_DEFAULT_COMMAND_TIMEOUT      (2000)


typedef enum {
    AMC_CONNECTION_ACCEPTED = 0,
    AMC_CONNECTION_REFUSED_UNACCEPTABLE_PROTOCOL_VERSION = 1,
    AMC_CONNECTION_REFUSED_IDENTIFIER_REJECTED = 2,
    AMC_CONNECTION_REFUSED_SERVER_UNAVAILABLE = 3,
    AMC_CONNECTION_REFUSED_BAD_USERDATA = 4,
    AMC_CONNECTION_REFUSED_NOT_AUTHORIZED = 5
} amc_connect_ack_code_t;


/*******************************************
 * mqtt client状态类型
*******************************************/
typedef enum {
    AMC_STATE_INVALID = 0,                    //client无效状态
    AMC_STATE_INITIALIZED = 1,                //client初始化状态
    AMC_STATE_CONNECTED = 2,                  //client已经连接状态
    AMC_STATE_DISCONNECTED = 3,               //client连接丢失状态
    AMC_STATE_DISCONNECTED_RECONNECTING = 4,  //client正在重连状态
} amc_state_t;


/*******************************************
 * 网络事件类型（用于网络底层事件回调）
*******************************************/
typedef enum {
    AMC_NETWORK_CONNECTED = 0,          //网络连接
    AMC_NETWORK_DISCONNECTED = 1,       //网络失去连接
    AMC_NETWORK_LINK_LOSS = 2,          //网络连接丢失
    AMC_NETWORK_MAX_NUMBER              //网络连接最大数
} amc_network_state_t;


/*******************************************
 * 节点状态标记
*******************************************/
typedef enum MQTT_NODE_STATE {
    AMC_NODE_NORMANL_STATE = 0,
    AMC_NODE_INVALID_STATE,
} amc_node_t;


/*******************************************
 * 订阅主题对应的消息处理结构
*******************************************/
typedef struct {
    const char *topicFilter;
    aliot_mqtt_event_handle_t handle;
} amc_topic_handle_t;


/*******************************************
 *         sub消息的信息记录
*******************************************/
typedef struct SUBSCRIBE_INFO {
    enum msgTypes     type;                               //sub消息类型（sub or unsub）
    unsigned int      msgId;                              //sub报文标识符
    aliot_time_t      subTime;                            //sub消息时间
    amc_node_t        nodeState;                          //node状态
    amc_topic_handle_t   handler;
    int               len;                                //sub消息长度
    unsigned char     *buf;                               //sub消息体
}amc_subsribe_info_t, *amc_subsribe_info_pt;


typedef struct REPUBLISH_INFO {
    aliot_time_t      pubTime;      //pub消息的时间
    amc_node_t        nodeState;    //node状态
    uint16_t          msgId;        //pub消息的报文标识符
    uint32_t          len;          //pub消息长度
    char              *buf;         //pub消息体
} amc_pub_info_t, *amc_pub_info_pt;


/*******************************************
 * mqtt client网络重连参数
*******************************************/
typedef struct {
    aliot_time_t       reconnectDelayTimer;             //重连定时器，判断是否到重连时间
    bool               isAutoReconnectEnabled;          //自动重连标志
    uint32_t           currentReconnectWaitInterval;    //网络重连时间周期，单位ms
} amc_reconnect_param_t;


/*******************************************
 * MQTT CLIENT数据结构
*******************************************/
typedef struct Client {
    void *                          lock_generic;                            //MQTT报文标志符锁
    uint32_t                        next_packetid;                           //MQTT报文标识符
    uint32_t                        command_timeout_ms;                      //MQTT消息传输超时时间，时间内阻塞传输。单位：毫秒
    uint32_t                        buf_size_send;                           //MQTT消息发送buffer的大小
    uint32_t                        buf_size_read;                           //MQTT消息接收buffer的大小
    char                            *buf_send;                               //MQTT消息发送buffer
    char                            *buf_read;                               //MQTT消息接收buffer
    amc_topic_handle_t              messageHandlers[MAX_MESSAGE_HANDLERS];   //订阅主题对应的消息处理结构数组
    aliot_network_pt                      ipstack;                                 //MQTT使用的网络参数
    aliot_time_t                    ping_timer;                              //MQTT保活定时器，时间未到不做保活包发送
    int                             pingMark;                                //ping消息发送标志
    void *                          pingMarkLock;                            //ping消息发送标志锁
    amc_state_t                     clientState;                             //MQTT client状态
    void *                          stateLock;                               //MQTT client状态锁
    amc_reconnect_param_t           reconnect_params;                         //MQTT client重连参数
    MQTTPacket_connectData          connectdata;                             //MQTT重连报文数据
    list_t                          *pubInfoList;                            //publish消息信息链表
    list_t                          *subInfoList;                            //subscribe、unsubscribe消息信息链表
    void *                          pubInfoLock;                             //publish消息信息数组锁
    void *                          subInfoLock;                             //subscribe消息信息数组锁
    void *                          writebufLock;                            //MQTT消息发送buffer锁

    aliot_mqtt_event_handle_t       handle_event;                            //event handle
}amc_client_t, *amc_client_pt;

static int amc_send_packet(amc_client_t *c, char *buf, int length, aliot_time_t *timer);
static amc_state_t amc_get_client_state(amc_client_t *pClient);
static void amc_set_client_state(amc_client_t *pClient, amc_state_t newState);
static int amc_keepalive_sub(amc_client_t *pClient);
static void amc_disconnect_callback(amc_client_t *pClient) ;
static bool amc_check_state_normal(amc_client_t *c);
static int amc_handle_reconnect(amc_client_t *pClient);
static void amc_reconnect_callback(amc_client_t *pClient);
static int amc_push_pubInfo_to(amc_client_t *c, int len, unsigned short msgId, list_node_t **node);
static int amc_push_subInfo_to(amc_client_t *c, int len, unsigned short msgId, enum msgTypes type, amc_topic_handle_t *handler,
                    list_node_t **node);
static int amc_check_handle_is_identical(amc_topic_handle_t *messageHandlers1, amc_topic_handle_t *messageHandler2);


typedef enum ALIYUN_IOT_TOPIC_TYPE {
    TOPIC_NAME_TYPE = 0,
    TOPIC_FILTER_TYPE
} ALIYUN_IOT_TOPIC_TYPE_E;

static int amc_check_rule(char *iterm, ALIYUN_IOT_TOPIC_TYPE_E type)
{
    if (NULL == iterm) {
        ALIOT_LOG_ERROR("iterm is NULL");
        return FAIL_RETURN;
    }

    int i = 0;
    int len = strlen(iterm);
    for (i = 0; i < len; i++) {
        if (TOPIC_FILTER_TYPE == type) {
            if ('+' == iterm[i] || '#' == iterm[i]) {
                if (1 != len) {
                    ALIOT_LOG_ERROR("the character # and + is error");
                    return FAIL_RETURN;
                }
            }
        } else {
            if ('+' == iterm[i] || '#' == iterm[i]) {
                ALIOT_LOG_ERROR("has character # and + is error");
                return FAIL_RETURN;
            }
        }

        if (iterm[i] < 32 || iterm[i] >= 127) {
            return FAIL_RETURN;
        }
    }
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliot_check_topic
* 描       述: topic校验
* 输入参数: const char * topicName
*          ALIYUN_IOT_TOPIC_TYPE_E type 校验类型
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: topicname校验时不允许有+，#符号
*           topicfilter校验时+，#允许存在但必须是单独的一个路径单元，
*           且#只能存在于最后一个路径单元
************************************************************/
static int amc_check_topic(const char *topicName, ALIYUN_IOT_TOPIC_TYPE_E type)
{
    if (NULL == topicName || '/' != topicName[0]) {
        return FAIL_RETURN;
    }

    if (strlen(topicName) > TOPIC_NAME_MAX_LEN) {
        ALIOT_LOG_ERROR("len of topicName exceeds 64");
        return FAIL_RETURN;
    }

    int mask = 0;
    char topicString[TOPIC_NAME_MAX_LEN];
    memset(topicString, 0x0, TOPIC_NAME_MAX_LEN);
    strncpy(topicString, topicName, TOPIC_NAME_MAX_LEN);

    char *delim = "/";
    char *iterm = NULL;
    iterm = strtok(topicString, delim);

    if (SUCCESS_RETURN != amc_check_rule(iterm, type)) {
        ALIOT_LOG_ERROR("run aliot_check_rule error");
        return FAIL_RETURN;
    }

    for (;;) {
        iterm = strtok(NULL, delim);

        if (iterm == NULL) {
            break;
        }

        //当路径中包含#字符，且不是最后一个路径名时报错
        if (1 == mask) {
            ALIOT_LOG_ERROR("the character # is error");
            return FAIL_RETURN;
        }

        if (SUCCESS_RETURN != amc_check_rule(iterm, type)) {
            ALIOT_LOG_ERROR("run aliot_check_rule error");
            return FAIL_RETURN;
        }

        if (iterm[0] == '#') {
            mask = 1;
        }
    }

    return SUCCESS_RETURN;
}


/***********************************************************
* 函数名称: MQTTKeepalive
* 描       述: mqtt保活打包发送接口
* 输入参数: Client*pClient
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明:
************************************************************/
static int MQTTKeepalive(amc_client_t *pClient)
{
    /* there is no ping outstanding - send ping packet */
    aliot_time_t timer;
    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, 1000);
    int len = 0;
    int rc = 0;

    aliot_platform_mutex_lock(pClient->writebufLock);
    len = MQTTSerialize_pingreq((unsigned char *)pClient->buf_send, pClient->buf_size_send);
    if (len <= 0) {
        aliot_platform_mutex_unlock(pClient->writebufLock);
        ALIOT_LOG_ERROR("Serialize ping request is error");
        return MQTT_PING_PACKET_ERROR;
    }

    rc = amc_send_packet(pClient, pClient->buf_send, len, &timer);
    if (SUCCESS_RETURN != rc) {
        aliot_platform_mutex_unlock(pClient->writebufLock);
        /*ping outstanding , then close socket  unsubcribe topic and handle callback function*/
        ALIOT_LOG_ERROR("ping outstanding is error,result = %d", rc);
        return MQTT_NETWORK_ERROR;
    }
    aliot_platform_mutex_unlock(pClient->writebufLock);

    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: MQTTConnect
* 描       述: mqtt连接打包发送接口
* 输入参数: Client*pClient
*           MQTTPacket_connectData* pConnectParams
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明:
************************************************************/
int MQTTConnect(amc_client_t *pClient)
{
    MQTTPacket_connectData *pConnectParams = &pClient->connectdata;
    aliot_time_t connectTimer;
    int len = 0;
    int rc = 0;

    aliot_platform_mutex_lock(pClient->writebufLock);
    if ((len = MQTTSerialize_connect((unsigned char *)pClient->buf_send, pClient->buf_size_send, pConnectParams)) <= 0) {
        aliot_platform_mutex_unlock(pClient->writebufLock);
        ALIOT_LOG_ERROR("Serialize connect packet failed,len = %d", len);
        return MQTT_CONNECT_PACKET_ERROR;
    }

    /* send the connect packet*/
    aliot_time_init(&connectTimer);
    aliot_time_cutdown(&connectTimer, pClient->command_timeout_ms);
    if ((rc = amc_send_packet(pClient, pClient->buf_send, len, &connectTimer)) != SUCCESS_RETURN) {
        aliot_platform_mutex_unlock(pClient->writebufLock);
        ALIOT_LOG_ERROR("send connect packet failed");
        return MQTT_NETWORK_ERROR;
    }
    aliot_platform_mutex_unlock(pClient->writebufLock);

    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: MQTTPublish
* 描       述: mqtt发布消息打包发送接口
* 返 回  值: 0：成功  非0：失败
* 说       明:
************************************************************/
int MQTTPublish(amc_client_t *c, const char *topicName, aliot_mqtt_topic_info_pt topic_msg)

{
    aliot_time_t timer;
    int32_t lefttime = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int len = 0;

    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->command_timeout_ms);

    aliot_platform_mutex_lock(c->writebufLock);
    len = MQTTSerialize_publish((unsigned char *)c->buf_send,
                    c->buf_size_send,
                    0,
                    topic_msg->qos,
                    topic_msg->retain,
                    topic_msg->packet_id,
                    topic,
                    (unsigned char *)topic_msg->payload,
                    topic_msg->payload_len);
    if (len <= 0) {
        aliot_platform_mutex_unlock(c->writebufLock);
        ALIOT_LOG_ERROR("MQTTSerialize_publish is error, len=%d, buf_size=%u, payloadlen=%u",
                        len,
                        c->buf_size_send,
                        topic_msg->payload_len);
        return MQTT_PUBLISH_PACKET_ERROR;
    }

    list_node_t *node = NULL;
    if (topic_msg->qos > ALIOT_MQTT_QOS0) {
        //将pub信息保存到pubInfoList中
        if (SUCCESS_RETURN != amc_push_pubInfo_to(c, len, topic_msg->packet_id, &node)) {
            ALIOT_LOG_ERROR("push publish into to pubInfolist failed!");
            aliot_platform_mutex_unlock(c->writebufLock);
            return MQTT_PUSH_TO_LIST_ERROR;
        }
    }

    if (amc_send_packet(c, c->buf_send, len, &timer) != SUCCESS_RETURN) { // send the subscribe packet
        if (topic_msg->qos > ALIOT_MQTT_QOS0) {
            //发送失败则删除之前放入pubInfoList链表中的节点
            aliot_platform_mutex_lock(c->pubInfoLock);
            list_remove(c->pubInfoList, node);
            aliot_platform_mutex_unlock(c->pubInfoLock);
        }

        lefttime = aliot_time_left(&timer);
        ALIOT_LOG_ERROR("sendPacket failed, lefttime = %d!", lefttime);
        aliot_platform_mutex_unlock(c->writebufLock);
        return MQTT_NETWORK_ERROR;
    }

    aliot_platform_mutex_unlock(c->writebufLock);
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: MQTTPuback
* 描       述: mqtt发布消息ACK打包发送接口
* 输入参数: Client*pClient
*           unsigned int msgId
*           enum msgTypes type
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明:
************************************************************/
static int MQTTPuback(amc_client_t *c, unsigned int msgId, enum msgTypes type)
{
    int rc = 0;
    int len = 0;
    aliot_time_t timer;
    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->command_timeout_ms);

    aliot_platform_mutex_lock(c->writebufLock);
    if (type == PUBACK) {
        len = MQTTSerialize_ack((unsigned char *)c->buf_send, c->buf_size_send, PUBACK, 0, msgId);
    } else if (type == PUBREC) {
        len = MQTTSerialize_ack((unsigned char *)c->buf_send, c->buf_size_send, PUBREC, 0, msgId);
    } else if (type == PUBREL) {
        len = MQTTSerialize_ack((unsigned char *)c->buf_send, c->buf_size_send, PUBREL, 0, msgId);
    } else {
        aliot_platform_mutex_unlock(c->writebufLock);
        return MQTT_PUBLISH_ACK_TYPE_ERROR;
    }

    if (len <= 0) {
        aliot_platform_mutex_unlock(c->writebufLock);
        return MQTT_PUBLISH_ACK_PACKET_ERROR;
    }

    rc = amc_send_packet(c, c->buf_send, len, &timer);
    if (rc != SUCCESS_RETURN) {
        aliot_platform_mutex_unlock(c->writebufLock);
        return MQTT_NETWORK_ERROR;
    }

    aliot_platform_mutex_unlock(c->writebufLock);
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: MQTTSubscribe
* 描       述: mqtt订阅消息打包发送接口
* 输入参数: Client*pClient
*           char* topicFilter
*           enum QoS qos
*           unsigned int msgId
*           messageHandler messageHandler
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明:
************************************************************/
static int MQTTSubscribe(amc_client_t *c, const char *topicFilter, aliot_mqtt_qos_t qos, unsigned int msgId,
                aliot_mqtt_event_handle_func_fpt messageHandler, void *pcontext)
{
    int rc = 0;
    int len = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;

    aliot_time_t timer;
    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->command_timeout_ms);

    aliot_platform_mutex_lock(c->writebufLock);

    len = MQTTSerialize_subscribe((unsigned char *)c->buf_send, c->buf_size_send, 0, (unsigned short)msgId, 1, &topic, (int *)&qos);
    if (len <= 0) {
        aliot_platform_mutex_unlock(c->writebufLock);
        return MQTT_SUBSCRIBE_PACKET_ERROR;
    }

    amc_topic_handle_t handler = {topicFilter, {messageHandler, pcontext}};

    //将sub信息保存到subInfoList中
    list_node_t *node = NULL;
    if (SUCCESS_RETURN != amc_push_subInfo_to(c, len, msgId, SUBSCRIBE, &handler, &node)) {
        ALIOT_LOG_ERROR("push publish into to pubInfolist failed!");
        aliot_platform_mutex_unlock(c->writebufLock);
        return MQTT_PUSH_TO_LIST_ERROR;
    }

    if ((rc = amc_send_packet(c, c->buf_send, len, &timer)) != SUCCESS_RETURN) { // send the subscribe packet
        //发送失败则删除之前放入subInfoList链表中的节点
        aliot_platform_mutex_lock(c->subInfoLock);
        list_remove(c->subInfoList, node);
        aliot_platform_mutex_unlock(c->subInfoLock);
        aliot_platform_mutex_unlock(c->writebufLock);
        ALIOT_LOG_ERROR("run sendPacket error!");
        return MQTT_NETWORK_ERROR;
    }

    aliot_platform_mutex_unlock(c->writebufLock);
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: MQTTUnsubscribe
* 描       述: mqtt取消订阅消息打包发送接口
* 输入参数: Client*pClient
*           const char* topicFilter
*           unsigned int msgId
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明:
************************************************************/
static int MQTTUnsubscribe(amc_client_t *c, const char *topicFilter, unsigned int msgId)
{
    aliot_time_t timer;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    int len = 0;
    int rc = 0;

    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->command_timeout_ms);

    aliot_platform_mutex_lock(c->writebufLock);

    if ((len = MQTTSerialize_unsubscribe((unsigned char *)c->buf_send, c->buf_size_send, 0, (unsigned short)msgId, 1, &topic)) <= 0) {
        aliot_platform_mutex_unlock(c->writebufLock);
        return MQTT_UNSUBSCRIBE_PACKET_ERROR;
    }

    amc_topic_handle_t handler = {topicFilter, {NULL, NULL}};

    //将sub信息保存到subInfoList中
    list_node_t *node = NULL;
    if (SUCCESS_RETURN != amc_push_subInfo_to(c, len, msgId, UNSUBSCRIBE, &handler, &node)) {
        ALIOT_LOG_ERROR("push publish into to pubInfolist failed!");
        aliot_platform_mutex_unlock(c->writebufLock);
        return MQTT_PUSH_TO_LIST_ERROR;
    }

    if ((rc = amc_send_packet(c, c->buf_send, len, &timer)) != SUCCESS_RETURN) { // send the subscribe packet
        //发送失败则删除之前放入subInfoList链表中的节点
        aliot_platform_mutex_lock(c->subInfoLock);
        list_remove(c->subInfoList, node);
        aliot_platform_mutex_unlock(c->subInfoLock);
        aliot_platform_mutex_unlock(c->writebufLock);
        return MQTT_NETWORK_ERROR;
    }

    aliot_platform_mutex_unlock(c->writebufLock);

    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: MQTTDisconnect
* 描       述: mqtt断开连接消息打包发送接口
* 输入参数: Client* c
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: mqtt协议层断开连接
************************************************************/
static int MQTTDisconnect(amc_client_t *c)
{
    int rc = FAIL_RETURN;
    aliot_time_t timer;     // we might wait for incomplete incoming publishes to complete

    aliot_platform_mutex_lock(c->writebufLock);
    int len = MQTTSerialize_disconnect((unsigned char *)c->buf_send, c->buf_size_send);

    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->command_timeout_ms);

    if (len > 0) {
        rc = amc_send_packet(c, c->buf_send, len, &timer);           // send the disconnect packet
    }

    aliot_platform_mutex_unlock(c->writebufLock);

    return rc;
}



/***********************************************************
* 函数名称: mask_pubInfo_from
* 描       述: 标记待删除的pubinfo信息
* 输入参数: Client* c
*          unsigned int  msgId
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: 删除对应msgId的pubinfo信息，在接收到puback后调用此接口删除对应pub信息
************************************************************/
static int amc_mask_pubInfo_from(amc_client_t *c, unsigned int  msgId)
{
    aliot_platform_mutex_lock(c->pubInfoLock);
    if (c->pubInfoList->len) {
        list_iterator_t *iter = list_iterator_new(c->pubInfoList, LIST_TAIL);
        list_node_t *node = NULL;
        amc_pub_info_t *repubInfo = NULL;

        for (;;) {
            node = list_iterator_next(iter);

            if (NULL == node) {
                break;
            }

            repubInfo = (amc_pub_info_t *) node->val;
            if (NULL == repubInfo) {
                ALIOT_LOG_ERROR("node's value is invalid!");
                continue;
            }

            if (repubInfo->msgId == msgId) {
                //标记无效节点
                repubInfo->nodeState = AMC_NODE_INVALID_STATE;
            }
        }

        list_iterator_destroy(iter);
    }
    aliot_platform_mutex_unlock(c->pubInfoLock);

    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: push_pubInfo_to
* 描       述: 将pubinfo信息放入pubInfoList链表
* 输入参数: Client* c
*          PUBLISH_STRUCT_INFO_S *pubInfo
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明: 将pubinfo信息放入，在发送pub消息时调用
************************************************************/
static int amc_push_pubInfo_to(amc_client_t *c, int len, unsigned short msgId, list_node_t **node)
{
    if ((len < 0) || (len > c->buf_size_send)) {
        ALIOT_LOG_ERROR("the param of len is error!")
        return FAIL_RETURN;
    }

    aliot_platform_mutex_lock(c->pubInfoLock);

    if (c->pubInfoList->len >= MQTT_REPUB_NUM_MAX) {
        aliot_platform_mutex_unlock(c->pubInfoLock);
        ALIOT_LOG_ERROR("more than %u elements in republish list. List overflow!", c->pubInfoList->len);
        return FAIL_RETURN;
    }

    //开辟内存空间
    amc_pub_info_t *repubInfo = (amc_pub_info_t *)aliot_platform_malloc(sizeof(amc_pub_info_t) + len);
    if (NULL == repubInfo) {
        aliot_platform_mutex_unlock(c->pubInfoLock);
        ALIOT_LOG_ERROR("run aliot_memory_malloc is error!");
        return FAIL_RETURN;
    }

    repubInfo->nodeState = AMC_NODE_NORMANL_STATE;
    repubInfo->msgId = msgId;
    repubInfo->len = len;
    aliot_time_start(&repubInfo->pubTime);
    repubInfo->buf = (char *)repubInfo + sizeof(amc_pub_info_t);

    //复制保存的内容
    memcpy(repubInfo->buf, c->buf_send, len);

    //创建保存结点
    *node = list_node_new(repubInfo);
    if (NULL == *node) {
        aliot_platform_mutex_unlock(c->pubInfoLock);
        ALIOT_LOG_ERROR("run list_node_new is error!");
        return FAIL_RETURN;
    }

    //将结点放入链表中
    list_rpush(c->pubInfoList, *node);

    aliot_platform_mutex_unlock(c->pubInfoLock);

    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: push_subInfo_to
* 描       述: 存放subinfo信息
* 输入参数: Client* c
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明: 将subinfo信息放入，在发送sub消息时调用
************************************************************/
static int amc_push_subInfo_to(amc_client_t *c, int len, unsigned short msgId, enum msgTypes type, amc_topic_handle_t *handler,
                    list_node_t **node)
{
    aliot_platform_mutex_lock(c->subInfoLock);

    if (c->subInfoList->len >= MQTT_SUB_NUM_MAX) {
        aliot_platform_mutex_unlock(c->subInfoLock);
        ALIOT_LOG_ERROR("number of subInfo more than max!,size = %d", c->subInfoList->len);
        return FAIL_RETURN;
    }


    //开辟内存空间
    amc_subsribe_info_t *subInfo = (amc_subsribe_info_t *)aliot_platform_malloc(sizeof(amc_subsribe_info_t) + len);
    if (NULL == subInfo) {
        aliot_platform_mutex_unlock(c->subInfoLock);
        ALIOT_LOG_ERROR("run aliot_memory_malloc is error!");
        return FAIL_RETURN;
    }

    subInfo->nodeState = AMC_NODE_NORMANL_STATE;
    subInfo->msgId = msgId;
    subInfo->len = len;
    aliot_time_start(&subInfo->subTime);
    subInfo->type = type;
    subInfo->handler = *handler;
    subInfo->buf = (unsigned char *)subInfo + sizeof(amc_subsribe_info_t);

    //复制保存的内容
    memcpy(subInfo->buf, c->buf_send, len);

    //创建保存结点
    *node = list_node_new(subInfo);
    if (NULL == *node) {
        aliot_platform_mutex_unlock(c->subInfoLock);
        ALIOT_LOG_ERROR("run list_node_new is error!");
        return FAIL_RETURN;
    }

    //将结点放入链表中
    list_rpush(c->subInfoList, *node);

    aliot_platform_mutex_unlock(c->subInfoLock);

    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: mask_subInfo_from
* 描       述: 删除subinfo信息
* 输入参数: Client* c
*          unsigned int  msgId
*          MessageHandlers *messageHandler
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明: 删除对应msgId的subinfo信息，在接收到suback后调用此接口删除对应sub信息
************************************************************/
static int amc_mask_subInfo_from(amc_client_t *c, unsigned int msgId, amc_topic_handle_t *messageHandler)
{
    aliot_platform_mutex_lock(c->subInfoLock);
    if (c->subInfoList->len) {
        list_iterator_t *iter = list_iterator_new(c->subInfoList, LIST_TAIL);
        list_node_t *node = NULL;
        amc_subsribe_info_t *subInfo = NULL;

        for (;;) {
            node = list_iterator_next(iter);
            if (NULL == node) {
                break;
            }

            subInfo = (amc_subsribe_info_t *) node->val;
            if (NULL == subInfo) {
                ALIOT_LOG_ERROR("node's value is invalid!");
                continue;
            }

            if (subInfo->msgId == msgId) {
                //此处删除的是重复消息，所以messageHandler作为出参值返回一个
                *messageHandler = subInfo->handler;

                //标记待删除结点
                subInfo->nodeState = AMC_NODE_INVALID_STATE;
            }
        }

        list_iterator_destroy(iter);
    }
    aliot_platform_mutex_unlock(c->subInfoLock);

    return SUCCESS_RETURN;
}


/***********************************************************
* 函数名称: getNextPacketId
* 描       述: 获取报文标识符
* 输入参数: Client *c
* 输出参数: VOID
* 返 回  值: id
* 说      明:
************************************************************/
static int amc_get_next_packetid(amc_client_t *c)
{
    unsigned int id = 0;
    aliot_platform_mutex_lock(c->lock_generic);
    c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
    id = c->next_packetid;
    aliot_platform_mutex_unlock(c->lock_generic);

    return id;
}

/***********************************************************
* 函数名称: sendPacket
* 描       述: 发送mqtt报文操作
* 输入参数: Client *c
*          int length
*          Timer* timer 发送超时定时器
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明:
************************************************************/
static int amc_send_packet(amc_client_t *c, char *buf, int length, aliot_time_t *time)
{
    int rc = FAIL_RETURN;
    int sent = 0;

    while (sent < length && !aliot_time_is_expired(time)) {
        rc = c->ipstack->write(c->ipstack, &buf[sent], length, aliot_time_left(time));
        if (rc < 0) { // there was an error writing the data
            break;
        }
        sent += rc;
    }

    if (sent == length) {
        rc = SUCCESS_RETURN;
    } else {
        rc = MQTT_NETWORK_ERROR;
    }
    return rc;
}

/***********************************************************
* 函数名称: decodePacket
* 描       述: 解码接收报文
* 输入参数: Client *c
*          int* value,
*          int timeout
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明:
************************************************************/
static int amc_decode_packet(amc_client_t *c, int *value, int timeout)
{
    char i;
    int multiplier = 1;
    int len = 0;
    const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

    *value = 0;
    do {
        int rc = MQTTPACKET_READ_ERROR;

        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES) {
            return MQTTPACKET_READ_ERROR; /* bad data */
        }

        rc = c->ipstack->read(c->ipstack, &i, 1, timeout);
        if (rc != 1) {
            return MQTT_NETWORK_ERROR;
        }

        *value += (i & 127) * multiplier;
        multiplier *= 128;
    } while ((i & 128) != 0);

    return len;
}

/***********************************************************
* 函数名称: readPacket
* 描       述: 读取报文
* 输入参数: Client*
*          Timer* timer
*          unsigned int *packet_type
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明:
************************************************************/
static int amc_read_packet(amc_client_t *c, aliot_time_t *timer, unsigned int *packet_type)
{
    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;
    int rc = 0;

    /* 1. read the header byte.  This has the packet type in it */
    rc = c->ipstack->read(c->ipstack, c->buf_read, 1, aliot_time_left(timer));
    if (0 == rc) {
        *packet_type = 0;
        return SUCCESS_RETURN;
    } else if (1 != rc) {
        ALIOT_LOG_DEBUG("mqtt read error, rc=%d", rc);
        return FAIL_RETURN;
    }

    len = 1;

    /* 2. read the remaining length.  This is variable in itself */
    if ((rc = amc_decode_packet(c, &rem_len, aliot_time_left(timer))) < 0) {
        ALIOT_LOG_ERROR("decodePacket error,rc = %d", rc);
        return rc;
    }

    len += MQTTPacket_encode((unsigned char *)c->buf_read + 1, rem_len); /* put the original remaining length back into the buffer */

    /*Check if the received data length exceeds mqtt read buffer length*/
    if ((rem_len > 0) && ((rem_len + len) > c->buf_size_read)) {
        ALIOT_LOG_ERROR("mqtt read buffer is too short, mqttReadBufLen : %u, remainDataLen : %d", c->buf_size_read, rem_len);
        int needReadLen = c->buf_size_read - len;
        if (c->ipstack->read(c->ipstack, c->buf_read + len, needReadLen, aliot_time_left(timer)) != needReadLen) {
            ALIOT_LOG_ERROR("mqtt read error");
            return FAIL_RETURN;
        }

        /* drop data whitch over the length of mqtt buffer*/
        int remainDataLen = rem_len - needReadLen;
        char *remainDataBuf = aliot_platform_malloc(remainDataLen + 1);
        if (!remainDataBuf) {
            ALIOT_LOG_ERROR("malloc remain buffer failed");
            return FAIL_RETURN;
        }

        if (c->ipstack->read(c->ipstack, remainDataBuf, remainDataLen, aliot_time_left(timer)) != remainDataLen) {
            ALIOT_LOG_ERROR("mqtt read error");
            aliot_platform_free(remainDataBuf);
            remainDataBuf = NULL;
            return FAIL_RETURN;
        }

        aliot_platform_free(remainDataBuf);
        remainDataBuf = NULL;

        return FAIL_RETURN;

    }

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    if (rem_len > 0 && (c->ipstack->read(c->ipstack, c->buf_read + len, rem_len, aliot_time_left(timer)) != rem_len)) {
        ALIOT_LOG_ERROR("mqtt read error");
        return FAIL_RETURN;
    }

    header.byte = c->buf_read[0];
    *packet_type = header.bits.type;
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: isTopicMatched
* 描       述: 主题是否匹配接口
* 输入参数: char* topicFilter
*           MQTTString* topicName
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明: 根据主题规则判断是否有主题匹配
************************************************************/
static char amc_is_topic_matched(char *topicFilter, MQTTString *topicName)
{
    char *curf = topicFilter;
    char *curn = topicName->lenstring.data;
    char *curn_end = curn + topicName->lenstring.len;

    while (*curf && curn < curn_end) {
        if (*curn == '/' && *curf != '/') {
            break;
        }

        if (*curf != '+' && *curf != '#' && *curf != *curn) {
            break;
        }

        if (*curf == '+') {
            // skip until we meet the next separator, or end of string
            char *nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/') {
                nextpos = ++curn + 1;
            }
        } else if (*curf == '#') {
            curn = curn_end - 1;    // skip until end of string
        }
        curf++;
        curn++;
    }

    return (curn == curn_end) && (*curf == '\0');
}

/***********************************************************
* 函数名称: deliverMessage
* 描       述: 执行pub消息接收回调
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明:
************************************************************/
static void amc_deliver_message(amc_client_t *c, MQTTString *topicName, aliot_mqtt_topic_info_pt topic_msg)
{
    int i, flag_matched = 0;

    topic_msg->ptopic = topicName->lenstring.data;
    topic_msg->topic_len = topicName->lenstring.len;

    // we have to find the right message handler - indexed by topic
    aliot_platform_mutex_lock(c->lock_generic);
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {

        if ((c->messageHandlers[i].topicFilter != 0)
             && (MQTTPacket_equals(topicName, (char *)c->messageHandlers[i].topicFilter)
                 || amc_is_topic_matched((char *)c->messageHandlers[i].topicFilter, topicName))) {
            ALIOT_LOG_DEBUG("topic be matched");

            amc_topic_handle_t msg_handle = c->messageHandlers[i];
            aliot_platform_mutex_unlock(c->lock_generic);

            if (NULL != msg_handle.handle.h_fp) {
                aliot_mqtt_event_msg_t msg;
                msg.event_type = ALIOT_MQTT_EVENT_PUBLISH_RECVEIVED;
                msg.msg = (void *)topic_msg;

                msg_handle.handle.h_fp(msg_handle.handle.pcontext, c, &msg);
                flag_matched = 1;
            }

            aliot_platform_mutex_lock(c->lock_generic);
        }
    }

    aliot_platform_mutex_unlock(c->lock_generic);

    if (0 == flag_matched) {
        ALIOT_LOG_DEBUG("NO matching any topic, call default handle function");

        if (NULL != c->handle_event.h_fp) {
            aliot_mqtt_event_msg_t msg;

            msg.event_type = ALIOT_MQTT_EVENT_PUBLISH_RECVEIVED;
            msg.msg = topic_msg;

            c->handle_event.h_fp(c->handle_event.pcontext, c, &msg);
        }
    }
}

/***********************************************************
* 函数名称: recvConnAckProc
* 描       述: 接收connect ack报文处理函数
* 输入参数: Client* c
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明:
************************************************************/
static int amc_handle_recv_CONNACK(amc_client_t *c)
{
    int rc = SUCCESS_RETURN;
    unsigned char connack_rc = 255;
    char sessionPresent = 0;
    if (MQTTDeserialize_connack((unsigned char *)&sessionPresent, &connack_rc, (unsigned char *)c->buf_read, c->buf_size_read) != 1) {
        ALIOT_LOG_ERROR("connect ack is error");
        return MQTT_CONNECT_ACK_PACKET_ERROR;
    }

    switch (connack_rc) {
        case AMC_CONNECTION_ACCEPTED:
            rc = SUCCESS_RETURN;
            break;
        case AMC_CONNECTION_REFUSED_UNACCEPTABLE_PROTOCOL_VERSION:
            rc = MQTT_CONANCK_UNACCEPTABLE_PROTOCOL_VERSION_ERROR;
            break;
        case AMC_CONNECTION_REFUSED_IDENTIFIER_REJECTED:
            rc = MQTT_CONNACK_IDENTIFIER_REJECTED_ERROR;
            break;
        case AMC_CONNECTION_REFUSED_SERVER_UNAVAILABLE:
            rc = MQTT_CONNACK_SERVER_UNAVAILABLE_ERROR;
            break;
        case AMC_CONNECTION_REFUSED_BAD_USERDATA:
            rc = MQTT_CONNACK_BAD_USERDATA_ERROR;
            break;
        case AMC_CONNECTION_REFUSED_NOT_AUTHORIZED:
            rc = MQTT_CONNACK_NOT_AUTHORIZED_ERROR;
            break;
        default:
            rc = MQTT_CONNACK_UNKNOWN_ERROR;
            break;
    }

    return rc;
}

/***********************************************************
* 函数名称: recvPubAckProc
* 描       述: 接收pub ack报文处理函数
* 输入参数: Client* c
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明:
************************************************************/
static int amc_handle_recv_PUBACK(amc_client_t *c)
{
    unsigned short mypacketid;
    unsigned char dup = 0;
    unsigned char type = 0;

    if (MQTTDeserialize_ack(&type, &dup, &mypacketid, (unsigned char *)c->buf_read, c->buf_size_read) != 1) {
        return MQTT_PUBLISH_ACK_PACKET_ERROR;
    }

    (void)amc_mask_pubInfo_from(c, mypacketid);

    if (NULL != c->handle_event.h_fp) {
        //call callback function to notify that PUBLISH is successful.
        if (NULL != c->handle_event.h_fp) {
            aliot_mqtt_event_msg_t msg;
            msg.event_type = ALIOT_MQTT_EVENT_PUBLISH_SUCCESS;
            msg.msg = (void *)mypacketid;
            c->handle_event.h_fp(c->handle_event.pcontext, c, &msg);
        }
    }

    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: recvSubAckProc
* 描       述: 接收sub ack报文处理函数
* 输入参数: Client* c
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明:
************************************************************/
static int amc_handle_recv_SUBACK(amc_client_t *c)
{
    unsigned short mypacketid;
    int i, count = 0, grantedQoS = -1;
    if (MQTTDeserialize_suback(&mypacketid, 1, &count, &grantedQoS, (unsigned char *)c->buf_read, c->buf_size_read) != 1) {
        ALIOT_LOG_ERROR("Sub ack packet error");
        return MQTT_SUBSCRIBE_ACK_PACKET_ERROR;
    }

    if (grantedQoS == 0x80) {
        aliot_mqtt_event_msg_t msg;

        ALIOT_LOG_ERROR("MQTT SUBSCRIBE failed, ack code is 0x80");

        msg.event_type = ALIOT_MQTT_EVENT_SUBCRIBE_NACK;
        msg.msg = (void *)mypacketid;
        c->handle_event.h_fp(c->handle_event.pcontext, c, &msg);

        return MQTT_SUBSCRIBE_ACK_FAILURE;
    }

    amc_topic_handle_t messagehandler;
    memset(&messagehandler, 0, sizeof(amc_topic_handle_t));
    (void)amc_mask_subInfo_from(c, mypacketid, &messagehandler);

    if ((NULL == messagehandler.handle.h_fp) || (NULL == messagehandler.topicFilter)) {
        return MQTT_SUB_INFO_NOT_FOUND_ERROR;
    }


    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        /*If subscribe the same topic and callback function, then ignore*/
        if ((NULL != c->messageHandlers[i].topicFilter)
            && (0 == amc_check_handle_is_identical(&c->messageHandlers[i], &messagehandler))) {
            //if subscribe a identical topic and relate callback function, then ignore this subscribe.
            return SUCCESS_RETURN;
        }
    }

    /*Search a free element to record topic and related callback function*/
    aliot_platform_mutex_lock(c->lock_generic);
    for (i = 0 ; i < MAX_MESSAGE_HANDLERS; ++i) {
        if (NULL == c->messageHandlers[i].topicFilter) {
            c->messageHandlers[i].topicFilter = messagehandler.topicFilter;
            c->messageHandlers[i].handle.h_fp = messagehandler.handle.h_fp;
            c->messageHandlers[i].handle.pcontext = messagehandler.handle.pcontext;
            aliot_platform_mutex_unlock(c->lock_generic);

            //call callback function to notify that SUBSCRIBE is successful.
            if (NULL != c->handle_event.h_fp) {
                aliot_mqtt_event_msg_t msg;
                msg.event_type = ALIOT_MQTT_EVENT_SUBCRIBE_SUCCESS;
                msg.msg = (void *)mypacketid;
                c->handle_event.h_fp(c->handle_event.pcontext, c, &msg);
            }

            return SUCCESS_RETURN;
        }
    }

    aliot_platform_mutex_unlock(c->lock_generic);
    /*Not free element be found*/
    ALIOT_LOG_ERROR("NOT more @messageHandlers space!");
    return FAIL_RETURN;
}

/***********************************************************
* 函数名称: recvPublishProc
* 描       述: 接收pub报文处理函数
* 输入参数: Client* c
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明:
************************************************************/
static int amc_handle_recv_PUBLISH(amc_client_t *c)
{
    int result = 0;
    MQTTString topicName;
    aliot_mqtt_topic_info_t topic_msg;

    memset(&topic_msg, 0x0, sizeof(aliot_mqtt_topic_info_t));
    memset(&topicName, 0x0, sizeof(MQTTString));

    if (1 != MQTTDeserialize_publish((unsigned char *)&topic_msg.dup,
                                (int *)&topic_msg.qos,
                                (unsigned char *)&topic_msg.retain,
                                (unsigned short *)&topic_msg.packet_id,
                                &topicName,
                                (unsigned char **)&topic_msg.payload,
                                (int *)&topic_msg.payload_len,
                                (unsigned char *)c->buf_read,
                                c->buf_size_read)) {
        return MQTT_PUBLISH_PACKET_ERROR;
    }

    topic_msg.ptopic = NULL;
    topic_msg.topic_len = 0;

    ALIOT_LOG_DEBUG("deliver msg");
    amc_deliver_message(c, &topicName, &topic_msg);
    ALIOT_LOG_DEBUG("end of delivering msg");

    if (topic_msg.qos == ALIOT_MQTT_QOS0) {
        return SUCCESS_RETURN;
    } else if (topic_msg.qos == ALIOT_MQTT_QOS1) {
        result = MQTTPuback(c, topic_msg.packet_id, PUBACK);
    } else if (topic_msg.qos == ALIOT_MQTT_QOS2) {
        result = MQTTPuback(c, topic_msg.packet_id, PUBREC);
    } else {
        ALIOT_LOG_ERROR("Invalid QOS, QOSvalue = %d", topic_msg.qos);
        return MQTT_PUBLISH_QOS_ERROR;
    }

    return result;
}



/***********************************************************
* 函数名称: recvUnsubAckProc
* 描       述: 接收unsub ack处理函数
* 输入参数: Client* c
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明:
************************************************************/
static int amc_handle_recv_UNSUBACK(amc_client_t *c)
{
    unsigned short i, mypacketid = 0;  // should be the same as the packetid above

    if (MQTTDeserialize_unsuback(&mypacketid, (unsigned char *)c->buf_read, c->buf_size_read) != 1) {

        return MQTT_UNSUBSCRIBE_ACK_PACKET_ERROR;
    }

    amc_topic_handle_t messageHandler;
    (void)amc_mask_subInfo_from(c, mypacketid, &messageHandler);

    /* Remove from message handler array */
    aliot_platform_mutex_lock(c->lock_generic);
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        if ((c->messageHandlers[i].topicFilter != NULL)
            && (0 == amc_check_handle_is_identical(&c->messageHandlers[i], &messageHandler))) {
            memset(&c->messageHandlers[i], 0, sizeof(amc_topic_handle_t));

            /* NOTE: in case of more than one register(subscribe) with different callback function,
             *       so we must keep continuously searching related message handle. */
       }
    }

    if (NULL != c->handle_event.h_fp) {
        aliot_mqtt_event_msg_t msg;
        msg.event_type = ALIOT_MQTT_EVENT_UNSUBCRIBE_SUCCESS;
        msg.msg = (void *)mypacketid;

        c->handle_event.h_fp(c->handle_event.pcontext, c, &msg);
    }

    aliot_platform_mutex_unlock(c->lock_generic);
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: waitforConnack
* 描       述: 等待connect ack
* 输入参数: Client* c
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明:
************************************************************/
static int amc_wait_CONNACK(amc_client_t *c)
{
    unsigned int packetType = 0;
    int rc = 0;
    aliot_time_t timer;
    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->connectdata.keepAliveInterval * 1000);

    do {
        // read the socket, see what work is due
        rc = amc_read_packet(c, &timer, &packetType);
        if (rc != SUCCESS_RETURN) {
            ALIOT_LOG_ERROR("readPacket error,result = %d", rc);
            return MQTT_NETWORK_ERROR;
        }

    } while (packetType != CONNACK);

    rc = amc_handle_recv_CONNACK(c);
    if (SUCCESS_RETURN != rc) {
        ALIOT_LOG_ERROR("recvConnackProc error,result = %d", rc);
    }

    return rc;
}

/***********************************************************
* 函数名称: cycle
* 描       述: 接收循环
* 输入参数: Client* c
*          Timer* timer
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说      明:
************************************************************/
static int amc_cycle(amc_client_t *c, aliot_time_t *timer)
{
    unsigned int packetType;
    int rc = SUCCESS_RETURN;

    amc_state_t state = amc_get_client_state(c);
    if (state != AMC_STATE_CONNECTED) {
        ALIOT_LOG_DEBUG("state = %d", state);
        return MQTT_STATE_ERROR;
    }

    // read the socket, see what work is due
    rc = amc_read_packet(c, timer, &packetType);
    if (rc != SUCCESS_RETURN) {
        amc_set_client_state(c, AMC_STATE_DISCONNECTED);
        ALIOT_LOG_DEBUG("readPacket error,result = %d", rc);
        return MQTT_NETWORK_ERROR;
    }

    if (MQTT_CPT_RESERVED == packetType) {
        //ALIOT_LOG_DEBUG("wait data timeout");
        return SUCCESS_RETURN;
    }

    //receive any data to renew ping_timer
    aliot_time_cutdown(&c->ping_timer, c->connectdata.keepAliveInterval * 1000);

    //接收到任何数据即清除ping消息标志
    aliot_platform_mutex_lock(c->pingMarkLock);
    c->pingMark = 0;
    aliot_platform_mutex_unlock(c->pingMarkLock);

    switch (packetType) {
        case CONNACK: {
            ALIOT_LOG_DEBUG("CONNACK");
            break;
        }
        case PUBACK: {
            rc = amc_handle_recv_PUBACK(c);

            if (SUCCESS_RETURN != rc) {
                ALIOT_LOG_ERROR("recvPubackProc error,result = %d", rc);
            }

            break;
        }
        case SUBACK: {
            rc = amc_handle_recv_SUBACK(c);
            if (SUCCESS_RETURN != rc) {
                ALIOT_LOG_ERROR("recvSubAckProc error,result = %d", rc);
            }
            ALIOT_LOG_DEBUG("SUBACK");
            break;
        }
        case PUBLISH: {
            rc = amc_handle_recv_PUBLISH(c);
            if (SUCCESS_RETURN != rc) {
                ALIOT_LOG_ERROR("recvPublishProc error,result = %d", rc);
            }
            ALIOT_LOG_DEBUG("PUBLISH");
            break;
        }
        case UNSUBACK: {
            rc = amc_handle_recv_UNSUBACK(c);
            if (SUCCESS_RETURN != rc) {
                ALIOT_LOG_ERROR("recvUnsubAckProc error,result = %d", rc);
            }
            break;
        }
        case PINGRESP: {
            rc = SUCCESS_RETURN;
            ALIOT_LOG_INFO("receive ping response!");
            break;
        }
        default:
            ALIOT_LOG_ERROR("INVALID TYPE");
            return FAIL_RETURN;
    }

    return rc;
}


//check MQTT client is in normal state.
//0, in abnormal state; 1, in normal state.
static bool amc_check_state_normal(amc_client_t *c)
{
    if (amc_get_client_state(c) == AMC_STATE_CONNECTED) {
        return 1;
    }

    return 0;
}


//return: 0, identical; NOT 0, different.
static int amc_check_handle_is_identical(amc_topic_handle_t *messageHandlers1, amc_topic_handle_t *messageHandler2)
{
    int topicNameLen = strlen(messageHandlers1->topicFilter);

    if (topicNameLen != strlen(messageHandler2->topicFilter)) {
        return 1;
    }

    if (0 != strncmp(messageHandlers1->topicFilter, messageHandler2->topicFilter, topicNameLen)) {
        return 1;
    }

    if (messageHandlers1->handle.h_fp != messageHandler2->handle.h_fp) {
        return 1;
    }

    //context must be identical also.
    if (messageHandlers1->handle.pcontext != messageHandler2->handle.pcontext) {
        return 1;
    }

    return 0;
}

/***********************************************************
* 函数名称: aliot_mqtt_subscribe
* 描       述: mqtt订阅消息
* 输入参数: Client* c                     mqtt客户端
*          char* topicFilter             订阅的主题规则
*          enum QoS qos                  消息服务类型
*          messageHandler messageHandler 消息处理结构
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: MQTT订阅操作，订阅成功，即收到sub ack
************************************************************/
static aliot_err_t amc_subscribe(amc_client_t *c,
                        const char *topicFilter,
                        aliot_mqtt_qos_t qos,
                        aliot_mqtt_event_handle_func_fpt topic_handle_func,
                        void *pcontext)
{
    if (NULL == c || NULL == topicFilter) {
        return NULL_VALUE_ERROR;
    }

    int rc = FAIL_RETURN;

    if (!amc_check_state_normal(c)) {
        ALIOT_LOG_ERROR("mqtt client state is error,state = %d", amc_get_client_state(c));
        return MQTT_STATE_ERROR;
    }

    if (0 != amc_check_topic(topicFilter, TOPIC_FILTER_TYPE)) {
        ALIOT_LOG_ERROR("topic format is error,topicFilter = %s", topicFilter);
        return MQTT_TOPIC_FORMAT_ERROR;
    }

    unsigned int msgId = amc_get_next_packetid(c);
    rc = MQTTSubscribe(c, topicFilter, qos, msgId, topic_handle_func, pcontext);
    if (rc != SUCCESS_RETURN) {
        if (rc == MQTT_NETWORK_ERROR) {
            amc_set_client_state(c, AMC_STATE_DISCONNECTED);
        }

        ALIOT_LOG_ERROR("run MQTTSubscribe error");
        return rc;
    }

    ALIOT_LOG_INFO("mqtt subscribe success,topic = %s!", topicFilter);
    return msgId;
}



/***********************************************************
* 函数名称: aliot_mqtt_unsubscribe
* 描       述: mqtt取消订阅
* 输入参数: Client* c  mqtt客户端
*          char* topicFilter 订阅的主题规则
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: 同subscribe
************************************************************/
static aliot_err_t amc_unsubscribe(amc_client_t *c, const char *topicFilter)
{
    if (NULL == c || NULL == topicFilter) {
        return NULL_VALUE_ERROR;
    }

    if (0 != amc_check_topic(topicFilter, TOPIC_FILTER_TYPE)) {
        ALIOT_LOG_ERROR("topic format is error,topicFilter = %s", topicFilter);
        return MQTT_TOPIC_FORMAT_ERROR;
    }

    int rc = FAIL_RETURN;

    if (!amc_check_state_normal(c)) {
        ALIOT_LOG_ERROR("mqtt client state is error,state = %d", amc_get_client_state(c));
        return MQTT_STATE_ERROR;
    }

    unsigned int msgId = amc_get_next_packetid(c);

    rc = MQTTUnsubscribe(c, topicFilter, msgId);
    if (rc != SUCCESS_RETURN) {
        if (rc == MQTT_NETWORK_ERROR) { // send the subscribe packet
            amc_set_client_state(c, AMC_STATE_DISCONNECTED);
        }

        ALIOT_LOG_ERROR("run MQTTUnsubscribe error!");
        return rc;
    }

    ALIOT_LOG_INFO("mqtt unsubscribe success,topic = %s!", topicFilter);
    return msgId;
}

/***********************************************************
* 函数名称: aliot_mqtt_publish
* 描       述: mqtt发布消息
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: MQTT发布消息操作，发布成功，即收到pub ack
************************************************************/
static aliot_err_t amc_publish(amc_client_t *c, const char *topicName, aliot_mqtt_topic_info_pt topic_msg)
{
    uint16_t msg_id = 0;

    if (NULL == c || NULL == topicName || NULL == topic_msg) {
        return NULL_VALUE_ERROR;
    }

    if (0 != amc_check_topic(topicName, TOPIC_NAME_TYPE)) {
        ALIOT_LOG_ERROR("topic format is error,topicFilter = %s", topicName);
        return MQTT_TOPIC_FORMAT_ERROR;
    }

    int rc = FAIL_RETURN;

    if (!amc_check_state_normal(c)) {
        ALIOT_LOG_ERROR("mqtt client state is error,state = %d", amc_get_client_state(c));
        return MQTT_STATE_ERROR;
    }

    if (topic_msg->qos == ALIOT_MQTT_QOS1 || topic_msg->qos == ALIOT_MQTT_QOS2) {
        msg_id = amc_get_next_packetid(c);
        topic_msg->packet_id = msg_id;
    }

    rc = MQTTPublish(c, topicName, topic_msg);
    if (rc != SUCCESS_RETURN) { // send the subscribe packet
        if (rc == MQTT_NETWORK_ERROR) {
            amc_set_client_state(c, AMC_STATE_DISCONNECTED);
        }
        ALIOT_LOG_ERROR("MQTTPublish is error, rc = %d", rc);
        return rc;
    }

    return msg_id;
}

/***********************************************************
* 函数名称: aliot_mqtt_get_client_state
* 描       述: 获取mqtt客户端状态
* 输入参数: Client* c  mqtt客户端
* 输出参数: VOID
* 返 回  值: mqtt client状态
* 说       明:
************************************************************/
static amc_state_t amc_get_client_state(amc_client_t *pClient)
{
    ALIOT_FUNC_ENTRY;

    amc_state_t state;
    aliot_platform_mutex_lock(pClient->stateLock);
    state = pClient->clientState;
    aliot_platform_mutex_unlock(pClient->stateLock);

    return state;
}

/***********************************************************
* 函数名称: aliot_mqtt_set_client_state
* 描       述: 设置mqtt客户端状态
* 输入参数: Client* c  mqtt客户端
* 输出参数: VOID
* 返 回  值: mqtt client状态
* 说       明:
************************************************************/
static void amc_set_client_state(amc_client_t *pClient, amc_state_t newState)
{
    ALIOT_FUNC_ENTRY;

    aliot_platform_mutex_lock(pClient->stateLock);
    pClient->clientState = newState;
    aliot_platform_mutex_unlock(pClient->stateLock);
}



/***********************************************************
* 函数名称: aliot_mqtt_set_connect_params
* 描       述: 设置mqtt连接参数
* 输入参数: Client* c  mqtt客户端
*           MQTTPacket_connectData *pConnectParams
* 输出参数: VOID
* 返 回  值: mqtt client状态
* 说       明:
************************************************************/
static int amc_set_connect_params(amc_client_t *pClient, MQTTPacket_connectData *pConnectParams)
{
    ALIOT_FUNC_ENTRY;

    if (NULL == pClient || NULL == pConnectParams) {
        ALIOT_FUNC_EXIT_RC(NULL_VALUE_ERROR);
    }

    memcpy(pClient->connectdata.struct_id, pConnectParams->struct_id, 4);
    pClient->connectdata.struct_version = pConnectParams->struct_version;
    pClient->connectdata.MQTTVersion = pConnectParams->MQTTVersion;
    pClient->connectdata.clientID = pConnectParams->clientID;
    pClient->connectdata.cleansession = pConnectParams->cleansession;
    pClient->connectdata.willFlag = pConnectParams->willFlag;
    pClient->connectdata.username = pConnectParams->username;
    pClient->connectdata.password = pConnectParams->password;
    memcpy(pClient->connectdata.will.struct_id, pConnectParams->will.struct_id, 4);
    pClient->connectdata.will.struct_version = pConnectParams->will.struct_version;
    pClient->connectdata.will.topicName = pConnectParams->will.topicName;
    pClient->connectdata.will.message = pConnectParams->will.message;
    pClient->connectdata.will.qos = pConnectParams->will.qos;
    pClient->connectdata.will.retained = pConnectParams->will.retained;

    if (pConnectParams->keepAliveInterval < KEEP_ALIVE_INTERVAL_DEFAULT_MIN) {
        pClient->connectdata.keepAliveInterval = KEEP_ALIVE_INTERVAL_DEFAULT_MIN;
    } else if (pConnectParams->keepAliveInterval > KEEP_ALIVE_INTERVAL_DEFAULT_MAX) {
        pClient->connectdata.keepAliveInterval = KEEP_ALIVE_INTERVAL_DEFAULT_MAX;
    } else {
        pClient->connectdata.keepAliveInterval = pConnectParams->keepAliveInterval;
    }

    ALIOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}

/***********************************************************
* 函数名称: aliot_mqtt_init
* 描       述: mqtt初始化
* 输入参数: Client *pClient
*          IOT_CLIENT_INIT_PARAMS *pInitParams
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: 用户创建Client结构，此接口根据用户的初始化参数初始化Client结构
************************************************************/
static aliot_err_t amc_init(amc_client_t *pClient, aliot_mqtt_param_t *pInitParams)
{
    ALIOT_FUNC_ENTRY;
    int rc = FAIL_RETURN;

    if ((NULL == pClient) || (NULL == pInitParams)) {
        ALIOT_FUNC_EXIT_RC(NULL_VALUE_ERROR);
    }

    memset(pClient, 0x0, sizeof(amc_client_t));

    MQTTPacket_connectData connectdata = MQTTPacket_connectData_initializer;

    connectdata.MQTTVersion = AMC_MQTT_VERSION;
    connectdata.keepAliveInterval = pInitParams->keepalive_interval;

    //根据用户信息初始化连接参数
    connectdata.clientID.cstring = pInitParams->client_id;
    connectdata.username.cstring = pInitParams->user_name;
    connectdata.password.cstring = pInitParams->password;


    memset(pClient->messageHandlers, 0, MAX_MESSAGE_HANDLERS * sizeof(amc_topic_handle_t));

    pClient->next_packetid = 0;
    pClient->lock_generic = aliot_platform_mutex_create();
    pClient->subInfoLock = aliot_platform_mutex_create();
    pClient->pubInfoLock = aliot_platform_mutex_create();

    //消息发送超时时间
    if (pInitParams->request_timeout_ms < ALIOT_MQTT_MIN_COMMAND_TIMEOUT
        || pInitParams->request_timeout_ms > ALIOT_MQTT_MAX_COMMAND_TIMEOUT) {

        pClient->command_timeout_ms = ALIOT_MQTT_DEFAULT_COMMAND_TIMEOUT;
    } else {
        pClient->command_timeout_ms = pInitParams->request_timeout_ms;
    }

    pClient->buf_send = pInitParams->pwrite_buf;
    pClient->buf_size_send = pInitParams->write_buf_size;
    pClient->buf_read = pInitParams->pread_buf;
    pClient->buf_size_read = pInitParams->read_buf_size;

    pClient->handle_event.h_fp = pInitParams->handle_event.h_fp;
    pClient->handle_event.pcontext = pInitParams->handle_event.pcontext;

    pClient->stateLock = aliot_platform_mutex_create();
    pClient->pingMarkLock = aliot_platform_mutex_create();

    /*reconnect params init*/
    pClient->reconnect_params.currentReconnectWaitInterval = ALIOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL;

    pClient->pubInfoList = list_new();
    pClient->pubInfoList->free = aliot_platform_free;
    pClient->subInfoList = list_new();
    pClient->subInfoList->free = aliot_platform_free;

    pClient->writebufLock = aliot_platform_mutex_create();


    /*init mqtt connect params*/
    rc = amc_set_connect_params(pClient, &connectdata);
    if (SUCCESS_RETURN != rc) {
        amc_set_client_state(pClient, AMC_STATE_INVALID);
        ALIOT_FUNC_EXIT_RC(rc);
    }

    aliot_time_init(&pClient->ping_timer);
    aliot_time_init(&pClient->reconnect_params.reconnectDelayTimer);

    pClient->ipstack = (aliot_network_pt)aliot_platform_malloc(sizeof(aliot_network_t));
    if (NULL == pClient->ipstack) {
        ALIOT_LOG_ERROR("malloc Network failed");
        ALIOT_FUNC_EXIT_RC(FAIL_RETURN);
    }
    memset(pClient->ipstack, 0x0, sizeof(aliot_network_t));


    //TODO
    //rc = aliot_mqtt_network_init(pClient->ipstack, g_userInfo.hostAddress, g_userInfo.port, g_userInfo.pubKey);

    rc = aliot_net_init(pClient->ipstack, pInitParams->host, pInitParams->port, pInitParams->pub_key);
    if (SUCCESS_RETURN != rc) {
        amc_set_client_state(pClient, AMC_STATE_INVALID);
        ALIOT_FUNC_EXIT_RC(rc);
    }

    amc_set_client_state(pClient, AMC_STATE_INITIALIZED);
    ALIOT_LOG_INFO("mqtt init success!");
    ALIOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}


/***********************************************************
* 函数名称: MQTTSubInfoProc
* 描       述: sub 消息后等待响应的处理
* 输入参数: Client *pClient
* 输出参数: VOID
* 返 回  值: VOID*
* 说       明:
************************************************************/
static int MQTTSubInfoProc(amc_client_t *pClient)
{
    int rc = SUCCESS_RETURN;

    aliot_platform_mutex_lock(pClient->subInfoLock);
    do {
        if (0 == pClient->subInfoList->len) {
            break;
        }

        list_iterator_t *iter = list_iterator_new(pClient->subInfoList, LIST_TAIL);
        list_node_t *node = NULL;
        list_node_t *tempNode = NULL;
        uint16_t packet_id = 0;
        enum msgTypes msg_type;

        for (;;) {
            node = list_iterator_next(iter);

            if (NULL != tempNode) {
                list_remove(pClient->subInfoList, tempNode);
                tempNode = NULL;
            }

            if (NULL == node) {
                //轮询结束
                break;
            }

            amc_subsribe_info_t *subInfo = (amc_subsribe_info_t *) node->val;
            if (NULL == subInfo) {
                ALIOT_LOG_ERROR("node's value is invalid!");
                tempNode = node;
                continue;
            }

            //无效结点直接删除
            if (AMC_NODE_INVALID_STATE == subInfo->nodeState) {
                tempNode = node;
                continue;
            }

            //状态异常则继续循环
            amc_state_t state = amc_get_client_state(pClient);
            if (state != AMC_STATE_CONNECTED) {
                continue;
            }

            //判断节点是否超时
            if (aliot_time_spend(&subInfo->subTime) <= (pClient->command_timeout_ms * 2)) {
                //没有超时则继续下一个节点
                continue;
            }

            /* When arrive here, it means timeout to wait ACK */

            packet_id = subInfo->msgId;
            msg_type = subInfo->type;

            aliot_platform_mutex_unlock(pClient->subInfoLock);

            //Wait MQTT SUBSCRIBE ACK timeout
            if (NULL != pClient->handle_event.h_fp) {
                aliot_mqtt_event_msg_t msg;

                if (SUBSCRIBE == msg_type) {
                    //subscribe timeout
                    msg.event_type = ALIOT_MQTT_EVENT_SUBCRIBE_TIMEOUT;
                    msg.msg = (void *)packet_id;
                } else /*if (UNSUBSCRIBE == msg_type)*/ {
                    //unsubscribe timeout
                    msg.event_type = ALIOT_MQTT_EVENT_UNSUBCRIBE_TIMEOUT;
                    msg.msg = (void *)packet_id;
                }

                pClient->handle_event.h_fp(pClient->handle_event.pcontext, pClient, &msg);
            }

            aliot_platform_mutex_lock(pClient->subInfoLock);

            tempNode = node;
        }

        list_iterator_destroy(iter);

    } while (0);

    aliot_platform_mutex_unlock(pClient->subInfoLock);

    return rc;
}


static void amc_keepalive(amc_client_t *pClient)
{
    int rc = 0;

    /*Periodic sending ping packet to detect whether the network is connected*/
    amc_keepalive_sub(pClient);

    amc_state_t currentState = amc_get_client_state(pClient);
    do {
        /*if Exceeds the maximum delay time, then return reconnect timeout*/
        if (AMC_STATE_DISCONNECTED_RECONNECTING == currentState) {
            /*Reconnection is successful, Resume regularly ping packets*/
            rc = amc_handle_reconnect(pClient);
            if (SUCCESS_RETURN != rc) {
                //重连失败增加计数
                ALIOT_LOG_DEBUG("reconnect network fail, rc = %d", rc);
            } else {
                ALIOT_LOG_INFO("network is reconnected!");

                aliot_platform_mutex_lock(pClient->pingMarkLock);
                pClient->pingMark = 0;
                aliot_platform_mutex_unlock(pClient->pingMarkLock);

                //重连成功调用网络恢复回调函数
                amc_reconnect_callback(pClient);

                pClient->reconnect_params.currentReconnectWaitInterval = ALIOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL;
            }

            break;
        }

        /*If network suddenly interrupted, stop pinging packet,  try to reconnect network immediately*/
        if (AMC_STATE_DISCONNECTED == currentState) {
            ALIOT_LOG_ERROR("network is disconnected!");

            //网络异常则调用网络断开连接回调函数
            amc_disconnect_callback(pClient);

            pClient->reconnect_params.currentReconnectWaitInterval = ALIOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL;
            aliot_time_cutdown(&(pClient->reconnect_params.reconnectDelayTimer),
                        pClient->reconnect_params.currentReconnectWaitInterval);

            //断开socket
            pClient->ipstack->disconnect(pClient->ipstack);

            amc_set_client_state(pClient, AMC_STATE_DISCONNECTED_RECONNECTING);

            break;
        }

    } while (0);
}

/***********************************************************
* 函数名称: MQTTRePublish
* 描       述: pub消息重发
* 输入参数: Client*c
*           unsigned char*buf
*           int len
* 输出参数:
* 返 回  值: 0成功；非0失败
* 说       明:
************************************************************/
static int MQTTRePublish(amc_client_t *c, char *buf, int len)
{
    aliot_time_t timer;
    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->command_timeout_ms);

    aliot_platform_mutex_lock(c->writebufLock);

    if (amc_send_packet(c, buf, len, &timer) != SUCCESS_RETURN) {
        aliot_platform_mutex_unlock(c->writebufLock);
        return MQTT_NETWORK_ERROR;
    }

    aliot_platform_mutex_unlock(c->writebufLock);
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: MQTTPubInfoProc
* 描       述: pub 消息响应后对缓存数据的处理
* 输入参数: Client *pClient
* 输出参数: VOID
* 返 回  值: VOID*
* 说       明: 因为重发的状态未可知，可能阻塞或耗时，所以删除操作
*          和重传如果在统一线程才能保证删除资源时没有其它线程在使用
************************************************************/
static int MQTTPubInfoProc(amc_client_t *pClient)
{
    int rc = 0;
    amc_state_t state = AMC_STATE_INVALID;

    aliot_platform_mutex_lock(pClient->pubInfoLock);
    do {
        if (0 == pClient->pubInfoList->len) {
            break;
        }

        list_iterator_t *iter = list_iterator_new(pClient->pubInfoList, LIST_TAIL);
        list_node_t *node = NULL;
        list_node_t *tempNode = NULL;

        for (;;) {
            node = list_iterator_next(iter);

            if (NULL != tempNode) {
                list_remove(pClient->pubInfoList, tempNode);
                tempNode = NULL;
            }

            if (NULL == node) {
                //轮询结束
                break;
            }

            amc_pub_info_t *repubInfo = (amc_pub_info_t *) node->val;
            if (NULL == repubInfo) {
                ALIOT_LOG_ERROR("node's value is invalid!");
                tempNode = node;
                continue;
            }

            //是无效节点则直接删除
            if (AMC_NODE_INVALID_STATE == repubInfo->nodeState) {
                tempNode = node;
                continue;
            }

            //状态异常退出循环(状态判断不放在外环的原因是在断网的条件下依然可以删除无效pubInfo结点)
            state = amc_get_client_state(pClient);
            if (state != AMC_STATE_CONNECTED) {
                continue;
            }

            //判断节点是否超时
            if (aliot_time_spend(&repubInfo->pubTime) <= (pClient->command_timeout_ms * 2)) {
                //没有超时则继续下一个节点
                continue;
            }

            //以下为超时重发
            aliot_platform_mutex_unlock(pClient->pubInfoLock);
            rc = MQTTRePublish(pClient, repubInfo->buf, repubInfo->len);
            aliot_time_start(&repubInfo->pubTime);
            aliot_platform_mutex_lock(pClient->pubInfoLock);

            if (MQTT_NETWORK_ERROR == rc) {
                amc_set_client_state(pClient, AMC_STATE_DISCONNECTED);
                break;
            }
        }

        list_iterator_destroy(iter);

    } while (0);

    aliot_platform_mutex_unlock(pClient->pubInfoLock);

    return SUCCESS_RETURN;
}




/***********************************************************
* 函数名称: aliot_mqtt_connect
* 描       述: mqtt协议连接
* 输入参数: Client *pClient
*           MQTTPacket_connectData* pConnectParams 连接报文参数
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: 实现网络connect和mqtt协议的connect
************************************************************/
static int amc_connect(amc_client_t *pClient)
{
    ALIOT_FUNC_ENTRY;
    int rc = FAIL_RETURN;

    if (NULL == pClient) {
        ALIOT_FUNC_EXIT_RC(NULL_VALUE_ERROR);
    }

    /*Establish TCP or TLS connection*/
    rc = pClient->ipstack->connect(pClient->ipstack);
    if (SUCCESS_RETURN != rc) {
        pClient->ipstack->disconnect(pClient->ipstack);
        ALIOT_LOG_ERROR("TCP or TLS Connection failed");

        if (ERROR_CERTIFICATE_EXPIRED == rc) {
            ALIOT_LOG_ERROR("certificate is expired!");
            return ERROR_CERT_VERIFY_FAIL;
        } else {
            return MQTT_NETWORK_CONNECT_ERROR;
        }
    }

    rc = MQTTConnect(pClient);
    if (rc  != SUCCESS_RETURN) {
        pClient->ipstack->disconnect(pClient->ipstack);
        ALIOT_LOG_ERROR("send connect packet failed");
        return rc;
    }

    if (SUCCESS_RETURN != amc_wait_CONNACK(pClient)) {
        (void)MQTTDisconnect(pClient);
        pClient->ipstack->disconnect(pClient->ipstack);
        ALIOT_LOG_ERROR("wait connect ACK timeout, or receive a ACK indicating error!");
        return MQTT_CONNECT_ERROR;
    }

    amc_set_client_state(pClient, AMC_STATE_CONNECTED);


    ALIOT_LOG_INFO("mqtt connect success!");
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliot_mqtt_attempt_reconnect
* 描       述: mqtt重连
* 输入参数: Client *pClient
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明:
************************************************************/
static int amc_attempt_reconnect(amc_client_t *pClient)
{
    ALIOT_FUNC_ENTRY;

    int rc;

    ALIOT_LOG_INFO("reconnect params:MQTTVersion =%d clientID =%s keepAliveInterval =%d username = %s",
                   pClient->connectdata.MQTTVersion,
                   pClient->connectdata.clientID.cstring,
                   pClient->connectdata.keepAliveInterval,
                   pClient->connectdata.username.cstring);

    /* Ignoring return code. failures expected if network is disconnected */
    rc = amc_connect(pClient);

    if (SUCCESS_RETURN != rc) {
        ALIOT_LOG_ERROR("run aliot_mqtt_connect() error!");
        return rc;
    }

    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliot_mqtt_handle_reconnect
* 描       述: mqtt手动重连接口
* 输入参数: Client *pClient
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: 用户手动重连接口，在自动重连失败或者没有设置自动重连时使用此接口进行重连
************************************************************/
static int amc_handle_reconnect(amc_client_t *pClient)
{
    ALIOT_FUNC_ENTRY;

    if (NULL == pClient) {
        return NULL_VALUE_ERROR;
    }

    if (!aliot_time_is_expired(&(pClient->reconnect_params.reconnectDelayTimer))) {
        /* Timer has not expired. Not time to attempt reconnect yet. Return attempting reconnect */
        return FAIL_RETURN;
    }

    ALIOT_LOG_INFO("start reconnect");

    //REDO AUTH before each reconnection
    if (0 != aliot_auth(aliot_get_device_info(), aliot_get_user_info())) {
        ALIOT_LOG_ERROR("run aliot_auth() error!\n");
        return -1;
    }

    int rc = FAIL_RETURN;
    rc = amc_attempt_reconnect(pClient);
    if (SUCCESS_RETURN == rc) {
        amc_set_client_state(pClient, AMC_STATE_CONNECTED);
        return SUCCESS_RETURN;
    } else {
        /*if reconnect network failed, then increase currentReconnectWaitInterval,
        ex: init currentReconnectWaitInterval=1s,  reconnect failed then 2s .4s. 8s*/
        if (ALIOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL > pClient->reconnect_params.currentReconnectWaitInterval) {
            pClient->reconnect_params.currentReconnectWaitInterval *= 2;
        } else {
            pClient->reconnect_params.currentReconnectWaitInterval = ALIOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL;
        }
    }

    aliot_time_cutdown(&(pClient->reconnect_params.reconnectDelayTimer),
                pClient->reconnect_params.currentReconnectWaitInterval);

    ALIOT_LOG_ERROR("mqtt reconnect failed rc = %d", rc);

    return rc;
}

/***********************************************************
* 函数名称: aliot_mqtt_disconnect
* 描       述: mqtt协议断开连接
* 输入参数: Client *pClient
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: mqtt协议的disconnect和网络的disconnect
************************************************************/
static int amc_disconnect(amc_client_t *pClient)
{
    if (NULL == pClient) {
        ALIOT_FUNC_EXIT_RC(NULL_VALUE_ERROR);
    }

    if (!amc_check_state_normal(pClient)) {
        return SUCCESS_RETURN;
    }

    ALIOT_FUNC_ENTRY;

    (void)MQTTDisconnect(pClient);

    /*close tcp/ip socket or free tls resources*/
    pClient->ipstack->disconnect(pClient->ipstack);

    amc_set_client_state(pClient, AMC_STATE_INITIALIZED);

    ALIOT_LOG_INFO("mqtt disconnect!");
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliot_mqtt_disconnect_callback
* 描       述: mqtt断开链接回调处理
* 输入参数: Client *pClient
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明:
************************************************************/
static void amc_disconnect_callback(amc_client_t *pClient)
{
    ALIOT_FUNC_ENTRY;

    if (NULL != pClient->handle_event.h_fp) {
        aliot_mqtt_event_msg_t msg;
        msg.event_type = ALIOT_MQTT_EVENT_DISCONNECT;
        msg.msg = NULL;

        pClient->handle_event.h_fp(pClient->handle_event.pcontext,
                                   pClient,
                                   &msg);
    }
}


/***********************************************************
* 函数名称: aliot_mqtt_release
* 描       述: mqtt释放
* 输入参数: Client *pClient
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明: 释放mqtt初始化时创建的资源
************************************************************/
static int amc_release(amc_client_t *pClient)
{
    ALIOT_FUNC_ENTRY;

    if (NULL == pClient) {
        ALIOT_FUNC_EXIT_RC(NULL_VALUE_ERROR);
    }

    //aliot_delete_thread(pClient);
    aliot_platform_msleep(100);

    amc_disconnect(pClient);
    amc_set_client_state(pClient, AMC_STATE_INVALID);
    aliot_platform_msleep(100);

    aliot_platform_mutex_destroy(pClient->lock_generic);
    aliot_platform_mutex_destroy(pClient->subInfoLock);
    aliot_platform_mutex_destroy(pClient->pubInfoLock);
    aliot_platform_mutex_destroy(pClient->stateLock);
    aliot_platform_mutex_destroy(pClient->pingMarkLock);
    aliot_platform_mutex_destroy(pClient->writebufLock);
    //networkRecoverSignalRelease(&pClient->networkRecoverSignal);
    //(void)aliot_sem_destory(&pClient->semaphore);

    list_destroy(pClient->pubInfoList);
    list_destroy(pClient->subInfoList);

    if (NULL != pClient->ipstack) {
        aliot_platform_free(pClient->ipstack);
    }

    ALIOT_LOG_INFO("mqtt release!");
    ALIOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}


/***********************************************************
* 函数名称: aliot_mqtt_reconnect_callback
* 描       述: mqtt恢复链接回调处理
* 输入参数: Client *pClient
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明:
************************************************************/
static void amc_reconnect_callback(amc_client_t *pClient)
{
    ALIOT_FUNC_ENTRY;

    /*handle callback function*/
    if (NULL != pClient->handle_event.h_fp) {
        aliot_mqtt_event_msg_t msg;
        msg.event_type = ALIOT_MQTT_EVENT_RECONNECT;
        msg.msg = NULL;

        pClient->handle_event.h_fp(pClient->handle_event.pcontext,
                                   pClient,
                                   &msg);
    }
}

/***********************************************************
* 函数名称: aliot_mqtt_keep_alive
* 描       述: mqtt发送保活数据
* 输入参数: Client *pClient
* 输出参数: VOID
* 返 回  值: 0：成功  非0：失败
* 说       明:
************************************************************/
static int amc_keepalive_sub(amc_client_t *pClient)
{
    ALIOT_FUNC_ENTRY;

    int rc = SUCCESS_RETURN;

    if (NULL == pClient) {
        return NULL_VALUE_ERROR;
    }

    /*if in disabled state, without having to send ping packets*/
    if (!amc_check_state_normal(pClient)) {
        return SUCCESS_RETURN;
    }

    /*if there is no ping_timer timeout, then return success*/
    if (!aliot_time_is_expired(&pClient->ping_timer)) {
        return SUCCESS_RETURN;
    }

    //update to next time sending MQTT keep-alive
    aliot_time_cutdown(&pClient->ping_timer, pClient->connectdata.keepAliveInterval * 1000);

    //如果已经发送的ping信号没有pong响应则不再重复发送，直接退出
    aliot_platform_mutex_lock(pClient->pingMarkLock);
    if (0 != pClient->pingMark) {
        aliot_platform_mutex_unlock(pClient->pingMarkLock);
        return SUCCESS_RETURN;
    }
    aliot_platform_mutex_unlock(pClient->pingMarkLock);

    //保活超时需要记录超时时间判断是否做断网重连处理
    rc = MQTTKeepalive(pClient);
    if (SUCCESS_RETURN != rc) {
        if (rc == MQTT_NETWORK_ERROR) {
            amc_set_client_state(pClient, AMC_STATE_DISCONNECTED);
        }
        ALIOT_LOG_ERROR("ping outstanding is error,result = %d", rc);
        return rc;
    }

    ALIOT_LOG_INFO("send MQTT ping...");

    //发送ping信号则设置标志位
    aliot_platform_mutex_lock(pClient->pingMarkLock);
    pClient->pingMark = 1;
    aliot_platform_mutex_unlock(pClient->pingMarkLock);

    return SUCCESS_RETURN;
}



/************************  Public Interface ************************/
void *aliot_mqtt_construct(aliot_mqtt_param_t *pInitParams)
{
    aliot_err_t err;
    amc_client_t *pclient = (amc_client_t *)aliot_platform_malloc(sizeof(amc_client_t));
    if (NULL == pclient) {
        ALIOT_LOG_ERROR("not enough memory.")
        return NULL;
    }

    err = amc_init( pclient, pInitParams );
    if (SUCCESS_RETURN != err) {
        aliot_platform_free(pclient);
        return NULL;
    }

    err = amc_connect(pclient);
    if (SUCCESS_RETURN != err) {
        amc_release(pclient);
        aliot_platform_free(pclient);
        return NULL;
    }

    return pclient;
}


aliot_err_t aliot_mqtt_deconstruct(void *handle)
{
    if (NULL == handle) {
        return NULL_VALUE_ERROR;
    }

    amc_release((amc_client_t *)handle);

    aliot_platform_free(handle);

    return SUCCESS_RETURN;
}


void aliot_mqtt_yield(void *handle, int timeout_ms)
{
    int rc = SUCCESS_RETURN;
    amc_client_t *pClient = (amc_client_t *)handle;
    aliot_time_t time;

    aliot_time_init(&time);
    aliot_time_cutdown(&time, timeout_ms);

    do {
        /*acquire package in cycle, such as PINGRESP  PUBLISH*/
        rc = amc_cycle(pClient, &time);
        if (SUCCESS_RETURN == rc) {
            //如果是pub ack则删除缓存在pub list中的信息（QOS=1时）
            MQTTPubInfoProc(pClient);

            //如果是sub ack则删除缓存在sub list中的信息
            MQTTSubInfoProc(pClient);
        }

        //Keep MQTT alive or reconnect if connection abort.
        amc_keepalive(pClient);

    } while (!aliot_time_is_expired(&time) && (SUCCESS_RETURN == rc));
}


//check whether MQTT connection is established or not.
bool aliot_mqtt_check_state_normal(void *handle)
{
    return amc_check_state_normal((amc_client_t *)handle);
}


int32_t aliot_mqtt_subscribe(void *handle,
                const char *topic_filter,
                aliot_mqtt_qos_t qos,
                aliot_mqtt_event_handle_func_fpt topic_handle_func,
                void *pcontext)
{
    return amc_subscribe((amc_client_t *)handle, topic_filter, qos, topic_handle_func, pcontext);
}


int32_t aliot_mqtt_unsubscribe(void *handle, const char *topic_filter)
{
    return amc_unsubscribe((amc_client_t *)handle, topic_filter);
}


int32_t aliot_mqtt_publish(void *handle, const char *topic_name, aliot_mqtt_topic_info_pt topic_msg)
{
    return amc_publish((amc_client_t *)handle, topic_name, topic_msg);
}



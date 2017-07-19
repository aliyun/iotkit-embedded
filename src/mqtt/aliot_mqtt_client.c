
#include <stdlib.h>
#include <stddef.h>
#include "aliot_platform.h"
#include "aliot_auth.h"
#include "utils_error.h"
#include "lite/lite-log.h"
#include "utils_net.h"
#include "utils_list.h"
#include "utils_timer.h"

#include "MQTTPacket/MQTTPacket.h"
#include "aliot_mqtt_client.h"


//amc, aliot MQTT client


/* MQTT client version number */
#define AMC_MQTT_VERSION                    (4)

/* maximum length of topic name in byte */
#define AMC_TOPIC_NAME_MAX_LEN              (64)

/* maximum MQTT packet-id */
#define AMC_PACKET_ID_MAX                   (65535)

/* maximum number of simultaneously invoke subscribe request */
#define AMC_SUB_REQUEST_NUM_MAX             (10)

/* Minimum interval of MQTT reconnect in millisecond */
#define AMC_RECONNECT_INTERVAL_MIN_MS       (1000)

/* Maximum interval of MQTT reconnect in millisecond */
#define AMC_RECONNECT_INTERVAL_MAX_MS       (60000)

/* Minimum timeout interval of MQTT request in millisecond */
#define AMC_REQUEST_TIMEOUT_MIN_MS          (500)

/* Maximum timeout interval of MQTT request in millisecond */
#define AMC_REQUEST_TIMEOUT_MAX_MS          (5000)

/* Default timeout interval of MQTT request in millisecond */
#define AMC_REQUEST_TIMEOUT_DEFAULT_MS      (2000)


typedef enum {
    AMC_CONNECTION_ACCEPTED = 0,
    AMC_CONNECTION_REFUSED_UNACCEPTABLE_PROTOCOL_VERSION = 1,
    AMC_CONNECTION_REFUSED_IDENTIFIER_REJECTED = 2,
    AMC_CONNECTION_REFUSED_SERVER_UNAVAILABLE = 3,
    AMC_CONNECTION_REFUSED_BAD_USERDATA = 4,
    AMC_CONNECTION_REFUSED_NOT_AUTHORIZED = 5
} amc_connect_ack_code_t;


/* State of MQTT client */
typedef enum {
    AMC_STATE_INVALID = 0,                    //MQTT in invalid state
    AMC_STATE_INITIALIZED = 1,                //MQTT in initializing state
    AMC_STATE_CONNECTED = 2,                  //MQTT in connected state
    AMC_STATE_DISCONNECTED = 3,               //MQTT in disconnected state
    AMC_STATE_DISCONNECTED_RECONNECTING = 4,  //MQTT in reconnecting state
} amc_state_t;


typedef enum MQTT_NODE_STATE {
    AMC_NODE_STATE_NORMANL = 0,
    AMC_NODE_STATE_INVALID,
} amc_node_t;


/* Handle structure of subscribed topic */
typedef struct {
    const char *topic_filter;
    aliot_mqtt_event_handle_t handle;
} amc_topic_handle_t;


/* Information structure of subscribed topic */
typedef struct SUBSCRIBE_INFO {
    enum msgTypes       type;           //type, (sub or unsub)
    uint16_t            msg_id;         //packet id of subscribe(unsubcribe)
    aliot_time_t        sub_start_time; //start time of subscribe request
    amc_node_t          node_state;     //state of this node
    amc_topic_handle_t  handler;        //handle of topic subscribed(unsubcribed)
    uint16_t            len;            //length of subscribe message
    unsigned char       *buf;           //subscribe message
}amc_subsribe_info_t, *amc_subsribe_info_pt;


/* Information structure of published topic */
typedef struct REPUBLISH_INFO {
    aliot_time_t      pub_start_time;   //start time of publish request
    amc_node_t        node_state;       //state of this node
    uint16_t          msg_id;           //packet id of publish
    uint32_t          len;              //length of publish message
    unsigned char    *buf;              //publish message
} amc_pub_info_t, *amc_pub_info_pt;


/* Reconnected parameter of MQTT client */
typedef struct {
    aliot_time_t       reconnect_next_time;         //the next time point of reconnect
    uint32_t           reconnect_time_interval_ms;  //time interval of this reconnect
} amc_reconnect_param_t;


/* structure of MQTT client */
typedef struct Client {
    void *                          lock_generic;                            //generic lock
    uint32_t                        packet_id;                               //packet id
    uint32_t                        request_timeout_ms;                      //request timeout in millisecond
    uint32_t                        buf_size_send;                           //send buffer size in byte
    uint32_t                        buf_size_read;                           //read buffer size in byte
    char                           *buf_send;                                //pointer of send buffer
    char                           *buf_read;                                //pointer of read buffer
    amc_topic_handle_t              sub_handle[AMC_SUB_NUM_MAX];             //array of subscribe handle
    aliot_network_pt                ipstack;                                 //network parameter
    aliot_time_t                    next_ping_time;                          //next ping time
    int                             ping_mark;                               //flag of ping
    amc_state_t                     client_state;                            //state of MQTT client
    amc_reconnect_param_t           reconnect_param;                         //reconnect parameter
    MQTTPacket_connectData          connect_data;                            //connection parameter
    list_t                         *list_pub_wait_ack;                       //list of wait publish ack
    list_t                         *list_sub_wait_ack;                       //list of subscribe or unsubscribe ack
    void *                          lock_list_pub;                           //lock of list of wait publish ack
    void *                          lock_list_sub;                           //lock of list of subscribe or unsubscribe ack
    void *                          lock_write_buf;                          //lock of write
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


typedef enum {
    TOPIC_NAME_TYPE = 0,
    TOPIC_FILTER_TYPE
} amc_topic_type_t;


//check rule whether is valid or not
static int amc_check_rule(char *iterm, amc_topic_type_t type)
{
    if (NULL == iterm) {
        log_err("iterm is NULL");
        return FAIL_RETURN;
    }

    int i = 0;
    int len = strlen(iterm);
    for (i = 0; i < len; i++) {
        if (TOPIC_FILTER_TYPE == type) {
            if ('+' == iterm[i] || '#' == iterm[i]) {
                if (1 != len) {
                    log_err("the character # and + is error");
                    return FAIL_RETURN;
                }
            }
        } else {
            if ('+' == iterm[i] || '#' == iterm[i]) {
                log_err("has character # and + is error");
                return FAIL_RETURN;
            }
        }

        if (iterm[i] < 32 || iterm[i] >= 127) {
            return FAIL_RETURN;
        }
    }
    return SUCCESS_RETURN;
}


//Check topic name
//0, topic name is valid; NOT 0, topic name is invalid
static int amc_check_topic(const char *topicName, amc_topic_type_t type)
{
    if (NULL == topicName || '/' != topicName[0]) {
        return FAIL_RETURN;
    }

    if (strlen(topicName) > AMC_TOPIC_NAME_MAX_LEN) {
        log_err("len of topicName exceeds 64");
        return FAIL_RETURN;
    }

    int mask = 0;
    char topicString[AMC_TOPIC_NAME_MAX_LEN];
    memset(topicString, 0x0, AMC_TOPIC_NAME_MAX_LEN);
    strncpy(topicString, topicName, AMC_TOPIC_NAME_MAX_LEN);

    char *delim = "/";
    char *iterm = NULL;
    iterm = strtok(topicString, delim);

    if (SUCCESS_RETURN != amc_check_rule(iterm, type)) {
        log_err("run aliot_check_rule error");
        return FAIL_RETURN;
    }

    for (;;) {
        iterm = strtok(NULL, delim);

        if (iterm == NULL) {
            break;
        }

        //The character '#' is not in the last
        if (1 == mask) {
            log_err("the character # is error");
            return FAIL_RETURN;
        }

        if (SUCCESS_RETURN != amc_check_rule(iterm, type)) {
            log_err("run aliot_check_rule error");
            return FAIL_RETURN;
        }

        if (iterm[0] == '#') {
            mask = 1;
        }
    }

    return SUCCESS_RETURN;
}


//Send keepalive packet
static int MQTTKeepalive(amc_client_t *pClient)
{
    /* there is no ping outstanding - send ping packet */
    aliot_time_t timer;
    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, 1000);
    int len = 0;
    int rc = 0;

    aliot_platform_mutex_lock(pClient->lock_write_buf);
    len = MQTTSerialize_pingreq((unsigned char *)pClient->buf_send, pClient->buf_size_send);
    if (len <= 0) {
        aliot_platform_mutex_unlock(pClient->lock_write_buf);
        log_err("Serialize ping request is error");
        return MQTT_PING_PACKET_ERROR;
    }

    rc = amc_send_packet(pClient, pClient->buf_send, len, &timer);
    if (SUCCESS_RETURN != rc) {
        aliot_platform_mutex_unlock(pClient->lock_write_buf);
        /*ping outstanding, then close socket unsubscribe topic and handle callback function*/
        log_err("ping outstanding is error,result = %d", rc);
        return MQTT_NETWORK_ERROR;
    }
    aliot_platform_mutex_unlock(pClient->lock_write_buf);

    return SUCCESS_RETURN;
}


//MQTT send connect packet
int MQTTConnect(amc_client_t *pClient)
{
    MQTTPacket_connectData *pConnectParams = &pClient->connect_data;
    aliot_time_t connectTimer;
    int len = 0;
    int rc = 0;

    aliot_platform_mutex_lock(pClient->lock_write_buf);
    if ((len = MQTTSerialize_connect((unsigned char *)pClient->buf_send, pClient->buf_size_send, pConnectParams)) <= 0) {
        aliot_platform_mutex_unlock(pClient->lock_write_buf);
        log_err("Serialize connect packet failed,len = %d", len);
        return MQTT_CONNECT_PACKET_ERROR;
    }

    /* send the connect packet*/
    aliot_time_init(&connectTimer);
    aliot_time_cutdown(&connectTimer, pClient->request_timeout_ms);
    if ((rc = amc_send_packet(pClient, pClient->buf_send, len, &connectTimer)) != SUCCESS_RETURN) {
        aliot_platform_mutex_unlock(pClient->lock_write_buf);
        log_err("send connect packet failed");
        return MQTT_NETWORK_ERROR;
    }
    aliot_platform_mutex_unlock(pClient->lock_write_buf);

    return SUCCESS_RETURN;
}


//MQTT send publish packet
int MQTTPublish(amc_client_t *c, const char *topicName, aliot_mqtt_topic_info_pt topic_msg)

{
    aliot_time_t timer;
    int32_t lefttime = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int len = 0;

    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->request_timeout_ms);

    aliot_platform_mutex_lock(c->lock_write_buf);
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
        aliot_platform_mutex_unlock(c->lock_write_buf);
        log_err("MQTTSerialize_publish is error, len=%d, buf_size=%u, payloadlen=%u",
                        len,
                        c->buf_size_send,
                        topic_msg->payload_len);
        return MQTT_PUBLISH_PACKET_ERROR;
    }

    list_node_t *node = NULL;

    //If the QOS >1, push the information into list of wait publish ACK
    if (topic_msg->qos > ALIOT_MQTT_QOS0) {
        //push into list
        if (SUCCESS_RETURN != amc_push_pubInfo_to(c, len, topic_msg->packet_id, &node)) {
            log_err("push publish into to pubInfolist failed!");
            aliot_platform_mutex_unlock(c->lock_write_buf);
            return MQTT_PUSH_TO_LIST_ERROR;
        }
    }

    //send the publish packet
    if (amc_send_packet(c, c->buf_send, len, &timer) != SUCCESS_RETURN) {
        if (topic_msg->qos > ALIOT_MQTT_QOS0) {
            //If failed, remove from list
            aliot_platform_mutex_lock(c->lock_list_pub);
            list_remove(c->list_pub_wait_ack, node);
            aliot_platform_mutex_unlock(c->lock_list_pub);
        }

        aliot_platform_mutex_unlock(c->lock_write_buf);
        return MQTT_NETWORK_ERROR;
    }

    aliot_platform_mutex_unlock(c->lock_write_buf);
    return SUCCESS_RETURN;
}


//MQTT send publish ACK
static int MQTTPuback(amc_client_t *c, unsigned int msgId, enum msgTypes type)
{
    int rc = 0;
    int len = 0;
    aliot_time_t timer;
    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->request_timeout_ms);

    aliot_platform_mutex_lock(c->lock_write_buf);
    if (type == PUBACK) {
        len = MQTTSerialize_ack((unsigned char *)c->buf_send, c->buf_size_send, PUBACK, 0, msgId);
    } else if (type == PUBREC) {
        len = MQTTSerialize_ack((unsigned char *)c->buf_send, c->buf_size_send, PUBREC, 0, msgId);
    } else if (type == PUBREL) {
        len = MQTTSerialize_ack((unsigned char *)c->buf_send, c->buf_size_send, PUBREL, 0, msgId);
    } else {
        aliot_platform_mutex_unlock(c->lock_write_buf);
        return MQTT_PUBLISH_ACK_TYPE_ERROR;
    }

    if (len <= 0) {
        aliot_platform_mutex_unlock(c->lock_write_buf);
        return MQTT_PUBLISH_ACK_PACKET_ERROR;
    }

    rc = amc_send_packet(c, c->buf_send, len, &timer);
    if (rc != SUCCESS_RETURN) {
        aliot_platform_mutex_unlock(c->lock_write_buf);
        return MQTT_NETWORK_ERROR;
    }

    aliot_platform_mutex_unlock(c->lock_write_buf);
    return SUCCESS_RETURN;
}


//MQTT send subscribe packet
static int MQTTSubscribe(amc_client_t *c, const char *topicFilter, aliot_mqtt_qos_t qos, unsigned int msgId,
                aliot_mqtt_event_handle_func_fpt messageHandler, void *pcontext)
{
    int rc = 0;
    int len = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;

    aliot_time_t timer;
    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->request_timeout_ms);

    aliot_platform_mutex_lock(c->lock_write_buf);

    len = MQTTSerialize_subscribe((unsigned char *)c->buf_send, c->buf_size_send, 0, (unsigned short)msgId, 1, &topic, (int *)&qos);
    if (len <= 0) {
        aliot_platform_mutex_unlock(c->lock_write_buf);
        return MQTT_SUBSCRIBE_PACKET_ERROR;
    }

    amc_topic_handle_t handler = {topicFilter, {messageHandler, pcontext}};

    list_node_t *node = NULL;


    /*
     * NOTE: It prefer to push the element into list and then remove it when send failed,
     *       because some of extreme cases
     * */

    //push the element to list of wait subscribe ACK
    if (SUCCESS_RETURN != amc_push_subInfo_to(c, len, msgId, SUBSCRIBE, &handler, &node)) {
        log_err("push publish into to pubInfolist failed!");
        aliot_platform_mutex_unlock(c->lock_write_buf);
        return MQTT_PUSH_TO_LIST_ERROR;
    }

    if ((rc = amc_send_packet(c, c->buf_send, len, &timer)) != SUCCESS_RETURN) { // send the subscribe packet
        //If send failed, remove it
        aliot_platform_mutex_lock(c->lock_list_sub);
        list_remove(c->list_sub_wait_ack, node);
        aliot_platform_mutex_unlock(c->lock_list_sub);
        aliot_platform_mutex_unlock(c->lock_write_buf);
        log_err("run sendPacket error!");
        return MQTT_NETWORK_ERROR;
    }

    aliot_platform_mutex_unlock(c->lock_write_buf);
    return SUCCESS_RETURN;
}


//MQTT send unsubscribe packet
static int MQTTUnsubscribe(amc_client_t *c, const char *topicFilter, unsigned int msgId)
{
    aliot_time_t timer;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    int len = 0;
    int rc = 0;

    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->request_timeout_ms);

    aliot_platform_mutex_lock(c->lock_write_buf);

    if ((len = MQTTSerialize_unsubscribe((unsigned char *)c->buf_send, c->buf_size_send, 0, (unsigned short)msgId, 1, &topic)) <= 0) {
        aliot_platform_mutex_unlock(c->lock_write_buf);
        return MQTT_UNSUBSCRIBE_PACKET_ERROR;
    }

    amc_topic_handle_t handler = {topicFilter, {NULL, NULL}};

    //push into list
    list_node_t *node = NULL;
    if (SUCCESS_RETURN != amc_push_subInfo_to(c, len, msgId, UNSUBSCRIBE, &handler, &node)) {
        log_err("push publish into to pubInfolist failed!");
        aliot_platform_mutex_unlock(c->lock_write_buf);
        return MQTT_PUSH_TO_LIST_ERROR;
    }

    if ((rc = amc_send_packet(c, c->buf_send, len, &timer)) != SUCCESS_RETURN) { // send the subscribe packet
        //remove from list
        aliot_platform_mutex_lock(c->lock_list_sub);
        list_remove(c->list_sub_wait_ack, node);
        aliot_platform_mutex_unlock(c->lock_list_sub);
        aliot_platform_mutex_unlock(c->lock_write_buf);
        return MQTT_NETWORK_ERROR;
    }

    aliot_platform_mutex_unlock(c->lock_write_buf);

    return SUCCESS_RETURN;
}


//MQTT send disconnect packet
static int MQTTDisconnect(amc_client_t *c)
{
    int rc = FAIL_RETURN;
    aliot_time_t timer;     // we might wait for incomplete incoming publishes to complete

    aliot_platform_mutex_lock(c->lock_write_buf);
    int len = MQTTSerialize_disconnect((unsigned char *)c->buf_send, c->buf_size_send);

    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->request_timeout_ms);

    if (len > 0) {
        rc = amc_send_packet(c, c->buf_send, len, &timer);           // send the disconnect packet
    }

    aliot_platform_mutex_unlock(c->lock_write_buf);

    return rc;
}


//remove the list element specified by @msgId from list of wait publish ACK
//return: 0, success; NOT 0, fail;
static int amc_mask_pubInfo_from(amc_client_t *c, uint16_t msgId)
{
    aliot_platform_mutex_lock(c->lock_list_pub);
    if (c->list_pub_wait_ack->len) {
        list_iterator_t *iter;
        list_node_t *node = NULL;
        amc_pub_info_t *repubInfo = NULL;

        if (NULL == (iter = list_iterator_new(c->list_pub_wait_ack, LIST_TAIL))) {
            aliot_platform_mutex_unlock(c->lock_list_pub);
            return SUCCESS_RETURN;
        }


        for (;;) {
            node = list_iterator_next(iter);

            if (NULL == node) {
                break;
            }

            repubInfo = (amc_pub_info_t *) node->val;
            if (NULL == repubInfo) {
                log_err("node's value is invalid!");
                continue;
            }

            if (repubInfo->msg_id == msgId) {
                repubInfo->node_state = AMC_NODE_STATE_INVALID; //mark as invalid node
            }
        }

        list_iterator_destroy(iter);
    }
    aliot_platform_mutex_unlock(c->lock_list_pub);

    return SUCCESS_RETURN;
}


//push the wait element into list of wait publish ACK
//return: 0, success; NOT 0, fail;
static int amc_push_pubInfo_to(amc_client_t *c, int len, unsigned short msgId, list_node_t **node)
{
    if ((len < 0) || (len > c->buf_size_send)) {
        log_err("the param of len is error!");
        return FAIL_RETURN;
    }

    aliot_platform_mutex_lock(c->lock_list_pub);

    if (c->list_pub_wait_ack->len >= AMC_REPUB_NUM_MAX) {
        aliot_platform_mutex_unlock(c->lock_list_pub);
        log_err("more than %u elements in republish list. List overflow!", c->list_pub_wait_ack->len);
        return FAIL_RETURN;
    }

    amc_pub_info_t *repubInfo = (amc_pub_info_t *)aliot_platform_malloc(sizeof(amc_pub_info_t) + len);
    if (NULL == repubInfo) {
        aliot_platform_mutex_unlock(c->lock_list_pub);
        log_err("run aliot_memory_malloc is error!");
        return FAIL_RETURN;
    }

    repubInfo->node_state = AMC_NODE_STATE_NORMANL;
    repubInfo->msg_id = msgId;
    repubInfo->len = len;
    aliot_time_start(&repubInfo->pub_start_time);
    repubInfo->buf = (char *)repubInfo + sizeof(amc_pub_info_t);

    memcpy(repubInfo->buf, c->buf_send, len);

    *node = list_node_new(repubInfo);
    if (NULL == *node) {
        aliot_platform_mutex_unlock(c->lock_list_pub);
        log_err("run list_node_new is error!");
        return FAIL_RETURN;
    }

    list_rpush(c->list_pub_wait_ack, *node);

    aliot_platform_mutex_unlock(c->lock_list_pub);

    return SUCCESS_RETURN;
}


//push the wait element into list of wait subscribe(unsubscribe) ACK
//return: 0, success; NOT 0, fail;
static int amc_push_subInfo_to(amc_client_t *c, int len, unsigned short msgId, enum msgTypes type, amc_topic_handle_t *handler,
                    list_node_t **node)
{
    aliot_platform_mutex_lock(c->lock_list_sub);

    if (c->list_sub_wait_ack->len >= AMC_SUB_REQUEST_NUM_MAX) {
        aliot_platform_mutex_unlock(c->lock_list_sub);
        log_err("number of subInfo more than max!,size = %d", c->list_sub_wait_ack->len);
        return FAIL_RETURN;
    }

    amc_subsribe_info_t *subInfo = (amc_subsribe_info_t *)aliot_platform_malloc(sizeof(amc_subsribe_info_t) + len);
    if (NULL == subInfo) {
        aliot_platform_mutex_unlock(c->lock_list_sub);
        log_err("run aliot_memory_malloc is error!");
        return FAIL_RETURN;
    }

    subInfo->node_state = AMC_NODE_STATE_NORMANL;
    subInfo->msg_id = msgId;
    subInfo->len = len;
    aliot_time_start(&subInfo->sub_start_time);
    subInfo->type = type;
    subInfo->handler = *handler;
    subInfo->buf = (unsigned char *)subInfo + sizeof(amc_subsribe_info_t);

    memcpy(subInfo->buf, c->buf_send, len);

    *node = list_node_new(subInfo);
    if (NULL == *node) {
        aliot_platform_mutex_unlock(c->lock_list_sub);
        log_err("run list_node_new is error!");
        return FAIL_RETURN;
    }

    list_rpush(c->list_sub_wait_ack, *node);

    aliot_platform_mutex_unlock(c->lock_list_sub);

    return SUCCESS_RETURN;
}


//remove the list element specified by @msgId from list of wait subscribe(unsubscribe) ACK
//and return message handle by @messageHandler
//return: 0, success; NOT 0, fail;
static int amc_mask_subInfo_from(amc_client_t *c, unsigned int msgId, amc_topic_handle_t *messageHandler)
{
    aliot_platform_mutex_lock(c->lock_list_sub);
    if (c->list_sub_wait_ack->len) {
        list_iterator_t *iter;
        list_node_t *node = NULL;
        amc_subsribe_info_t *subInfo = NULL;

        if (NULL == (iter = list_iterator_new(c->list_sub_wait_ack, LIST_TAIL))){
            aliot_platform_mutex_lock(c->lock_list_sub);
            return SUCCESS_RETURN;
        }

        for (;;) {
            node = list_iterator_next(iter);
            if (NULL == node) {
                break;
            }

            subInfo = (amc_subsribe_info_t *) node->val;
            if (NULL == subInfo) {
                log_err("node's value is invalid!");
                continue;
            }

            if (subInfo->msg_id == msgId) {
                *messageHandler = subInfo->handler; //return handle
                subInfo->node_state = AMC_NODE_STATE_INVALID; //mark as invalid node
            }
        }

        list_iterator_destroy(iter);
    }
    aliot_platform_mutex_unlock(c->lock_list_sub);

    return SUCCESS_RETURN;
}


//get next packet-id
static int amc_get_next_packetid(amc_client_t *c)
{
    unsigned int id = 0;
    aliot_platform_mutex_lock(c->lock_generic);
    c->packet_id = (c->packet_id == AMC_PACKET_ID_MAX) ? 1 : c->packet_id + 1;
    id = c->packet_id;
    aliot_platform_mutex_unlock(c->lock_generic);

    return id;
}


//send packet
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


//decode packet
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


//read packet
static int amc_read_packet(amc_client_t *c, aliot_time_t *timer, unsigned int *packet_type)
{
    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;
    int rc = 0;

    /* 1. read the header byte.  This has the packet type in it */
    rc = c->ipstack->read(c->ipstack, c->buf_read, 1, aliot_time_left(timer));
    if (0 == rc) { //timeout
        *packet_type = 0;
        return SUCCESS_RETURN;
    } else if (1 != rc) {
        log_debug("mqtt read error, rc=%d", rc);
        return FAIL_RETURN;
    }

    len = 1;

    /* 2. read the remaining length.  This is variable in itself */
    if ((rc = amc_decode_packet(c, &rem_len, aliot_time_left(timer))) < 0) {
        log_err("decodePacket error,rc = %d", rc);
        return rc;
    }

    len += MQTTPacket_encode((unsigned char *)c->buf_read + 1, rem_len); /* put the original remaining length back into the buffer */

    /*Check if the received data length exceeds mqtt read buffer length*/
    if ((rem_len > 0) && ((rem_len + len) > c->buf_size_read)) {
        log_err("mqtt read buffer is too short, mqttReadBufLen : %u, remainDataLen : %d", c->buf_size_read, rem_len);
        int needReadLen = c->buf_size_read - len;
        if (c->ipstack->read(c->ipstack, c->buf_read + len, needReadLen, aliot_time_left(timer)) != needReadLen) {
            log_err("mqtt read error");
            return FAIL_RETURN;
        }

        /* drop data whitch over the length of mqtt buffer*/
        int remainDataLen = rem_len - needReadLen;
        char *remainDataBuf = aliot_platform_malloc(remainDataLen + 1);
        if (!remainDataBuf) {
            log_err("malloc remain buffer failed");
            return FAIL_RETURN;
        }

        if (c->ipstack->read(c->ipstack, remainDataBuf, remainDataLen, aliot_time_left(timer)) != remainDataLen) {
            log_err("mqtt read error");
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
        log_err("mqtt read error");
        return FAIL_RETURN;
    }

    header.byte = c->buf_read[0];
    *packet_type = header.bits.type;
    return SUCCESS_RETURN;
}


//check whether the topic is matched or not
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


//deliver message
static void amc_deliver_message(amc_client_t *c, MQTTString *topicName, aliot_mqtt_topic_info_pt topic_msg)
{
    int i, flag_matched = 0;

    topic_msg->ptopic = topicName->lenstring.data;
    topic_msg->topic_len = topicName->lenstring.len;

    // we have to find the right message handler - indexed by topic
    aliot_platform_mutex_lock(c->lock_generic);
    for (i = 0; i < AMC_SUB_NUM_MAX; ++i) {

        if ((c->sub_handle[i].topic_filter != 0)
             && (MQTTPacket_equals(topicName, (char *)c->sub_handle[i].topic_filter)
                 || amc_is_topic_matched((char *)c->sub_handle[i].topic_filter, topicName))) {
            log_debug("topic be matched");

            amc_topic_handle_t msg_handle = c->sub_handle[i];
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
        log_debug("NO matching any topic, call default handle function");

        if (NULL != c->handle_event.h_fp) {
            aliot_mqtt_event_msg_t msg;

            msg.event_type = ALIOT_MQTT_EVENT_PUBLISH_RECVEIVED;
            msg.msg = topic_msg;

            c->handle_event.h_fp(c->handle_event.pcontext, c, &msg);
        }
    }
}


//handle CONNACK packet received from remote MQTT broker
static int amc_handle_recv_CONNACK(amc_client_t *c)
{
    int rc = SUCCESS_RETURN;
    unsigned char connack_rc = 255;
    char sessionPresent = 0;
    if (MQTTDeserialize_connack((unsigned char *)&sessionPresent, &connack_rc, (unsigned char *)c->buf_read, c->buf_size_read) != 1) {
        log_err("connect ack is error");
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


//handle PUBACK packet received from remote MQTT broker
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


//handle SUBACK packet received from remote MQTT broker
static int amc_handle_recv_SUBACK(amc_client_t *c)
{
    unsigned short mypacketid;
    int i, count = 0, grantedQoS = -1;
    int i_free = -1, flag_dup = 0;
    if (MQTTDeserialize_suback(&mypacketid, 1, &count, &grantedQoS, (unsigned char *)c->buf_read, c->buf_size_read) != 1) {
        log_err("Sub ack packet error");
        return MQTT_SUBSCRIBE_ACK_PACKET_ERROR;
    }

    if (grantedQoS == 0x80) {
        aliot_mqtt_event_msg_t msg;

        log_err("MQTT SUBSCRIBE failed, ack code is 0x80");

        msg.event_type = ALIOT_MQTT_EVENT_SUBCRIBE_NACK;
        msg.msg = (void *)mypacketid;
        c->handle_event.h_fp(c->handle_event.pcontext, c, &msg);

        return MQTT_SUBSCRIBE_ACK_FAILURE;
    }

    amc_topic_handle_t messagehandler;
    memset(&messagehandler, 0, sizeof(amc_topic_handle_t));
    (void)amc_mask_subInfo_from(c, mypacketid, &messagehandler);

    if ((NULL == messagehandler.handle.h_fp) || (NULL == messagehandler.topic_filter)) {
        return MQTT_SUB_INFO_NOT_FOUND_ERROR;
    }

    aliot_platform_mutex_lock(c->lock_generic);

    for (i = 0; i < AMC_SUB_NUM_MAX; ++i) {
        /*If subscribe the same topic and callback function, then ignore*/
        if ((NULL != c->sub_handle[i].topic_filter)
            && (0 == amc_check_handle_is_identical(&c->sub_handle[i], &messagehandler))) {
            //if subscribe a identical topic and relate callback function, then ignore this subscribe.
            flag_dup = 1;
            log_err("There is a identical topic and related handle in list!");
            break;
        } else {
            if (-1 == i_free) {
                i_free = i; //record free element
            }
        }
    }

    if (0 == flag_dup) {
        if (-1 == i_free) {
            log_err("NOT more @sub_handle space!");
            return FAIL_RETURN;
        } else {
            c->sub_handle[i_free].topic_filter = messagehandler.topic_filter;
            c->sub_handle[i_free].handle.h_fp = messagehandler.handle.h_fp;
            c->sub_handle[i_free].handle.pcontext = messagehandler.handle.pcontext;
        }
    }

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


//handle PUBLISH packet received from remote MQTT broker
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

    log_debug("deliver msg");
    amc_deliver_message(c, &topicName, &topic_msg);
    log_debug("end of delivering msg");

    if (topic_msg.qos == ALIOT_MQTT_QOS0) {
        return SUCCESS_RETURN;
    } else if (topic_msg.qos == ALIOT_MQTT_QOS1) {
        result = MQTTPuback(c, topic_msg.packet_id, PUBACK);
    } else if (topic_msg.qos == ALIOT_MQTT_QOS2) {
        result = MQTTPuback(c, topic_msg.packet_id, PUBREC);
    } else {
        log_err("Invalid QOS, QOSvalue = %d", topic_msg.qos);
        return MQTT_PUBLISH_QOS_ERROR;
    }

    return result;
}


//handle UNSUBACK packet received from remote MQTT broker
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
    for (i = 0; i < AMC_SUB_NUM_MAX; ++i) {
        if ((c->sub_handle[i].topic_filter != NULL)
            && (0 == amc_check_handle_is_identical(&c->sub_handle[i], &messageHandler))) {
            memset(&c->sub_handle[i], 0, sizeof(amc_topic_handle_t));

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


//wait CONNACK packet from remote MQTT broker
static int amc_wait_CONNACK(amc_client_t *c)
{
    unsigned int packetType = 0;
    int rc = 0;
    aliot_time_t timer;
    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->connect_data.keepAliveInterval * 1000);

    do {
        // read the socket, see what work is due
        rc = amc_read_packet(c, &timer, &packetType);
        if (rc != SUCCESS_RETURN) {
            log_err("readPacket error,result = %d", rc);
            return MQTT_NETWORK_ERROR;
        }

    } while (packetType != CONNACK);

    rc = amc_handle_recv_CONNACK(c);
    if (SUCCESS_RETURN != rc) {
        log_err("recvConnackProc error,result = %d", rc);
    }

    return rc;
}


//MQTT cycle to handle packet from remote broker
static int amc_cycle(amc_client_t *c, aliot_time_t *timer)
{
    unsigned int packetType;
    int rc = SUCCESS_RETURN;

    amc_state_t state = amc_get_client_state(c);
    if (state != AMC_STATE_CONNECTED) {
        log_debug("state = %d", state);
        return MQTT_STATE_ERROR;
    }

    // read the socket, see what work is due
    rc = amc_read_packet(c, timer, &packetType);
    if (rc != SUCCESS_RETURN) {
        amc_set_client_state(c, AMC_STATE_DISCONNECTED);
        log_debug("readPacket error,result = %d", rc);
        return MQTT_NETWORK_ERROR;
    }

    if (MQTT_CPT_RESERVED == packetType) {
        //log_debug("wait data timeout");
        return SUCCESS_RETURN;
    }

    //receive any data to renew ping_timer
    aliot_time_cutdown(&c->next_ping_time, c->connect_data.keepAliveInterval * 1000);

    //clear ping mark when any data received from MQTT broker
    aliot_platform_mutex_lock(c->lock_generic);
    c->ping_mark = 0;
    aliot_platform_mutex_unlock(c->lock_generic);

    switch (packetType) {
        case CONNACK: {
            log_debug("CONNACK");
            break;
        }
        case PUBACK: {
            rc = amc_handle_recv_PUBACK(c);

            if (SUCCESS_RETURN != rc) {
                log_err("recvPubackProc error,result = %d", rc);
            }

            break;
        }
        case SUBACK: {
            rc = amc_handle_recv_SUBACK(c);
            if (SUCCESS_RETURN != rc) {
                log_err("recvSubAckProc error,result = %d", rc);
            }
            log_debug("SUBACK");
            break;
        }
        case PUBLISH: {
            rc = amc_handle_recv_PUBLISH(c);
            if (SUCCESS_RETURN != rc) {
                log_err("recvPublishProc error,result = %d", rc);
            }
            log_debug("PUBLISH");
            break;
        }
        case UNSUBACK: {
            rc = amc_handle_recv_UNSUBACK(c);
            if (SUCCESS_RETURN != rc) {
                log_err("recvUnsubAckProc error,result = %d", rc);
            }
            break;
        }
        case PINGRESP: {
            rc = SUCCESS_RETURN;
            log_info("receive ping response!");
            break;
        }
        default:
            log_err("INVALID TYPE");
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
    int topicNameLen = strlen(messageHandlers1->topic_filter);

    if (topicNameLen != strlen(messageHandler2->topic_filter)) {
        return 1;
    }

    if (0 != strncmp(messageHandlers1->topic_filter, messageHandler2->topic_filter, topicNameLen)) {
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


//subscribe
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
        log_err("mqtt client state is error,state = %d", amc_get_client_state(c));
        return MQTT_STATE_ERROR;
    }

    if (0 != amc_check_topic(topicFilter, TOPIC_FILTER_TYPE)) {
        log_err("topic format is error,topicFilter = %s", topicFilter);
        return MQTT_TOPIC_FORMAT_ERROR;
    }

    unsigned int msgId = amc_get_next_packetid(c);
    rc = MQTTSubscribe(c, topicFilter, qos, msgId, topic_handle_func, pcontext);
    if (rc != SUCCESS_RETURN) {
        if (rc == MQTT_NETWORK_ERROR) {
            amc_set_client_state(c, AMC_STATE_DISCONNECTED);
        }

        log_err("run MQTTSubscribe error");
        return rc;
    }

    log_info("mqtt subscribe success,topic = %s!", topicFilter);
    return msgId;
}


//unsubscribe
static aliot_err_t amc_unsubscribe(amc_client_t *c, const char *topicFilter)
{
    if (NULL == c || NULL == topicFilter) {
        return NULL_VALUE_ERROR;
    }

    if (0 != amc_check_topic(topicFilter, TOPIC_FILTER_TYPE)) {
        log_err("topic format is error,topicFilter = %s", topicFilter);
        return MQTT_TOPIC_FORMAT_ERROR;
    }

    int rc = FAIL_RETURN;

    if (!amc_check_state_normal(c)) {
        log_err("mqtt client state is error,state = %d", amc_get_client_state(c));
        return MQTT_STATE_ERROR;
    }

    unsigned int msgId = amc_get_next_packetid(c);

    rc = MQTTUnsubscribe(c, topicFilter, msgId);
    if (rc != SUCCESS_RETURN) {
        if (rc == MQTT_NETWORK_ERROR) { // send the subscribe packet
            amc_set_client_state(c, AMC_STATE_DISCONNECTED);
        }

        log_err("run MQTTUnsubscribe error!");
        return rc;
    }

    log_info("mqtt unsubscribe success,topic = %s!", topicFilter);
    return msgId;
}


//publish
static aliot_err_t amc_publish(amc_client_t *c, const char *topicName, aliot_mqtt_topic_info_pt topic_msg)
{
    uint16_t msg_id = 0;

    if (NULL == c || NULL == topicName || NULL == topic_msg) {
        return NULL_VALUE_ERROR;
    }

    if (0 != amc_check_topic(topicName, TOPIC_NAME_TYPE)) {
        log_err("topic format is error,topicFilter = %s", topicName);
        return MQTT_TOPIC_FORMAT_ERROR;
    }

    int rc = FAIL_RETURN;

    if (!amc_check_state_normal(c)) {
        log_err("mqtt client state is error,state = %d", amc_get_client_state(c));
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
        log_err("MQTTPublish is error, rc = %d", rc);
        return rc;
    }

    return msg_id;
}


//get state of MQTT client
static amc_state_t amc_get_client_state(amc_client_t *pClient)
{
    ALIOT_FUNC_ENTRY;

    amc_state_t state;
    aliot_platform_mutex_lock(pClient->lock_generic);
    state = pClient->client_state;
    aliot_platform_mutex_unlock(pClient->lock_generic);

    return state;
}


//set state of MQTT client
static void amc_set_client_state(amc_client_t *pClient, amc_state_t newState)
{
    ALIOT_FUNC_ENTRY;

    aliot_platform_mutex_lock(pClient->lock_generic);
    pClient->client_state = newState;
    aliot_platform_mutex_unlock(pClient->lock_generic);
}


//set MQTT connection parameter
static int amc_set_connect_params(amc_client_t *pClient, MQTTPacket_connectData *pConnectParams)
{
    ALIOT_FUNC_ENTRY;

    if (NULL == pClient || NULL == pConnectParams) {
        ALIOT_FUNC_EXIT_RC(NULL_VALUE_ERROR);
    }

    memcpy(pClient->connect_data.struct_id, pConnectParams->struct_id, 4);
    pClient->connect_data.struct_version = pConnectParams->struct_version;
    pClient->connect_data.MQTTVersion = pConnectParams->MQTTVersion;
    pClient->connect_data.clientID = pConnectParams->clientID;
    pClient->connect_data.cleansession = pConnectParams->cleansession;
    pClient->connect_data.willFlag = pConnectParams->willFlag;
    pClient->connect_data.username = pConnectParams->username;
    pClient->connect_data.password = pConnectParams->password;
    memcpy(pClient->connect_data.will.struct_id, pConnectParams->will.struct_id, 4);
    pClient->connect_data.will.struct_version = pConnectParams->will.struct_version;
    pClient->connect_data.will.topicName = pConnectParams->will.topicName;
    pClient->connect_data.will.message = pConnectParams->will.message;
    pClient->connect_data.will.qos = pConnectParams->will.qos;
    pClient->connect_data.will.retained = pConnectParams->will.retained;

    if (pConnectParams->keepAliveInterval < KEEP_ALIVE_INTERVAL_DEFAULT_MIN) {
        pClient->connect_data.keepAliveInterval = KEEP_ALIVE_INTERVAL_DEFAULT_MIN;
    } else if (pConnectParams->keepAliveInterval > KEEP_ALIVE_INTERVAL_DEFAULT_MAX) {
        pClient->connect_data.keepAliveInterval = KEEP_ALIVE_INTERVAL_DEFAULT_MAX;
    } else {
        pClient->connect_data.keepAliveInterval = pConnectParams->keepAliveInterval;
    }

    ALIOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}


//Initialize MQTT client
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
    connectdata.keepAliveInterval = pInitParams->keepalive_interval_ms / 1000;

    connectdata.clientID.cstring = pInitParams->client_id;
    connectdata.username.cstring = pInitParams->user_name;
    connectdata.password.cstring = pInitParams->password;


    memset(pClient->sub_handle, 0, AMC_SUB_NUM_MAX * sizeof(amc_topic_handle_t));

    pClient->packet_id = 0;
    pClient->lock_generic = aliot_platform_mutex_create();
    pClient->lock_list_sub = aliot_platform_mutex_create();
    pClient->lock_list_pub = aliot_platform_mutex_create();

    if (pInitParams->request_timeout_ms < AMC_REQUEST_TIMEOUT_MIN_MS
        || pInitParams->request_timeout_ms > AMC_REQUEST_TIMEOUT_MAX_MS) {

        pClient->request_timeout_ms = AMC_REQUEST_TIMEOUT_DEFAULT_MS;
    } else {
        pClient->request_timeout_ms = pInitParams->request_timeout_ms;
    }

    pClient->buf_send = pInitParams->pwrite_buf;
    pClient->buf_size_send = pInitParams->write_buf_size;
    pClient->buf_read = pInitParams->pread_buf;
    pClient->buf_size_read = pInitParams->read_buf_size;

    pClient->handle_event.h_fp = pInitParams->handle_event.h_fp;
    pClient->handle_event.pcontext = pInitParams->handle_event.pcontext;

    /* Initialize reconnect parameter */
    pClient->reconnect_param.reconnect_time_interval_ms = AMC_RECONNECT_INTERVAL_MIN_MS;

    pClient->list_pub_wait_ack = list_new();
    pClient->list_pub_wait_ack->free = aliot_platform_free;
    pClient->list_sub_wait_ack = list_new();
    pClient->list_sub_wait_ack->free = aliot_platform_free;

    pClient->lock_write_buf = aliot_platform_mutex_create();


    /* Initialize MQTT connect parameter */
    rc = amc_set_connect_params(pClient, &connectdata);
    if (SUCCESS_RETURN != rc) {
        amc_set_client_state(pClient, AMC_STATE_INVALID);
        ALIOT_FUNC_EXIT_RC(rc);
    }

    aliot_time_init(&pClient->next_ping_time);
    aliot_time_init(&pClient->reconnect_param.reconnect_next_time);

    pClient->ipstack = (aliot_network_pt)aliot_platform_malloc(sizeof(aliot_network_t));
    if (NULL == pClient->ipstack) {
        log_err("malloc Network failed");
        ALIOT_FUNC_EXIT_RC(FAIL_RETURN);
    }
    memset(pClient->ipstack, 0x0, sizeof(aliot_network_t));

    rc = aliot_net_init(pClient->ipstack, pInitParams->host, pInitParams->port, pInitParams->pub_key);
    if (SUCCESS_RETURN != rc) {
        amc_set_client_state(pClient, AMC_STATE_INVALID);
        ALIOT_FUNC_EXIT_RC(rc);
    }

    amc_set_client_state(pClient, AMC_STATE_INITIALIZED);
    log_info("MQTT init success!");
    ALIOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}


//remove node of list of wait subscribe ACK, which is in invalid state or timeout
static int MQTTSubInfoProc(amc_client_t *pClient)
{
    int rc = SUCCESS_RETURN;

    aliot_platform_mutex_lock(pClient->lock_list_sub);
    do {
        if (0 == pClient->list_sub_wait_ack->len) {
            break;
        }

        list_iterator_t *iter;
        list_node_t *node = NULL;
        list_node_t *tempNode = NULL;
        uint16_t packet_id = 0;
        enum msgTypes msg_type;

        if (NULL == (iter = list_iterator_new(pClient->list_sub_wait_ack, LIST_TAIL))) {
            log_err("new list failed");
            aliot_platform_mutex_lock(pClient->lock_list_sub);
            return SUCCESS_RETURN;
        }

        for (;;) {
            node = list_iterator_next(iter);

            if (NULL != tempNode) {
                list_remove(pClient->list_sub_wait_ack, tempNode);
                tempNode = NULL;
            }

            if (NULL == node) {
                break; //end of list
            }

            amc_subsribe_info_t *subInfo = (amc_subsribe_info_t *) node->val;
            if (NULL == subInfo) {
                log_err("node's value is invalid!");
                tempNode = node;
                continue;
            }

            //remove invalid node
            if (AMC_NODE_STATE_INVALID == subInfo->node_state) {
                tempNode = node;
                continue;
            }

            if (amc_get_client_state(pClient) != AMC_STATE_CONNECTED) {
                continue;
            }

            //check the request if timeout or not
            if (aliot_time_spend(&subInfo->sub_start_time) <= (pClient->request_timeout_ms * 2)) {
                //continue to check the next node
                continue;
            }

            /* When arrive here, it means timeout to wait ACK */
            packet_id = subInfo->msg_id;
            msg_type = subInfo->type;

            aliot_platform_mutex_unlock(pClient->lock_list_sub);

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

            aliot_platform_mutex_lock(pClient->lock_list_sub);

            tempNode = node;
        }

        list_iterator_destroy(iter);

    } while (0);

    aliot_platform_mutex_unlock(pClient->lock_list_sub);

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
            aliot_platform_mutex_lock(pClient->lock_generic);
            pClient->ping_mark = 0;
            aliot_platform_mutex_unlock(pClient->lock_generic);
            rc = amc_handle_reconnect(pClient);
            if (SUCCESS_RETURN != rc) {
                log_debug("reconnect network fail, rc = %d", rc);
            } else {
                log_info("network is reconnected!");
                amc_reconnect_callback(pClient);
                pClient->reconnect_param.reconnect_time_interval_ms = AMC_RECONNECT_INTERVAL_MIN_MS;
            }

            break;
        }

        /*If network suddenly interrupted, stop pinging packet, try to reconnect network immediately*/
        if (AMC_STATE_DISCONNECTED == currentState) {
            log_err("network is disconnected!");
            amc_disconnect_callback(pClient);

            pClient->reconnect_param.reconnect_time_interval_ms = AMC_RECONNECT_INTERVAL_MIN_MS;
            aliot_time_cutdown(&(pClient->reconnect_param.reconnect_next_time),
                        pClient->reconnect_param.reconnect_time_interval_ms);

            pClient->ipstack->disconnect(pClient->ipstack);
            amc_set_client_state(pClient, AMC_STATE_DISCONNECTED_RECONNECTING);
            break;
        }

    } while (0);
}


//republish
static int MQTTRePublish(amc_client_t *c, char *buf, int len)
{
    aliot_time_t timer;
    aliot_time_init(&timer);
    aliot_time_cutdown(&timer, c->request_timeout_ms);

    aliot_platform_mutex_lock(c->lock_write_buf);

    if (amc_send_packet(c, buf, len, &timer) != SUCCESS_RETURN) {
        aliot_platform_mutex_unlock(c->lock_write_buf);
        return MQTT_NETWORK_ERROR;
    }

    aliot_platform_mutex_unlock(c->lock_write_buf);
    return SUCCESS_RETURN;
}


//remove node of list of wait publish ACK, which is in invalid state or timeout
static int MQTTPubInfoProc(amc_client_t *pClient)
{
    int rc = 0;
    amc_state_t state = AMC_STATE_INVALID;

    aliot_platform_mutex_lock(pClient->lock_list_pub);
    do {
        if (0 == pClient->list_pub_wait_ack->len) {
            break;
        }

        list_iterator_t *iter;
        list_node_t *node = NULL;
        list_node_t *tempNode = NULL;

        if (NULL == (iter = list_iterator_new(pClient->list_pub_wait_ack, LIST_TAIL))){
            log_err("new list failed");
            break;
        }

        for (;;) {
            node = list_iterator_next(iter);

            if (NULL != tempNode) {
                list_remove(pClient->list_pub_wait_ack, tempNode);
                tempNode = NULL;
            }

            if (NULL == node) {
                break; //end of list
            }

            amc_pub_info_t *repubInfo = (amc_pub_info_t *) node->val;
            if (NULL == repubInfo) {
                log_err("node's value is invalid!");
                tempNode = node;
                continue;
            }

            //remove invalid node
            if (AMC_NODE_STATE_INVALID == repubInfo->node_state) {
                tempNode = node;
                continue;
            }

            state = amc_get_client_state(pClient);
            if (state != AMC_STATE_CONNECTED) {
                continue;
            }

            //check the request if timeout or not
            if (aliot_time_spend(&repubInfo->pub_start_time) <= (pClient->request_timeout_ms * 2)) {
                continue;
            }

            //If wait ACK timeout, republish
            aliot_platform_mutex_unlock(pClient->lock_list_pub);
            rc = MQTTRePublish(pClient, repubInfo->buf, repubInfo->len);
            aliot_time_start(&repubInfo->pub_start_time);
            aliot_platform_mutex_lock(pClient->lock_list_pub);

            if (MQTT_NETWORK_ERROR == rc) {
                amc_set_client_state(pClient, AMC_STATE_DISCONNECTED);
                break;
            }
        }

        list_iterator_destroy(iter);

    } while (0);

    aliot_platform_mutex_unlock(pClient->lock_list_pub);

    return SUCCESS_RETURN;
}


//connect
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
        log_err("TCP or TLS Connection failed");

        if (ERROR_CERTIFICATE_EXPIRED == rc) {
            log_err("certificate is expired!");
            return ERROR_CERT_VERIFY_FAIL;
        } else {
            return MQTT_NETWORK_CONNECT_ERROR;
        }
    }

    log_debug("start MQTT connection with parameters: clientid=%s, username=%s, password=%s",
            pClient->connect_data.clientID.cstring,
            pClient->connect_data.username.cstring,
            pClient->connect_data.password.cstring);

    rc = MQTTConnect(pClient);
    if (rc  != SUCCESS_RETURN) {
        pClient->ipstack->disconnect(pClient->ipstack);
        log_err("send connect packet failed");
        return rc;
    }

    if (SUCCESS_RETURN != amc_wait_CONNACK(pClient)) {
        (void)MQTTDisconnect(pClient);
        pClient->ipstack->disconnect(pClient->ipstack);
        log_err("wait connect ACK timeout, or receive a ACK indicating error!");
        return MQTT_CONNECT_ERROR;
    }

    amc_set_client_state(pClient, AMC_STATE_CONNECTED);

    aliot_time_cutdown(&pClient->next_ping_time, pClient->connect_data.keepAliveInterval * 1000);

    log_info("mqtt connect success!");
    return SUCCESS_RETURN;
}


static int amc_attempt_reconnect(amc_client_t *pClient)
{
    ALIOT_FUNC_ENTRY;

    int rc;

    log_info("reconnect params:MQTTVersion =%d clientID =%s keepAliveInterval =%d username = %s",
                   pClient->connect_data.MQTTVersion,
                   pClient->connect_data.clientID.cstring,
                   pClient->connect_data.keepAliveInterval,
                   pClient->connect_data.username.cstring);

    /* Ignoring return code. failures expected if network is disconnected */
    rc = amc_connect(pClient);

    if (SUCCESS_RETURN != rc) {
        log_err("run aliot_mqtt_connect() error!");
        return rc;
    }

    return SUCCESS_RETURN;
}


//reconnect
static int amc_handle_reconnect(amc_client_t *pClient)
{
    ALIOT_FUNC_ENTRY;

    if (NULL == pClient) {
        return NULL_VALUE_ERROR;
    }

    if (!aliot_time_is_expired(&(pClient->reconnect_param.reconnect_next_time))) {
        /* Timer has not expired. Not time to attempt reconnect yet. Return attempting reconnect */
        return FAIL_RETURN;
    }

    log_info("start reconnect");

    //REDO AUTH before each reconnection
    if (0 != aliot_auth(aliot_get_device_info(), aliot_get_user_info())) {
        log_err("run aliot_auth() error!\n");
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
        if (AMC_RECONNECT_INTERVAL_MAX_MS > pClient->reconnect_param.reconnect_time_interval_ms) {
            pClient->reconnect_param.reconnect_time_interval_ms *= 2;
        } else {
            pClient->reconnect_param.reconnect_time_interval_ms = AMC_RECONNECT_INTERVAL_MAX_MS;
        }
    }

    aliot_time_cutdown(&(pClient->reconnect_param.reconnect_next_time),
                pClient->reconnect_param.reconnect_time_interval_ms);

    log_err("mqtt reconnect failed rc = %d", rc);

    return rc;
}


//disconnect
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

    log_info("mqtt disconnect!");
    return SUCCESS_RETURN;
}



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


//release MQTT resource
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
    aliot_platform_mutex_destroy(pClient->lock_list_sub);
    aliot_platform_mutex_destroy(pClient->lock_list_pub);
    aliot_platform_mutex_destroy(pClient->lock_write_buf);

    list_destroy(pClient->list_pub_wait_ack);
    list_destroy(pClient->list_sub_wait_ack);

    if (NULL != pClient->ipstack) {
        aliot_platform_free(pClient->ipstack);
    }

    log_info("mqtt release!");
    ALIOT_FUNC_EXIT_RC(SUCCESS_RETURN);
}


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
    if (!aliot_time_is_expired(&pClient->next_ping_time)) {
        return SUCCESS_RETURN;
    }

    //update to next time sending MQTT keep-alive
    aliot_time_cutdown(&pClient->next_ping_time, pClient->connect_data.keepAliveInterval * 1000);

    rc = MQTTKeepalive(pClient);
    if (SUCCESS_RETURN != rc) {
        if (rc == MQTT_NETWORK_ERROR) {
            amc_set_client_state(pClient, AMC_STATE_DISCONNECTED);
        }
        log_err("ping outstanding is error,result = %d", rc);
        return rc;
    }

    log_info("send MQTT ping...");

    aliot_platform_mutex_lock(pClient->lock_generic);
    pClient->ping_mark = 1;
    aliot_platform_mutex_unlock(pClient->lock_generic);

    return SUCCESS_RETURN;
}



/************************  Public Interface ************************/
void *aliot_mqtt_construct(aliot_mqtt_param_t *pInitParams)
{
    aliot_err_t err;
    amc_client_t *pclient = (amc_client_t *)aliot_platform_malloc(sizeof(amc_client_t));
    if (NULL == pclient) {
        log_err("not enough memory.");
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
            //check list of wait publish ACK to remove node that is ACKED or timeout
            MQTTPubInfoProc(pClient);

            //check list of wait subscribe(or unsubscribe) ACK to remove node that is ACKED or timeout
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



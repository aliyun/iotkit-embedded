/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef _IOTX_MQTT_CLIENT_H_
#define _IOTX_MQTT_CLIENT_H_
#if defined(__cplusplus)
extern "C" {
#endif

#include "iotx_mqtt_internal.h"

typedef enum {
    IOTX_MC_CONNECTION_ACCEPTED = 0,
    IOTX_MC_CONNECTION_REFUSED_UNACCEPTABLE_PROTOCOL_VERSION = 1,
    IOTX_MC_CONNECTION_REFUSED_IDENTIFIER_REJECTED = 2,
    IOTX_MC_CONNECTION_REFUSED_SERVER_UNAVAILABLE = 3,
    IOTX_MC_CONNECTION_REFUSED_BAD_USERDATA = 4,
    IOTX_MC_CONNECTION_REFUSED_NOT_AUTHORIZED = 5
} iotx_mc_connect_ack_code_t;


/* State of MQTT client */
typedef enum {
    IOTX_MC_STATE_INVALID = 0,                    /* MQTT in invalid state */
    IOTX_MC_STATE_INITIALIZED = 1,                /* MQTT in initializing state */
    IOTX_MC_STATE_CONNECTED = 2,                  /* MQTT in connected state */
    IOTX_MC_STATE_DISCONNECTED = 3,               /* MQTT in disconnected state */
    IOTX_MC_STATE_DISCONNECTED_RECONNECTING = 4,  /* MQTT in reconnecting state */
} iotx_mc_state_t;


typedef enum MQTT_NODE_STATE {
    IOTX_MC_NODE_STATE_NORMANL = 0,
    IOTX_MC_NODE_STATE_INVALID,
} iotx_mc_node_t;


/* Handle structure of subscribed topic */
typedef struct {
    const char *topic_filter;
    iotx_mqtt_event_handle_t handle;
} iotx_mc_topic_handle_t;


/* Information structure of subscribed topic */
typedef struct SUBSCRIBE_INFO {
    enum msgTypes           type;           /* type, (sub or unsub) */
    uint16_t                msg_id;         /* packet id of subscribe(unsubcribe) */
    iotx_time_t             sub_start_time; /* start time of subscribe request */
    iotx_mc_node_t          node_state;     /* state of this node */
    iotx_mc_topic_handle_t *handler;        /* handle of topic subscribed(unsubcribed) */
    uint16_t                len;            /* length of subscribe message */
    unsigned char          *buf;            /* subscribe message */
} iotx_mc_subsribe_info_t, *iotx_mc_subsribe_info_pt;


/* Information structure of published topic */
typedef struct REPUBLISH_INFO {
    iotx_time_t             pub_start_time;     /* start time of publish request */
    iotx_mc_node_t          node_state;         /* state of this node */
    uint16_t                msg_id;             /* packet id of publish */
    uint32_t                len;                /* length of publish message */
    unsigned char          *buf;                /* publish message */
} iotx_mc_pub_info_t, *iotx_mc_pub_info_pt;


/* Reconnected parameter of MQTT client */
typedef struct {
    iotx_time_t         reconnect_next_time;         /* the next time point of reconnect */
    uint32_t            reconnect_time_interval_ms;  /* time interval of this reconnect */
} iotx_mc_reconnect_param_t;

/* structure of MQTT client */
typedef struct Client {
    void                           *lock_generic;                            /* generic lock */
    uint32_t                        packet_id;                               /* packet id */
    uint32_t                        request_timeout_ms;                      /* request timeout in millisecond */
    uint32_t                        buf_size_send;                           /* send buffer size in byte */
    uint32_t                        buf_size_read;                           /* read buffer size in byte */
    uint8_t                         keepalive_probes;                        /* keepalive probes */
    char                           *buf_send;                                /* pointer of send buffer */
    char                           *buf_read;                                /* pointer of read buffer */
    iotx_mc_topic_handle_t          sub_handle[IOTX_MC_SUB_NUM_MAX];         /* array of subscribe handle */
    utils_network_pt                ipstack;                                 /* network parameter */
    iotx_time_t                     next_ping_time;                          /* next ping time */
    int                             ping_mark;                               /* flag of ping */
    iotx_mc_state_t                 client_state;                            /* state of MQTT client */
    iotx_mc_reconnect_param_t       reconnect_param;                         /* reconnect parameter */
    MQTTPacket_connectData          connect_data;                            /* connection parameter */
    list_t                         *list_pub_wait_ack;                       /* list of wait publish ack */
    list_t                         *list_sub_wait_ack;                       /* list of subscribe or unsubscribe ack */
    void                           *lock_list_pub;                           /* lock of list of wait publish ack */
    void
    *lock_list_sub;                           /* lock of list of subscribe or unsubscribe ack */
    void                           *lock_write_buf;                          /* lock of write */
    iotx_mqtt_event_handle_t        handle_event;                            /* event handle */
    int (*mqtt_auth)(void);
} iotx_mc_client_t, *iotx_mc_client_pt;

typedef enum {
    TOPIC_NAME_TYPE = 0,
    TOPIC_FILTER_TYPE
} iotx_mc_topic_type_t;

#if defined(__cplusplus)
}
#endif
#endif  /* #ifndef _IOTX_MQTT_CLIENT_H_ */

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

#ifndef SRC_SDK_IMPL_EXPORTS_IOT_EXPORT_CLOUD_CONNECTION_H_
#define SRC_SDK_IMPL_EXPORTS_IOT_EXPORT_CLOUD_CONNECTION_H_


/* follow as MQTT QoS */
typedef enum {
    IOTX_MESSAGE_QOS0 = 0,
    IOTX_MESSAGE_QOS1,
    IOTX_MESSAGE_QOS2
} iotx_message_qos_t;

/* The message payload encode format */
typedef enum {
    IOTX_MESSAGE_CONTENT_TYPE_JSON,
    IOTX_MESSAGE_CONTENT_TYPE_CBOR,
} iotx_message_content_type_t;

/* The message type */
typedef enum {
    IOTX_MESSAGE_CONFIRMABLE  = 0,           /* confirmable message */
    IOTX_MESSAGE_NON_CONFIRMABLE  = 1,       /* non-confirmable message */
} iotx_message_type_t;
    
    
/* event type */
typedef enum IOTX_CLOUD_CONNECTION_EVENT_TYPES {
    IOTX_CLOUD_CONNECTION_EVENT_DISCONNECT,
    
    IOTX_CLOUD_CONNECTION_EVENT_RECONNECT,

    /* Maximum number of protocol */
    IOTX_CLOUD_CONNECTION_EVENT_MAX
}iotx_cloud_connection_event_types_t;

/* protocol type */
typedef enum IOTX_CLOUD_CONNECTION_PROTOCOL_TYPES {
    /* MQTT */
    IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_MQTT = 1,
    
    /* CoAP */
    IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_COAP = 2,
    
    /* HTTP */
    IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_HTTP = 3,

    /* Maximum number of protocol */
    IOTX_CLOUD_CONNECTION_PROTOCOL_TYPE_MAX
}iotx_cloud_connection_protocol_types_t;


/* response type */
typedef enum IOTX_CLOUD_CONNECTION_RESPONSE_TYPES {
    /* subcribe success */
    IOTX_CLOUD_CONNECTION_RESPONSE_SUBSCRIBE_SUCCESS = 1,    
    
    /* subcribe fail */
    IOTX_CLOUD_CONNECTION_RESPONSE_SUBSCRIBE_FAIL = 2,  
    
    /* subcribe success */
    IOTX_CLOUD_CONNECTION_RESPONSE_UNSUBSCRIBE_SUCCESS = 3,    
    
    /* subcribe fail */
    IOTX_CLOUD_CONNECTION_RESPONSE_UNSUBSCRIBE_FAIL = 4,   
    
    /* send success */
    IOTX_CLOUD_CONNECTION_RESPONSE_SEND_SUCCESS = 5,    
    
    /* send fail */
    IOTX_CLOUD_CONNECTION_RESPONSE_SEND_FAIL = 6,      
    
    /* receive new data */
    IOTX_CLOUD_CONNECTION_RESPONSE_NEW_DATA = 7,       

    /* Maximum number of response type */
    IOTX_CLOUD_CONNECTION_RESPONSE_TYPE_MAX
}iotx_cloud_connection_response_types_t;



/* message type */
typedef enum IOTX_CLOUD_CONNECTION_MESSAGE_TYPES {
    /* MQTT: subcribe */
    IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_SUBSCRIBE = 1,    
    
    /* MQTT: unsubcribe */
    IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_UNSUBSCRIBE = 2,    
    
    /* MQTT: publish; CoAP & HTTP: send message */
    IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_PUBLISH = 3,

    /* Maximum number of message type */
    IOTX_CLOUD_CONNECTION_MESSAGE_TYPE_MAX
}iotx_cloud_connection_message_types_t;


/* The structure of cloud Connection event struct */
typedef struct {
    uint8_t                                   event_id;
    void*                                     msg;
} iotx_cloud_connection_event_msg_t, *iotx_cloud_connection_event_msg_pt;


/**
 * @brief It define a datatype of function pointer.
 *        This type of function will be called when a related event occur.
 *
 * @param pcontext : The program context.
 * @param pConnection : The cloud Connection.
 * @param msg : The event message.
 *
 * @return none
 */
typedef void (*iotx_cloud_connection_event_handle_func_fpt)(void *pconnection, iotx_cloud_connection_event_msg_pt msg);

/* The structure of cloud Connection param */
typedef struct {
    iotx_cloud_connection_response_types_t      rsp_type;
    char                                       *URI;
    uint32_t                                    URI_length;
    void                                       *payload;
    uint32_t                                    payload_length;
    iotx_message_qos_t                          QoS;
} iotx_cloud_connection_msg_rsp_t, *iotx_cloud_connection_msg_rsp_pt;


/**
 * @brief It define a datatype of function pointer.
 *        This type of function will be called when a related event occur.
 *
 * @param pcontext : The program context.
 * @param pConnection : The cloud Connection.
 * @param msg : The event message.
 *
 * @return none
 */
typedef void (*iotx_cloud_connection_msg_rsp_func_fpt)(void *pcontext, void *pconnection, iotx_cloud_connection_msg_rsp_pt msg);


/* The structure of cloud Connection param */
typedef struct {
    iotx_cloud_connection_protocol_types_t      protocol_type;
    iotx_device_info_t                         *device_info;
    uint8_t                                     clean_session;      /* MQTT: clean_session,  HTTP: keep_alive*/         
    uint32_t                                    request_timeout_ms;       
    uint32_t                                    keepalive_interval_ms;   /* MQTT keepalive_interval_ms */
    void*                                       event_pcontext;              
    iotx_cloud_connection_event_handle_func_fpt event_handler;              
} iotx_cloud_connection_param_t, *iotx_cloud_connection_param_pt;


typedef struct {
    iotx_cloud_connection_message_types_t       type;    
    char                                       *URI;
    uint32_t                                    URI_length;
    void                                       *payload;
    uint32_t                                    payload_length;
    iotx_message_qos_t                          QoS;              
    void*                                       response_pcontext;              
    iotx_cloud_connection_msg_rsp_func_fpt      response_handler;  
    iotx_message_content_type_t                 content_type;
    iotx_message_type_t                         message_type;             
} iotx_cloud_connection_msg_t, *iotx_cloud_connection_msg_pt;


/**
 * @brief cloud clinet initial
 *        This function initialize the cloud connection structures, establish network connection
 *
 * @param Connection_param, specify the cloud Connection parameter and event handler.
 *
 * @return NULL, construct failed; NOT NULL, the handle of cloud Connection.
 */
void* IOT_Cloud_Connection_Init(iotx_cloud_connection_param_pt connection_param);


/**
 * @brief Deconstruct the cloud connection
 *        Disconnect Network connection and        release the related resource.
 *
 * @param pointer of handle, specify the cloud Connection.
 *
 * @return 0, deconstruct success; -1, deconstruct failed.
 */
int IOT_Cloud_Connection_Deinit(void** handle);


/**
 * @brief Deconstruct the cloud connection
 *        Disconnect Network connection and        release the related resource.
 *
 * @param pointer of handle, specify the cloud connection.
 *
 * @return 0, deconstruct success; -1, deconstruct failed.
 */
int IOT_Cloud_Connection_Send_Message(void* handle, iotx_cloud_connection_msg_pt msg);


/**
 * @brief Handle packet from remote point and process timeout request
 *
 * @param [in] handle: specify the cloud Connection.
 * @param [in] timeout_ms: specify the timeout in millisecond in this loop.
 *
 * @return status.
 * @see None.
 */
int IOT_Cloud_Connection_Yield(void *handle, int timeout_ms);


#endif /* SRC_SDK_IMPL_EXPORTS_IOT_EXPORT_CLOUD_Connection_H_ */




#ifndef SRC_CLOUD_CONNECTION_UTIL_H_
#define SRC_CLOUD_CONNECTION_UTIL_H_
  
#include "iot_export_cloud_connection.h"
#include "iotx_cloud_connection_mqtt.h"
#include "iotx_cloud_connection_coap.h"
#include "iotx_cloud_connection_http.h"

typedef void* (*iotx_cloud_connection_init_function)(void *param, void* connection);

typedef int (*iotx_cloud_connection_subscribe_function)(void* connection,
                        const char *topic_filter,
                        iotx_message_qos_t qos);

typedef int (*iotx_cloud_connection_unsubscribe_function)(void* connection,
                        const char *topic_filter);

typedef int (*iotx_cloud_connection_publish_function)(void* connection,
                        iotx_cloud_connection_msg_pt message);

typedef int (*iotx_cloud_connection_deinit_function)(void* connection);

typedef int (*iotx_cloud_connection_yield_function)(void* connection, int timeout_ms);

typedef int (*iotx_cloud_connection_resposne_func_fpt)(void* connection, iotx_cloud_connection_msg_rsp_pt msg);

/* The structure of cloud connection context */
typedef struct iotx_cloud_connection_function_st {
    void                                       *pcontext;  
    iotx_cloud_connection_init_function         init_func;
    iotx_cloud_connection_subscribe_function    sub_func;
    iotx_cloud_connection_unsubscribe_function  unsub_func;
    iotx_cloud_connection_publish_function      pub_func;
    iotx_cloud_connection_yield_function        yield_func;
    iotx_cloud_connection_deinit_function       deinit_func;
} iotx_cloud_connection_function_t, *iotx_cloud_connection_function_pt;

/* The structure of cloud connection context */
typedef struct iotx_cloud_connection_mapping_st {
    void                                       *next;                                         
    char                                       *topic;
    iotx_cloud_connection_msg_rsp_func_fpt      func;
    void                                       *pcontext;
} iotx_cloud_connection_mapping_t, *iotx_cloud_connection_mapping_pt;


/* The structure of cloud connection context */
typedef struct iotx_cloud_connection_st {
    iotx_cloud_connection_function_pt           function_pt;                 /* function */         
    iotx_cloud_connection_protocol_types_t      protocol_type;
    iotx_device_info_pt                         device_info;                 /* todo: it is need or not*/
    uint8_t                                     clean_session;               /* MQTT: clean_session,  HTTP: keep_alive*/         
    uint32_t                                    request_timeout_ms;               
    void*                                       event_pcontext;              
    iotx_cloud_connection_event_handle_func_fpt event_handler;      
    iotx_cloud_connection_resposne_func_fpt     resposne_handler;
    iotx_cloud_connection_mapping_pt            mapping_list;
} iotx_cloud_connection_t, *iotx_cloud_connection_pt;

extern iotx_cloud_connection_pt g_cloud_connection_t;

int iotx_cloud_connection_check_function_context(iotx_cloud_connection_pt connection);

int iotx_cloud_connection_resposne(void* handle, iotx_cloud_connection_msg_rsp_pt msg);\

int iotx_cloud_connection_add_mapping(iotx_cloud_connection_pt connection, char* topic, iotx_cloud_connection_msg_rsp_func_fpt func, void* pcontext);

int iotx_cloud_connection_remove_mapping(iotx_cloud_connection_pt connection, char* topic);

int iotx_cloud_connection_remove_mapping_all(iotx_cloud_connection_pt connection);

iotx_cloud_connection_mapping_pt iotx_cloud_connection_find_mapping(iotx_cloud_connection_pt connection, char* topic);

#endif /* SRC_CLOUD_CONNECTION_UTIL_H_ */

#ifndef DM_ABSTRACT_H
#define DM_ABSTRACT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <stdarg.h>

/* callback funtion type:
 * 1. property value set 2. service triggered 3. cloud connected 4. cloud disconnected
 * 5. new_thing_created  6. thing disable     7. thing enable    8. raw data arrived. */

/* patterns: */
/* property value set:
 * thing_id, property_service_identifier(concern)    request_id(not concern)  raw_data(not concern) raw_data_length(concern) */
/* service_requested:
 * thing_id, property_service_identifier(concern)     request_id(concern)     raw_data(not concern) raw_data_length(concern) */
/* cloud [dis]connected:
 * thing_id, property_service_identifier(not concern) request_id(not concern) raw_data(not concern) raw_data_length(concern) */
/* new_thing_created:
 * thing_id, property_service_identifier(not concern) request_id(not concern) raw_data(not concern) raw_data_length(concern) */
/* thing_disabled:
 * thing_id, property_service_identifier(not concern) request_id(not concern) raw_data(not concern) raw_data_length(concern) */
/* thing_enabled:
 * thing_id, property_service_identifier(not concern) request_id(not concern) raw_data(not concern) raw_data_length(concern) */
/* raw_data_arrived:
 * thing_id, property_service_identifier(not concern) request_id(not concern) raw_data(concern)     raw_data_length(concern) */

typedef enum {
    dm_callback_type_property_value_set = 0,
    dm_callback_type_service_requested,
    dm_callback_type_cloud_connected,
    dm_callback_type_cloud_disconnected,
    dm_callback_type_new_thing_created,
    dm_callback_type_thing_disabled,
    dm_callback_type_thing_enabled,
    dm_callback_type_raw_data_arrived,

    dm_callback_type_number,
} dm_callback_type_t;

typedef void (*handle_dm_callback_fp_t)(dm_callback_type_t callback_type, void* thing_id,
                                        const char* property_service_identifier, int request_id,
                                        void* raw_data, int raw_data_length);

/* domain type */
typedef enum {
    /* iot-as-mqtt.cn-shanghai.aliyuncs.com */
    dm_cloud_domain_sh,
    /* USA */
    dm_cloud_domain_usa,

    dm_cloud_domain_max,
} dm_cloud_domain_type_t;

typedef struct {
    size_t size;
    const char*  _class_name;
    void* (*ctor)(void* _self, va_list* params);
    void* (*dtor)(void* _self);
    void* (*generate_new_thing)(void* _self, const char* tsl, int tsl_len);
    int   (*set_property_value)(void* _self, const void* thing_id, const void* identifier, const void* value, const char* value_str);
    int   (*set_event_output_value)(void* _self, const void* thing_id, const void* identifier, const void* value, const char* value_str);
    int   (*set_service_output_value)(void* _self, const void* thing_id, const void* identifier, const void* value, const char* value_str);
    int   (*get_property_value)(const void* _self, const void* thing_id, const void* identifier, void* value, char** value_str);
    int   (*get_service_input_value)(const void* _self, const void* thing_id, const void* identifier, void* value, char** value_str);
    int   (*get_service_output_value)(const void* _self, const void* thing_id, const void* identifier, void* value, char** value_str);
    int   (*get_event_output_value)(const void* _self, const void* thing_id, const void* identifier, void* value, char** value_str);
    int   (*install_callback_function)(void* _self, handle_dm_callback_fp_t linkkit_callback_fp);
    int   (*trigger_event)(const void* _self, const void* thing_id, const void* event_identifier, const char* property_identifier);
    int   (*answer_service)(const void* _self, const void* thing_id, const void* identifier, int response_id, int code);
    int   (*invoke_raw_service)(const void* _self, const void* thing_id, void* raw_data, int raw_data_length);
    int   (*answer_raw_service)(const void* _self, const void* thing_id, void* raw_data, int raw_data_length);
#ifndef CMP_SUPPORT_MULTI_THREAD
    int   (*yield)(const void* _self, int timeout_ms);
#endif
} dm_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DM_ABSTRACT_H */

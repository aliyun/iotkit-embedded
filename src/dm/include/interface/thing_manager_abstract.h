#ifndef THING_MANAGER_ABSTRACT_H
#define THING_MANAGER_ABSTRACT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <stdarg.h>

#include "iot_export_dm.h"

typedef struct {
    size_t size;
    const char*  _class_name;
    void* (*ctor)(void* _self, va_list* params);
    void* (*dtor)(void* _self);
    void* (*generate_new_local_thing)(void* _self, const char* tsl, int tsl_len);
    int   (*add_callback_function)(void* _self, handle_dm_callback_fp_t callback_func);
    int   (*set_thing_property_value)(void* _self, const void* thing_id, const void* identifier, const void* value, const char* value_str);
    int   (*get_thing_property_value)(void* _self, const void* thing_id, const void* identifier, void* value, char** value_str);
    int   (*set_thing_event_output_value)(void* _self, const void* thing_id, const void* identifier, const void* value, const char* value_str);
    int   (*get_thing_event_output_value)(void* _self, const void* thing_id, const void* identifier, void* value, char** value_str);
    int   (*get_thing_service_input_value)(void* _self, const void* thing_id, const void* identifier, void* value, char** value_str);
    int   (*get_thing_service_output_value)(void* _self, const void* thing_id, const void* identifier, void* value, char** value_str);
    int   (*set_thing_service_output_value)(void* _self, const void* thing_id, const void* identifier, const void* value, const char* value_str);
    int   (*trigger_event)(void* _self, const void* thing_id, const void* event_identifier, const char* property_identifier);
    int   (*answer_service)(void* _self, const void* thing_id, const void* identifier, int response_id, int code);
    int   (*invoke_raw_service)(void* _self, const void* thing_id, void* raw_data, int raw_data_length);
    int   (*answer_raw_service)(void* _self, const void* thing_id, void* raw_data, int raw_data_length);
#ifndef CMP_SUPPORT_MULTI_THREAD
    int   (*yield)(void* _self, int timeout);
#endif
} thing_manager_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* THING_MANAGER_ABSTRACT_H */

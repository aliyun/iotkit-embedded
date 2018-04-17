#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "interface/thing_manager_abstract.h"
#include "interface/message_info_abstract.h"
#include "interface/cmp_abstract.h"
#include "dm_thing_manager.h"
#include "logger.h"
#include "dm_thing.h"
#include "single_list.h"
#include "dm_import.h"
#include "cmp_message_info.h"
#include "cmp_abstract_impl.h"

#include "iot_import.h"
#include "iot_export.h"
#include "class_interface.h"

#include "cJSON.h"


static const char string_dm_thing_manager_class_name[] __DM_READ_ONLY__ = "dm_thg_mng_cls";
static const char string_thing_service_property_set[] __DM_READ_ONLY__ = "thing.service.property.set";
static const char string_thing_service_property_get[] __DM_READ_ONLY__ = "thing.service.property.get";
static const char string_thing_event_property_post[] __DM_READ_ONLY__ = "thing.event.property.post";
static const char string_method_name_thing_enable[] __DM_READ_ONLY__ = METHOD_NAME_THING_ENABLE;
static const char string_method_name_thing_disable[] __DM_READ_ONLY__ = METHOD_NAME_THING_DISABLE;
static const char string_method_name_thing_delete[] __DM_READ_ONLY__ = METHOD_NAME_THING_DELETE;
static const char string_method_name_thing_dsl_post_reply[] __DM_READ_ONLY__ = METHOD_NAME_THING_DSL_POST_REPLY;
static const char string_method_name_thing_dsl_get[] __DM_READ_ONLY__ = METHOD_NAME_THING_DSL_GET;
static const char string_method_name_thing_dsl_get_reply[] __DM_READ_ONLY__ = METHOD_NAME_THING_DSL_GET_REPLY;
static const char string_method_name_down_raw[] __DM_READ_ONLY__ = METHOD_NAME_DOWN_RAW;
static const char string_method_name_down_raw_reply[] __DM_READ_ONLY__ = METHOD_NAME_DOWN_RAW_PEPLY;
static const char string_method_name_up_raw[] __DM_READ_ONLY__ = METHOD_NAME_UP_RAW;
static const char string_method_name_up_raw_reply[] __DM_READ_ONLY__ = METHOD_NAME_UP_RAW_REPLY;
static const char string_method_name_property_set[] __DM_READ_ONLY__ = METHOD_NAME_PROPERTY_SET;
static const char string_method_name_property_get[] __DM_READ_ONLY__ = METHOD_NAME_PROPERTY_GET;
#ifdef DEVICEINFO_ENABLED
static const char string_method_name_deviceinfo_update[] __DM_READ_ONLY__ = METHOD_NAME_DEVICEINFO_UPDATE;
static const char string_method_name_deviceinfo_update_reply[] __DM_READ_ONLY__ = METHOD_NAME_DEVICEINFO_UPDATE_REPLY;
static const char string_method_name_deviceinfo_delete[] __DM_READ_ONLY__ = METHOD_NAME_DEVICEINFO_DELETE;
static const char string_method_name_deviceinfo_delete_reply[] __DM_READ_ONLY__ = METHOD_NAME_DEVICEINFO_DELETE_REPLY;
#endif /* DEVICEINFO_ENABLED */
#ifdef RRPC_ENABLED
static const char string_method_name_rrpc_request_plus[] __DM_READ_ONLY__ = METHOD_NAME_RRPC_REQUEST_PLUS;
static const char string_method_name_rrpc_request[] __DM_READ_ONLY__ = METHOD_NAME_RRPC_REQUEST;
#endif /* RRPC_ENABLED */
static const char string_cmp_event_handler_prompt_start[] __DM_READ_ONLY__ = "\ncmp_event_handler:\n###\n";
static const char string_cmp_event_handler_prompt_end[] __DM_READ_ONLY__ = "\n###\n";
static const char string_cmp_event_type_cloud_connected[] __DM_READ_ONLY__ = "cloud connected";
static const char string_cmp_event_type_cloud_disconnect[] __DM_READ_ONLY__ = "cloud disconnect";
static const char string_cmp_event_type_cloud_reconnect[] __DM_READ_ONLY__ = "cloud reconnect";
static const char string_cmp_event_type_found_device[] __DM_READ_ONLY__ = "found device";
static const char string_cmp_event_type_remove_device[] __DM_READ_ONLY__ = "remove device";
static const char string_cmp_event_type_register_result[] __DM_READ_ONLY__ = "register result";
static const char string_cmp_event_type_unregister_result[] __DM_READ_ONLY__ = "unregister result";
static const char string_cmp_event_type_send_result[] __DM_READ_ONLY__ = "send result";
static const char string_cmp_event_type_new_data_received[] __DM_READ_ONLY__ = "new data received";

static const char string_local_thing_list[] __DM_READ_ONLY__ = "local thing";
static const char string_local_thing_name_list[] __DM_READ_ONLY__ = "local thing name";
static const char string_sub_thing_list[] __DM_READ_ONLY__ = "sub thing";
static const char string_callback_list[] __DM_READ_ONLY__ = "callback list";
static const char string_service_property_get_identifier_list[] __DM_READ_ONLY__ = "service property get id list";

static const char string_success[] __DM_READ_ONLY__ = "success";
static const char string_fail[] __DM_READ_ONLY__ = "fail";
static const char string_uri_type_sys[] __DM_READ_ONLY__ = "SYS";
static const char string_uri_type_ext[] __DM_READ_ONLY__ = "EXT";
static const char string_uri_type_undefine[] __DM_READ_ONLY__ = "UNDEFINE";
static const char string_request[] __DM_READ_ONLY__ = "request";
static const char string_response[] __DM_READ_ONLY__ = "response";
static const char string_event[] __DM_READ_ONLY__ = "event";
static const char string_service[] __DM_READ_ONLY__ = "service";

static void free_list_string(void* _thing_name, va_list* params);
static void free_list_thing(void* _thing_name, va_list* params);
static void local_thing_list_iterator(const void* _self, handle_fp_t handle_fp);
static void invoke_callback_func(void* _callback_fp, va_list* params);
static void callback_list_iterator(const void* _self, handle_fp_t handle_fp);
static void generate_subscribe_uri(void* _dm_thing_manager, void* _thing);
static void send_request_to_uri(void* _dm_thing_manager, const char *_uri);
static void clear_and_set_message_info(message_info_t** _message_info, dm_thing_manager_t* _dm_thing_manager);
static void get_product_key_device_name(char* _product_key, char* _device_name, void* _thing, void* _dm_thing_manager);
static void* dm_thing_manager_generate_new_local_thing(void* _self, const char* tsl, int tsl_len);
static int dm_thing_manager_set_thing_property_value(void* _self, const void* thing_id, const void* identifier, const void* value, const char* value_str);

static int dm_thing_manager_install_product_key_device_name(void *_self, const void* thing_id, char *product_key, char *device_name);

#ifdef RRPC_ENABLED
static int dm_thing_manager_answer_service(void* _self, const void* thing_id, const void* identifier, int response_id, int code, int rrpc);
#else
static int dm_thing_manager_answer_service(void* _self, const void* thing_id, const void* identifier, int response_id, int code);
#endif /* RRPC_ENABLED */

static void cmp_register_handler(iotx_cmp_send_peer_t* _source, iotx_cmp_message_info_t* _msg, void* user_data);

static void list_insert(void* _list, void* _data)
{
    list_t** list = (list_t**)_list;

    (*list)->insert(list, _data);
}

static void invoke_callback_list(void* _dm_thing_manager, dm_callback_type_t dm_callback_type)
{
    dm_thing_manager_t* dm_thing_manager = _dm_thing_manager;
    list_t** list;

    assert(dm_callback_type < dm_callback_type_number);

    /* invoke callback funtions. */
    list = dm_thing_manager->_callback_list;
    if (list && !(*list)->empty(list)) {
        dm_thing_manager->_callback_type = dm_callback_type;
        callback_list_iterator(dm_thing_manager, invoke_callback_func);
    }
}

static void local_thing_generate_subscribe_uri(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    assert(dm_thing_manager && thing && *thing);

    /* subscribe subjects. */
    generate_subscribe_uri(dm_thing_manager, thing);
}

static void cmp_event_handler(void* pcontext, iotx_cmp_event_msg_t* msg, void* user_data)
{
    iotx_cmp_event_result_t* cmp_event_result;
    dm_thing_manager_t* dm_thing_manager = user_data;
    const char* event_str = NULL;

    dm_printf("%s", string_cmp_event_handler_prompt_start);
    if (IOTX_CMP_EVENT_REGISTER_RESULT == msg->event_id) {
        iotx_cmp_event_result_t* cmp_event_result = (iotx_cmp_event_result_t*)msg->msg;

        dm_log_info("register result:\tresult: %d(%s)\tURI: %s\tURI_type:%s",
                    cmp_event_result->result, cmp_event_result->result == 0 ? string_success : string_fail,
                    cmp_event_result->URI, cmp_event_result->URI_type == IOTX_CMP_URI_SYS ?
                        string_uri_type_sys : (cmp_event_result->URI_type == IOTX_CMP_URI_EXT ? string_uri_type_ext : string_uri_type_undefine));

        event_str = string_cmp_event_type_register_result;
    } else if (IOTX_CMP_EVENT_UNREGISTER_RESULT == msg->event_id) {
        cmp_event_result = (iotx_cmp_event_result_t*)msg->msg;

        dm_log_info("un-register result:\tresult: %s,\tURI: %s,\tURI_type:%s", cmp_event_result->result,
                    cmp_event_result->URI, cmp_event_result->URI_type == IOTX_CMP_URI_SYS ?
                        string_uri_type_sys : (cmp_event_result->URI_type == IOTX_CMP_URI_EXT ? string_uri_type_ext : string_uri_type_undefine));

        event_str = string_cmp_event_type_unregister_result;
    } else if (IOTX_CMP_EVENT_CLOUD_DISCONNECT == msg->event_id) {
        if (dm_thing_manager->_cloud_connected) {
            dm_thing_manager->_cloud_connected = 0;
            invoke_callback_list(dm_thing_manager, dm_callback_type_cloud_disconnected);
        }
        event_str = string_cmp_event_type_cloud_disconnect;
    } else if (IOTX_CMP_EVENT_CLOUD_RECONNECT == msg->event_id) {
        if (dm_thing_manager->_cloud_connected == 0) {
            dm_thing_manager->_cloud_connected = 1;
            invoke_callback_list(dm_thing_manager, dm_callback_type_cloud_connected);
        }
        event_str = string_cmp_event_type_cloud_reconnect;
    }  else if (IOTX_CMP_EVENT_CLOUD_CONNECTED == msg->event_id) {
        if (dm_thing_manager->_cloud_connected == 0) {
            dm_thing_manager->_cloud_connected = 1;

            if (dm_thing_manager->_destructing == 1) return;
            /* subscribe uris not related to tsl. */
            generate_subscribe_uri(dm_thing_manager, NULL);

            if (dm_thing_manager->_get_tsl_from_cloud) {
                /* get tsl template. */
                send_request_to_uri(dm_thing_manager, string_method_name_thing_dsl_get);
            }

            local_thing_list_iterator(dm_thing_manager, local_thing_generate_subscribe_uri);

            invoke_callback_list(dm_thing_manager, dm_callback_type_cloud_connected);
        }
        event_str = string_cmp_event_type_cloud_connected;
    } else if (IOTX_CMP_EVENT_FOUND_DEVICE == msg->event_id) {
        /* currently do not process this. */
        event_str = string_cmp_event_type_found_device;
    } else if (IOTX_CMP_EVENT_REMOVE_DEVICE == msg->event_id) {
        /* currently do not process this. */
        event_str = string_cmp_event_type_remove_device;
    } else if (IOTX_CMP_EVENT_SEND_RESULT == msg->event_id) {
        event_str = string_cmp_event_type_send_result;
    } else if (IOTX_CMP_EVENT_NEW_DATA_RECEIVED == msg->event_id) {
        iotx_cmp_new_data_t* new_data = (iotx_cmp_new_data_t*)msg->msg;
        cmp_register_handler(new_data->peer, new_data->message_info, dm_thing_manager);

        event_str = string_cmp_event_type_new_data_received;
    } else {
        dm_printf("unknown event %d\n", msg->event_id);
    }

    dm_printf("\nevent %d(%s)%s", msg->event_id, event_str, string_cmp_event_handler_prompt_end);
}

static void print_iotx_cmp_message_info(const void* _source, const void* _msg)
{
    const iotx_cmp_send_peer_t* iotx_cmp_send_peer = _source;
    const iotx_cmp_message_info_t* iotx_cmp_message_info = _msg;

    dm_printf("***\nsource\t%s:%s\n", iotx_cmp_send_peer->product_key, iotx_cmp_send_peer->device_name);

    dm_printf("type\t%s\n", iotx_cmp_message_info->message_type == IOTX_CMP_MESSAGE_RAW ?
                  "RAW" : (iotx_cmp_message_info->message_type == IOTX_CMP_MESSAGE_REQUEST ? string_request : string_response));
    dm_printf("URI\t%s\n", iotx_cmp_message_info->URI);

    dm_printf("URI_type\t%s\n", iotx_cmp_message_info->URI_type == IOTX_CMP_URI_SYS ?
                  string_uri_type_sys : (iotx_cmp_message_info->URI_type == IOTX_CMP_URI_EXT ? string_uri_type_ext : string_uri_type_undefine));

    dm_printf("code\t%d\n", iotx_cmp_message_info->code);
    dm_printf("id\t%d\n", iotx_cmp_message_info->id);

    if (iotx_cmp_message_info->method)
        dm_printf("method\t%s\n", iotx_cmp_message_info->method);
    else
        dm_printf("method\tNULL\n");

    dm_printf("param:\n%s\n", (char*)iotx_cmp_message_info->parameter);
    dm_printf("param len:\t%dbytes\n", iotx_cmp_message_info->parameter_length);
    dm_printf("***\n");
}

static int check_set_lite_property_for_struct(cJSON* cjson_obj, lite_property_t *lite_property)
{
    int i;
    lite_property_t* sub_property;
    cJSON *subobject;
    for(i = 0; i < lite_property->data_type.data_type_specs_number; i++) {
        sub_property = (lite_property_t*)lite_property->data_type.specs + i;
        subobject = cJSON_GetObjectItem(cjson_obj, sub_property->identifier);
        if(!subobject)
            return -1;
        if(data_type_type_struct == sub_property->data_type.type) {
            if(check_set_lite_property_for_struct(subobject, sub_property)) {
                return -1;
            }
        }
    }
    return 0;
}

static void find_and_set_array_item(dm_thing_manager_t* dm_thing_manager,thing_t** thing, cJSON* cjson_arr_obj, int item_index, lite_property_t *lite_property,data_type_type_t type)
{
    int int_val;
    float float_val;
    double double_val;
    char temp_buf[64] = {0};
    char identifier[64] = {0};
    char* string_val = NULL;
    cJSON *arr_json_item;
    if(item_index > lite_property->data_type.value.data_type_array_t.size) {
        dm_printf("input json array item:%d > lite json array size:%d ", item_index, lite_property->data_type.value.data_type_array_t.size);
        return;
    }

    arr_json_item = cJSON_GetArrayItem(cjson_arr_obj,item_index);
    if(!arr_json_item) {
        dm_log_err("get array[%d] failed", item_index);
        goto do_exit;
    }
    switch(lite_property->data_type.value.data_type_array_t.item_type) {
        case data_type_type_int: {
            int_val = arr_json_item->valueint;
            dm_snprintf(temp_buf, sizeof(temp_buf), "%d", int_val);
        }break;
        case data_type_type_double: {
                double_val = arr_json_item->valuedouble;
                dm_snprintf(temp_buf, sizeof(temp_buf), "%.16lf", double_val);
        }break;
        case data_type_type_float: {
            float_val = arr_json_item->valuedouble;
            dm_snprintf(temp_buf, sizeof(temp_buf), "%.7f", float_val);
        }break;
        case data_type_type_text: {
            string_val = dm_lite_calloc(1, strlen(arr_json_item->valuestring) + 1);
            assert(string_val);
            if (string_val == NULL) {
                dm_printf("NO memory...");
                return;
            }
            strcpy(string_val, arr_json_item->valuestring);
        }break;
        default:{
            dm_log_err("don't support %d type", lite_property->data_type.value.data_type_array_t.item_type);
            goto do_exit;
        }
    }
    dm_thing_manager->_property_identifier_value_set = NULL;
    if (string_val) {
        dm_thing_manager->_property_identifier_value_set = string_val;
    } else if (strlen(temp_buf)) {
        dm_thing_manager->_property_identifier_value_set = dm_lite_calloc(1, strlen(temp_buf) + 1);
        assert(dm_thing_manager->_property_identifier_value_set);
        strcpy(dm_thing_manager->_property_identifier_value_set, temp_buf);
    }

    if (dm_thing_manager->_property_identifier_set && dm_thing_manager->_property_identifier_value_set) {
        dm_snprintf(identifier, sizeof(identifier), "%s[%d]",(char*)dm_thing_manager->_property_identifier_set, item_index);
        dm_thing_manager_set_thing_property_value(dm_thing_manager, thing, identifier,
                                                  NULL, dm_thing_manager->_property_identifier_value_set);

        dm_lite_free(dm_thing_manager->_property_identifier_value_set);
        dm_thing_manager->_property_identifier_value_set = NULL;
    }
do_exit:
    return;
}


static void find_and_set_lite_property_struct_for_service_property_set(void* _item, ...);
static void find_and_set_lite_property_for_service_property_set(void* _item, int _index, va_list* params)
{
    property_t* property = _item;
    lite_property_t* lite_property;
    lite_property_t* lite_property_struct;
    dm_thing_manager_t* dm_thing_manager;
    thing_t** thing;
    cJSON* property_set_param_obj;
    cJSON* temp_cjson_obj;
    char temp_buf[64] = {0};
    int int_val;
    float float_val;
    double double_val;
    char* string_val = NULL;
    char* identifier_prefix = NULL;
    char identifier_buff[128] = {0};
    int index;
    int arrsize = 0;

    dm_thing_manager = va_arg(*params, dm_thing_manager_t*);
    property_set_param_obj = va_arg(*params, cJSON*);
    thing = va_arg(*params, thing_t**);
    identifier_prefix = va_arg(*params, char*);

    _index = _index;

    assert(property && dm_thing_manager && property_set_param_obj && thing && *thing);

    lite_property = (lite_property_t*)property;

    temp_cjson_obj = cJSON_GetObjectItem(property_set_param_obj, lite_property->identifier);
    if (temp_cjson_obj) {
        if (identifier_prefix) {
            dm_snprintf(identifier_buff, sizeof(identifier_buff), "%s%c%s", identifier_prefix, DEFAULT_DSL_DELIMITER, lite_property->identifier);
            dm_thing_manager->_property_identifier_set = identifier_buff;
        } else {
            dm_thing_manager->_property_identifier_set = lite_property->identifier;
        }

        if (lite_property->data_type.type == data_type_type_text) {
#ifndef CJSON_STRING_ZEROCOPY
            string_val = dm_lite_calloc(1, strlen(temp_cjson_obj->valuestring) + 1);
            assert(string_val);
            if (string_val == NULL) {
                dm_printf("NO memory...");
                return;
            }
            strcpy(string_val, temp_cjson_obj->valuestring);
#else
            string_val = dm_lite_calloc(1, temp_cjson_obj->valuestring_length + 1);
            assert(string_val);
            if (string_val == NULL) {
                dm_printf("NO memory...");
                return;
            }
            strncpy(string_val, temp_cjson_obj->valuestring, temp_cjson_obj->valuestring_length);
#endif
        } else if (lite_property->data_type.type == data_type_type_float) {
            float_val = temp_cjson_obj->valuedouble;
            dm_snprintf(temp_buf, sizeof(temp_buf), "%.7f", float_val);
        } else if (lite_property->data_type.type == data_type_type_double) {
            double_val = temp_cjson_obj->valuedouble;
            dm_snprintf(temp_buf, sizeof(temp_buf), "%.16lf", double_val);
        } else if (lite_property->data_type.type == data_type_type_date) {
            if(temp_cjson_obj->type == cJSON_String) {
                dm_snprintf(temp_buf, sizeof(temp_buf), "%s", temp_cjson_obj->valuestring);
            }else if(temp_cjson_obj->type == cJSON_Number) {
                dm_snprintf(temp_buf, sizeof(temp_buf), "%lf", temp_cjson_obj->valuedouble);
            }
        } else if (lite_property->data_type.type == data_type_type_enum ||
                   lite_property->data_type.type == data_type_type_bool ||
                   lite_property->data_type.type == data_type_type_int) {

            int_val = temp_cjson_obj->valueint;
            dm_snprintf(temp_buf, sizeof(temp_buf), "%d", int_val);
        } else if (lite_property->data_type.type == data_type_type_struct) {
            assert(cJSON_IsObject(temp_cjson_obj));
            if (!cJSON_IsObject(temp_cjson_obj)) return;
            if(check_set_lite_property_for_struct(temp_cjson_obj, lite_property)) {
                dm_printf("invalid json");
                return;
            }

            for (index = 0; index < lite_property->data_type.data_type_specs_number; ++index) {
                lite_property_struct = (lite_property_t*)lite_property->data_type.specs + index;
                find_and_set_lite_property_struct_for_service_property_set(lite_property_struct, dm_thing_manager,
                                                                           temp_cjson_obj, thing, lite_property->identifier);
            }
            dm_thing_manager->_identifier = lite_property->identifier;
            invoke_callback_list(dm_thing_manager, dm_callback_type_property_value_set);
        } else if(lite_property->data_type.type == data_type_type_array) {
            if(!cJSON_IsArray(temp_cjson_obj)) {
                dm_log_err("json type is not array");
                return;
            }
            arrsize = cJSON_GetArraySize(temp_cjson_obj);
            if(arrsize > lite_property->data_type.value.data_type_array_t.size) {
                dm_printf("input json array item:%d > lite json array item:%d ", arrsize, lite_property->data_type.value.data_type_array_t.size);
                return;
            }
            for(index = 0; index < arrsize; index++){
                find_and_set_array_item(dm_thing_manager,thing, temp_cjson_obj, index, lite_property, lite_property->data_type.type);
            }
            dm_thing_manager->_identifier = lite_property->identifier;
            invoke_callback_list(dm_thing_manager, dm_callback_type_property_value_set);
            return;
        }

        if (string_val) {
            dm_thing_manager->_property_identifier_value_set = string_val;
        } else if (strlen(temp_buf)) {
            dm_thing_manager->_property_identifier_value_set = dm_lite_calloc(1, strlen(temp_buf) + 1);
            assert(dm_thing_manager->_property_identifier_value_set);
            strcpy(dm_thing_manager->_property_identifier_value_set, temp_buf);
        }
    }

    if (dm_thing_manager->_property_identifier_set && dm_thing_manager->_property_identifier_value_set) {
        dm_thing_manager_set_thing_property_value(dm_thing_manager, thing, dm_thing_manager->_property_identifier_set,
                                                  NULL, dm_thing_manager->_property_identifier_value_set);

        dm_lite_free(dm_thing_manager->_property_identifier_value_set);
        dm_thing_manager->_property_identifier_value_set = NULL;
    }
}

static void find_and_set_lite_property_struct_for_service_property_set(void* _item, ...)
{
    va_list params;

    va_start(params, _item);

    find_and_set_lite_property_for_service_property_set(_item, 0, &params);

    va_end(params);
}

static void find_thing_via_product_key_and_device_name(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;
    iotx_cmp_send_peer_t* iotx_cmp_send_peer;

    char product_key[PRODUCT_KEY_MAXLEN] = {0};
    char device_name[DEVICE_NAME_MAXLEN] = {0};

    dm_thing_manager = va_arg(*params, void*);
    iotx_cmp_send_peer = va_arg(*params, iotx_cmp_send_peer_t*);

    assert(dm_thing_manager && thing && *thing);

    dm_thing_manager_install_product_key_device_name(dm_thing_manager,thing,product_key,device_name);

    if (strcmp(product_key, iotx_cmp_send_peer->product_key) == 0 && strcmp(device_name, iotx_cmp_send_peer->device_name) == 0) {
        dm_thing_manager->_thing_id = thing;
    }
}

static int set_service_array_input(dm_thing_manager_t* _dm_thing_manager, thing_t** thing, cJSON* cjson_arr_obj, int item_index,lite_property_t *lite_property, data_type_type_t type, char *_identifier)
{
    int ret = -1;
    int int_val;
    float float_val;
    double double_val;
    char temp_buf[64] = {0};
    char identifier[64] = {0};
    cJSON *arr_json_item;

    if(!_dm_thing_manager || !thing || !cjson_arr_obj || item_index < 0 || !lite_property || !_identifier)   {
        dm_log_err("invalid params");
        goto do_exit;
    }
    if (item_index > lite_property->data_type.value.data_type_array_t.size) {
        dm_printf("input json array item:%d > lite json array size:%d ", item_index, lite_property->data_type.value.data_type_array_t.size);
        goto do_exit;
    }


    arr_json_item = cJSON_GetArrayItem(cjson_arr_obj,item_index);
    if (!arr_json_item) {
        dm_log_err("get array[%d] failed", item_index);
        goto do_exit;
    }
    switch(lite_property->data_type.value.data_type_array_t.item_type) {
    case data_type_type_int: {
        int_val = arr_json_item->valueint;
        dm_snprintf(temp_buf, sizeof(temp_buf), "%d", int_val);
    }break;
    case data_type_type_double: {
        double_val = arr_json_item->valuedouble;
        dm_snprintf(temp_buf, sizeof(temp_buf), "%.16lf", double_val);
    }break;
    case data_type_type_float: {
        float_val = arr_json_item->valuedouble;
        dm_snprintf(temp_buf, sizeof(temp_buf), "%.7f", float_val);
    }break;
    case data_type_type_text: {
        dm_snprintf(temp_buf, sizeof(temp_buf), "%s", arr_json_item->valuestring);
    }break;
    default:{
        dm_log_err("don't support %d type", lite_property->data_type.value.data_type_array_t.item_type);
        goto do_exit;
    }
    }

    if (strlen(temp_buf) > 0) {
        dm_snprintf(identifier, sizeof(identifier), "%s[%d]", _identifier, item_index);
        (*thing)->set_service_input_output_data_value_by_identifier(thing, identifier, NULL, temp_buf);
        ret = 0;
    }
do_exit:
    return ret;
}

static int parse_and_set_service_input(dm_thing_manager_t *_dm_thing_manager,thing_t **thing, service_t *service, char* parameter)
{
    cJSON* obj = NULL;
    cJSON* item;
    int i;
    char identifier[128] = {0};
    char* string_val;
    int arr_size = 0;
    int arr_index = 0;
    input_data_t* service_input_data;
    lite_property_t* property;

    dm_thing_manager_t* dm_thing_manager = _dm_thing_manager;

    obj = cJSON_Parse(parameter);
    if (!obj) return - 1;

    for (i = 0; i < service->service_input_data_num; i++) {
        service_input_data = &service->service_input_data[i]; /* inputData */

        property = &service_input_data->lite_property;

        item = cJSON_GetObjectItem(obj, property->identifier);
        if (!item) continue;

        snprintf(identifier, sizeof(identifier), "%s.%s", service->identifier, property->identifier);

        switch (property->data_type.type) {
        case data_type_type_text:
        {
#ifndef CJSON_STRING_ZEROCOPY
            string_val = dm_lite_calloc(1, strlen(item->valuestring) + 1);

            if (string_val == NULL) {
                if (obj) cJSON_Delete(obj);
                return -1;
            }
            strcpy(string_val, item->valuestring);
#else
            string_val = dm_lite_calloc(1, item->valuestring_length + 1);

            if (string_val == NULL) {
                cJSON_Delete(obj);
                return -1;
            }
            strncpy(string_val, item->valuestring, item->valuestring_length);
#endif /* CJSON_STRING_ZEROCOPY */
            (*thing)->set_service_input_output_data_value_by_identifier(thing, identifier, NULL, string_val);

            dm_lite_free(string_val);
        }
            break;
        case data_type_type_enum:
        case data_type_type_bool:
        case data_type_type_int:
            (*thing)->set_service_input_output_data_value_by_identifier(thing, identifier, &item->valueint, NULL);
            break;
        case data_type_type_float:
        case data_type_type_double:
        case data_type_type_date:
            (*thing)->set_service_input_output_data_value_by_identifier(thing, identifier, &item->valuedouble, NULL);
            break;
        case data_type_type_array:
            arr_size = cJSON_GetArraySize(item);
            for(arr_index = 0; arr_index < arr_size; arr_index++) {
                set_service_array_input(dm_thing_manager, thing, item, arr_index, property, property->data_type.type, identifier);
            }
            break;
        default:
            break;
        }
    }

    if (obj) cJSON_Delete(obj);

    return 0;
}

static void find_and_set_service_input(void* _item, int index, va_list* params)
{
    service_t* service;
    thing_t **thing;
    dm_thing_manager_t* dm_thing_manager;
    iotx_cmp_message_info_t* iotx_cmp_message_info;
    char* parameter = NULL;

    dm_thing_manager = va_arg(*params, void*);
    iotx_cmp_message_info = va_arg(*params, iotx_cmp_message_info_t*);
    thing = va_arg(*params, thing_t**);

    service = _item;

    assert(dm_thing_manager && iotx_cmp_message_info && service);

    if (strcmp(service->method, iotx_cmp_message_info->method) == 0) {
        dm_thing_manager->_service_identifier_requested = service->identifier;

        if (strstr(iotx_cmp_message_info->URI, string_method_name_property_set) != NULL || strstr(iotx_cmp_message_info->URI, string_method_name_property_get) != NULL) return;

        parameter = iotx_cmp_message_info->parameter;
        if (parameter) parse_and_set_service_input(dm_thing_manager, thing, service, parameter);
    }
}

static void cmp_register_handler(iotx_cmp_send_peer_t* _source, iotx_cmp_message_info_t* _msg, void* user_data)
{
    dm_thing_manager_t* dm_thing_manager = user_data;
    iotx_cmp_send_peer_t* iotx_cmp_send_peer = _source;
    iotx_cmp_message_info_t* iotx_cmp_message_info = _msg;
    thing_t** thing;
    list_t** list;
    cJSON* property_set_param_obj;
    cJSON* property_get_param_obj;
    cJSON* property_get_param_item_obj;
    int array_size, index;
    char* property_get_param_identifier;

    assert(dm_thing_manager && iotx_cmp_send_peer && iotx_cmp_message_info);

    print_iotx_cmp_message_info(iotx_cmp_send_peer, iotx_cmp_message_info);

    list = dm_thing_manager->_local_thing_list;

    /* find thing id. */
    dm_thing_manager->_thing_id = NULL;
    list_iterator(list, find_thing_via_product_key_and_device_name, dm_thing_manager, iotx_cmp_send_peer);

    if (dm_thing_manager->_thing_id == NULL && strstr(iotx_cmp_message_info->URI, string_method_name_thing_dsl_get_reply) == NULL) {
        dm_log_err("thing id NOT match");

        return;
    }

    thing = dm_thing_manager->_thing_id;

    if (iotx_cmp_message_info->message_type == IOTX_CMP_MESSAGE_REQUEST) {
        dm_thing_manager->_request_id = iotx_cmp_message_info->id;

        /* find service id trigged. */
        dm_thing_manager->_service_identifier_requested = NULL;
        service_iterator(thing, find_and_set_service_input, dm_thing_manager, iotx_cmp_message_info, thing);
        assert(dm_thing_manager->_service_identifier_requested);
        if (dm_thing_manager->_service_identifier_requested == NULL) {
            dm_log_err("method NOT match of service requested");
            return;
        }
    } else if (iotx_cmp_message_info->message_type == IOTX_CMP_MESSAGE_RAW) {
        dm_thing_manager->_request_id = iotx_cmp_message_info->id;
        dm_thing_manager->_raw_data = iotx_cmp_message_info->parameter;
        dm_thing_manager->_raw_data_length = iotx_cmp_message_info->parameter_length;

        /* invoke callback funtions. */
        invoke_callback_list(dm_thing_manager, dm_callback_type_raw_data_arrived);

        dm_thing_manager->_raw_data = NULL;
        dm_thing_manager->_raw_data_length = 0;

        return;
    }

    if (iotx_cmp_message_info->URI) {
        if (strstr(iotx_cmp_message_info->URI, string_method_name_thing_dsl_get_reply)) { /* thing/dsltemplate/get_reply match */
            thing = dm_thing_manager_generate_new_local_thing(dm_thing_manager, iotx_cmp_message_info->parameter,
                                                              iotx_cmp_message_info->parameter_length);
            if(NULL == thing) {
                dm_log_err("generate new thing failed");
                return;
            }
        } else if (strstr(iotx_cmp_message_info->URI, string_method_name_property_set)) { /* thing/service/property/set match */
            assert(thing && iotx_cmp_message_info->parameter);
            property_set_param_obj = cJSON_Parse(iotx_cmp_message_info->parameter);
            assert(property_set_param_obj && cJSON_IsObject(property_set_param_obj));

            property_iterator(thing, find_and_set_lite_property_for_service_property_set, dm_thing_manager, property_set_param_obj, thing, NULL);

            dm_log_info("%s triggerd", string_method_name_property_set);

#ifdef RRPC_ENABLED
            dm_thing_manager_answer_service(dm_thing_manager, thing, dm_thing_manager->_service_identifier_requested,
                                            dm_thing_manager->_request_id, dm_thing_manager->_ret == 0 ? 200 : 400, 0);
#else
            dm_thing_manager_answer_service(dm_thing_manager, thing, dm_thing_manager->_service_identifier_requested,
                                            dm_thing_manager->_request_id, dm_thing_manager->_ret == 0 ? 200 : 400);
#endif /* RRPC_ENABLED */

            cJSON_Delete(property_set_param_obj);

        } else if (strstr(iotx_cmp_message_info->URI, string_method_name_property_get)) { /* thing/service/property/get match */
            assert(iotx_cmp_message_info->parameter);
            list = dm_thing_manager->_service_property_get_identifier_list;
            property_get_param_obj = cJSON_Parse(iotx_cmp_message_info->parameter);

            assert(property_get_param_obj && cJSON_IsArray(property_get_param_obj));

            if (property_get_param_obj == NULL || cJSON_IsArray(property_get_param_obj) == 0) {
                dm_log_err("UNABLE to resolve %s params format", string_method_name_property_get);
                return;
            }

            array_size = cJSON_GetArraySize(property_get_param_obj);
            for (index = 0; index < array_size; ++index) {
                property_get_param_item_obj = cJSON_GetArrayItem(property_get_param_obj, index);
                assert(cJSON_IsString(property_get_param_item_obj));
#ifndef CJSON_STRING_ZEROCOPY
                property_get_param_identifier = dm_lite_calloc(1, strlen(property_get_param_item_obj->valuestring) + 1);
                strcpy(property_get_param_identifier, property_get_param_item_obj->valuestring);
#else
                property_get_param_identifier = dm_lite_calloc(1, property_get_param_item_obj->valuestring_length + 1);
                strncpy(property_get_param_identifier, property_get_param_item_obj->valuestring,
                        property_get_param_item_obj->valuestring_length);
#endif
                list_insert(list, property_get_param_identifier);
            }
            cJSON_Delete(property_get_param_obj);
#ifdef RRPC_ENABLED
            dm_thing_manager_answer_service(dm_thing_manager, thing, dm_thing_manager->_service_identifier_requested,
                                            dm_thing_manager->_request_id, 200, 0);
#else
            dm_thing_manager_answer_service(dm_thing_manager, thing, dm_thing_manager->_service_identifier_requested,
                                            dm_thing_manager->_request_id, 200);
#endif /* RRPC_ENABLED */

        } else if (strstr(iotx_cmp_message_info->URI, string_method_name_thing_enable)) { /* thing/enable match */
            /* invoke callback funtions. */
            invoke_callback_list(dm_thing_manager, dm_callback_type_thing_enabled);
        } else if (strstr(iotx_cmp_message_info->URI, string_method_name_thing_disable)) { /* thing/disable match */
            /* invoke callback funtions. */
            invoke_callback_list(dm_thing_manager, dm_callback_type_thing_disabled);
#ifdef RRPC_ENABLED
        } else if (strstr(iotx_cmp_message_info->URI, string_method_name_rrpc_request)) { /* rrpc/request match */
            char* p;
//            "/sys/productKey/{productKey}/productKey/{deviceName}/rrpc/request/${messageId}"
            p = strrchr(iotx_cmp_message_info->URI, '/');
            if (p) {
                p++;
                dm_thing_manager->_rrpc_message_id = atoi(p);
            }
            /* invoke callback funtions. */
            dm_thing_manager->_identifier = dm_thing_manager->_service_identifier_requested;
            invoke_callback_list(dm_thing_manager, dm_callback_type_rrpc_requested);
#endif /* RRPC_ENABLED */
#ifdef DEVICEINFO_ENABLED
        } else if (strstr(iotx_cmp_message_info->URI, string_method_name_deviceinfo_update_reply)) { /* deviceinfo/update_reply. */

        } else if (strstr(iotx_cmp_message_info->URI, string_method_name_deviceinfo_delete_reply)) { /* deviceinfo/delete_reply. */
#endif /* DEVICEINFO_ENABLED*/
        } else if (iotx_cmp_message_info->message_type == IOTX_CMP_MESSAGE_REQUEST) { /* normal service. */
            /* invoke callback funtions. */
            dm_thing_manager->_identifier = dm_thing_manager->_service_identifier_requested;
            invoke_callback_list(dm_thing_manager, dm_callback_type_service_requested);
        } else if (iotx_cmp_message_info->message_type == IOTX_CMP_MESSAGE_RESPONSE) { /* event resonse. */
            /* invoke callback funtions. */
            dm_thing_manager->_identifier = dm_thing_manager->_service_identifier_requested;
        }
    }

    if (iotx_cmp_message_info->URI) {
        dm_lite_free(iotx_cmp_message_info->URI);
        iotx_cmp_message_info->URI = NULL;
    }

    if (iotx_cmp_message_info->method) {
        dm_lite_free(iotx_cmp_message_info->method);
        iotx_cmp_message_info->method = NULL;
    }

    if (iotx_cmp_message_info->parameter) {
        dm_lite_free(iotx_cmp_message_info->parameter);
        iotx_cmp_message_info->parameter = NULL;
    }

}

static void send_request_to_uri(void* _dm_thing_manager, const char* _uri)
{
    dm_thing_manager_t* dm_thing_manager = _dm_thing_manager;
    const char* uri = _uri;
    message_info_t** message_info = dm_thing_manager->_message_info;
    cmp_abstract_t** cmp = dm_thing_manager->_cmp;
    thing_t** thing = dm_thing_manager->_thing_id;
    char uri_buff[URI_MAX_LENGH] = {0};
    char method_buff[METHOD_MAX_LENGH] = {0};
    char product_key[PRODUCT_KEY_MAXLEN] = {0};
    char device_name[DEVICE_NAME_MAXLEN] = {0};
    char* p;
    int ret;

    assert(dm_thing_manager && uri && message_info && cmp);

    get_product_key_device_name(product_key, device_name, thing, dm_thing_manager);

    strcpy(method_buff, uri);

    /* subtitute '/' by '.' */
    do {
        p = strchr(method_buff, '/');
        if (p) *p = '.';
    } while (p);

    dm_thing_manager->_method = method_buff;
    dm_snprintf(uri_buff, sizeof(uri_buff), "/sys/%s/%s/%s", product_key, device_name, uri);

    clear_and_set_message_info(message_info, dm_thing_manager);

    (*message_info)->set_uri(message_info, uri_buff);
    (*message_info)->set_message_type(message_info, CMP_MESSAGE_INFO_MESSAGE_TYPE_REQUEST);

    ret = (*message_info)->serialize_to_payload_request(message_info);

    if (ret == -1) {
        dm_log_err("serialize_to_payload_request FAIL");
        return;
    }
    (*cmp)->send(cmp, message_info, NULL);

}

static void* dm_thing_manager_ctor(void* _self, va_list* params)
{
    dm_thing_manager_t* self = _self;
    cmp_abstract_t** cmp;
    handle_dm_callback_fp_t callback_func;
    list_t** list;

    self->_name = va_arg(*params, char*);
    self->_get_tsl_from_cloud = va_arg(*params, int);
    callback_func = va_arg(*params, void*);
    self->_cloud_domain = va_arg(*params, int);

    assert(self->_name);

    self->_local_thing_list = new_object(SINGLE_LIST_CLASS, string_local_thing_list);
    self->_local_thing_name_list = new_object(SINGLE_LIST_CLASS, string_local_thing_name_list);
    self->_sub_thing_list = new_object(SINGLE_LIST_CLASS, string_sub_thing_list);
    self->_callback_list = new_object(SINGLE_LIST_CLASS, string_callback_list);
    self->_service_property_get_identifier_list = new_object(SINGLE_LIST_CLASS, string_service_property_get_identifier_list);
    self->_local_thing_id = 0;
    self->_thing_id = NULL;
    self->_message_info = new_object(CMP_MESSAGE_INFO_CLASS);
    self->_dm_version = DM_REQUEST_VERSION_STRING;
    self->_id = 0;
    self->_method = NULL;
    self->_cmp = new_object(CMP_ABSTRACT_IMPL_CLASS);
    self->_cmp_event_handle_func_fp = cmp_event_handler;
    self->_cmp_register_func_fp = cmp_register_handler;
    self->_cloud_connected = 0;
    self->_property_identifier_set = NULL;
    self->_destructing = 0;

    list = (list_t**)self->_callback_list;
    if (callback_func) list_insert(list, callback_func);

    /* get relative information. */
    HAL_GetProductKey(self->_product_key);
    HAL_GetDeviceName(self->_device_name);
    HAL_GetDeviceSecret(self->_device_secret);
    HAL_GetDeviceID(self->_device_id);

    assert(self->_product_key && self->_device_name && self->_device_secret && self->_device_id);

    cmp = self->_cmp;

    if(FAIL_RETURN == (*cmp)->init(cmp, self->_product_key, self->_device_name, self->_device_secret, self->_device_id, self->_cmp_event_handle_func_fp, self, self->_cloud_domain)) {
        dm_log_err("cmp init failed");
    }

    cJSON_Hooks hook = {
        dm_lite_malloc,
        dm_lite_free_func,
    };

    cJSON_InitHooks(&hook);

    return self;
}

static void* dm_thing_manager_dtor(void* _self)
{
    dm_thing_manager_t* self = _self;
    list_t** list;

    self->_destructing = 1;

    local_thing_list_iterator(self, free_list_thing);

    list = self->_local_thing_name_list;
    list_iterator(list, free_list_string, self);

    list = self->_service_property_get_identifier_list;
    list_iterator(list, free_list_string, self);

    assert(self->_local_thing_list && self->_local_thing_name_list && self->_sub_thing_list && self->_callback_list && self->_message_info);

    delete_object(self->_local_thing_list);
    delete_object(self->_local_thing_name_list);
    delete_object(self->_service_property_get_identifier_list);
    delete_object(self->_sub_thing_list);
    delete_object(self->_callback_list);
    delete_object(self->_message_info);
    delete_object(self->_cmp);

    self->_dm_version = NULL;
    self->_id = 0;
    self->_method = NULL;

    return self;
}

static void generate_thing_event_service_subscribe_uri(void* _item, int index, va_list* params)
{
    event_t* event = NULL;
    service_t* service = NULL;
    dm_thing_manager_t* dm_thing_manager;
    thing_t** thing;
    cmp_abstract_t** cmp;

    char method_buff[METHOD_MAX_LENGH] = {0};
    char product_key[PRODUCT_KEY_MAXLEN] = {0};
    char device_name[DEVICE_NAME_MAXLEN] = {0};
    char delimeter[] = {'.', 0};
    char* uri_buff;
    char* p;
    char* type;

    dm_thing_manager = va_arg(*params, void*);
    thing = va_arg(*params, void*);
    uri_buff = va_arg(*params, char*);
    type = va_arg(*params, char*);

    if (strcmp(type, string_event) == 0) {
        event = _item;
    } else if (strcmp(type, string_service) == 0) {
        service = _item;
    } else {
        assert(0);
    }
    assert(dm_thing_manager && uri_buff && _item);

    get_product_key_device_name(product_key, device_name, thing, dm_thing_manager);

    dm_sprintf(uri_buff, "/sys/%s/%s", product_key, device_name);

    sprintf(method_buff, event ? "%s_reply" : "%s", event ? event->method : service->method);

    p = strtok(method_buff, delimeter);
    while (p) {
        dm_sprintf(uri_buff + strlen(uri_buff), "/%s", p);
        p = strtok(NULL, delimeter);
    }

    cmp = dm_thing_manager->_cmp;
    (*cmp)->regist(cmp, uri_buff, dm_thing_manager->_cmp_register_func_fp, dm_thing_manager, NULL);
}

static void generate_subscribe_uri(void* _dm_thing_manager, void* _thing)
{
    dm_thing_manager_t* dm_thing_manager = _dm_thing_manager;
    thing_t** thing = _thing;
    cmp_abstract_t** cmp = dm_thing_manager->_cmp;

    int index;
    const char** uri;
    char uri_buff[URI_MAX_LENGH] = {0};
    const char* uri_array[] = {string_method_name_thing_enable, string_method_name_thing_delete, string_method_name_thing_disable,
                               string_method_name_thing_dsl_get_reply, string_method_name_down_raw, string_method_name_up_raw_reply,
#ifdef DEVICEINFO_ENABLED
                               string_method_name_deviceinfo_update_reply, string_method_name_deviceinfo_delete_reply,
#endif /* DEVICEINFO_ENABLED*/
#ifdef RRPC_ENABLED
                               string_method_name_rrpc_request_plus,
#endif /* RRPC_ENABLED */
                              };

    if (dm_thing_manager->_cloud_connected == 0) {
        dm_log_err("subscribe not allowed when cloud not connected");
        return;
    }

    if (thing) {
        event_iterator(thing, generate_thing_event_service_subscribe_uri, dm_thing_manager, thing, uri_buff, string_event);
        service_iterator(thing, generate_thing_event_service_subscribe_uri, dm_thing_manager, thing, uri_buff, string_service);
    } else {
        for (index = 0 ; index < sizeof(uri_array) / sizeof(char*); ++index) {
            uri = uri_array + index;
            dm_snprintf(uri_buff, sizeof(uri_buff), "/sys/%s/%s/", dm_thing_manager->_product_key, dm_thing_manager->_device_name);
            strcat(uri_buff, *uri);

            (*cmp)->regist(cmp, uri_buff, dm_thing_manager->_cmp_register_func_fp, dm_thing_manager, NULL);
        }
    }
}

static void* dm_thing_manager_generate_new_local_thing(void* _self, const char* tsl, int tsl_len)
{
    dm_thing_manager_t* self = _self;
    thing_t** thing;
    list_t** list;
    char* thing_name;
    size_t name_size;

    assert(tsl);

    name_size = sizeof(DM_LOCAL_THING_NAME_PATTERN) + 2;
    thing_name = (char*)dm_lite_calloc(1, name_size);

    memset(thing_name, 0, name_size);
    dm_sprintf(thing_name, DM_LOCAL_THING_NAME_PATTERN, self->_local_thing_id++);

    thing = (thing_t**)new_object(DM_THING_CLASS, thing_name);

    if(0 == (*thing)->set_dsl_string(thing, tsl, tsl_len)) {
        list = self->_local_thing_list;
        list_insert(list, thing);

        list = self->_local_thing_name_list;
        list_insert(list, thing_name);

        self->_thing_id = thing;

        dm_log_debug("new thing created@%p", self->_thing_id);

        /* subscribe subjects. */
        generate_subscribe_uri(self, thing);

        /* invoke callback funtions. */
        invoke_callback_list(self, dm_callback_type_new_thing_created);
    }else {
        delete_object(thing);
        thing = NULL;
    }

    return thing;
}

static int dm_thing_manager_add_callback_function(void* _self, handle_dm_callback_fp_t callback_func)
{
    dm_thing_manager_t* self = _self;
    list_t** list;

    list = (list_t**)self->_callback_list;

    assert(list);

    if (callback_func) list_insert(list, callback_func);

    return 0;
}

static void free_list_thing(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    (void)dm_thing_manager;

    assert(thing && *thing);

    delete_object(thing);
}

static void free_list_string(void* _string, va_list* params)
{
    void* string = _string;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    (void)dm_thing_manager;

    assert(string && dm_thing_manager);

    dm_lite_free(string);

    string = NULL;
}

static void get_service_input_output_value(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    assert(dm_thing_manager && thing && *thing && (*thing)->get_service_input_output_data_value_by_identifier);

    if (dm_thing_manager->_thing_id == thing) {
        dm_thing_manager->_ret = (*thing)->get_service_input_output_data_value_by_identifier(thing, dm_thing_manager->_identifier,
                                                                                             dm_thing_manager->_get_value, &dm_thing_manager->_get_value_str);
    }
}

static void set_service_input_output_value(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    assert(dm_thing_manager && thing && *thing && (*thing)->set_service_input_output_data_value_by_identifier);

    if (dm_thing_manager->_thing_id == thing) {
        dm_thing_manager->_ret = (*thing)->set_service_input_output_data_value_by_identifier(thing, dm_thing_manager->_identifier,
                                                                                             dm_thing_manager->_set_value, dm_thing_manager->_set_value_str);
    }
}

static void get_event_output_value(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    assert(dm_thing_manager && thing && *thing && (*thing)->get_event_value_by_identifier);

    if (dm_thing_manager->_thing_id == thing) {
        dm_thing_manager->_ret = (*thing)->get_event_value_by_identifier(thing, dm_thing_manager->_identifier, dm_thing_manager->_get_value, &dm_thing_manager->_get_value_str);
    }
}

static void set_event_output_value(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    assert(dm_thing_manager && thing && *thing && (*thing)->set_event_value_by_identifier);

    if (dm_thing_manager->_thing_id == thing) {
        dm_thing_manager->_ret = (*thing)->set_event_value_by_identifier(thing, dm_thing_manager->_identifier,
                                                                         dm_thing_manager->_set_value, dm_thing_manager->_set_value_str);
    }
}

static void invoke_callback_func(void* _callback_fp, va_list* params)
{
    handle_dm_callback_fp_t callback_fp = _callback_fp;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    assert(dm_thing_manager && callback_fp);

    if (callback_fp && dm_thing_manager) {
        callback_fp(dm_thing_manager->_callback_type, dm_thing_manager->_thing_id, dm_thing_manager->_identifier,
                    dm_thing_manager->_request_id, dm_thing_manager->_raw_data, dm_thing_manager->_raw_data_length);
    }
}

static void callback_list_iterator(const void* _self, handle_fp_t handle_fp)
{
    const dm_thing_manager_t* self = _self;
    list_t** list = (list_t**)self->_callback_list;

    assert((*list)->iterator);
    list_iterator(list, handle_fp, self);
}

static void set_property_value(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    assert(dm_thing_manager && thing && *thing && (*thing)->set_property_value_by_identifier);

    if (dm_thing_manager->_thing_id == thing) {
        dm_thing_manager->_ret = (*thing)->set_property_value_by_identifier(thing, dm_thing_manager->_identifier, dm_thing_manager->_set_value, dm_thing_manager->_set_value_str);

        /* invoke callback funtions. */

            if (strchr(dm_thing_manager->_identifier, '.') == NULL &&
                    strchr(dm_thing_manager->_identifier, '[') == NULL &&
                    strchr(dm_thing_manager->_identifier, ']') == NULL ) {
                invoke_callback_list(dm_thing_manager, dm_callback_type_property_value_set);

        }
    }
}

static void get_property_value(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    assert(dm_thing_manager && thing && *thing && (*thing)->get_property_value_by_identifier);

    if (dm_thing_manager->_thing_id == thing) {
        dm_thing_manager->_ret = (*thing)->get_property_value_by_identifier(thing, dm_thing_manager->_identifier, dm_thing_manager->_get_value, &dm_thing_manager->_get_value_str);
    }
}

static void local_thing_list_iterator(const void* _self, handle_fp_t handle_fp)
{
    const dm_thing_manager_t* self = _self;
    list_t** list = (list_t**)self->_local_thing_list;

    assert((*list)->iterator);
    list_iterator(list, handle_fp, self);
}

static int dm_thing_manager_set_thing_property_value(void* _self, const void* thing_id, const void* identifier,
                                                     const void* value, const char* value_str)
{
    dm_thing_manager_t* self = _self;

    assert(thing_id && identifier && (value || value_str));

    self->_thing_id = (void*)thing_id;
    self->_identifier = (void*)identifier;
    self->_set_value = (void*)value;
    self->_set_value_str = (char*)value_str;
    self->_ret = -1;

    local_thing_list_iterator(self, set_property_value);

    return self->_ret;
}

static int dm_thing_manager_get_thing_property_value(void* _self, const void* thing_id, const void* identifier,
                                                     void* value, char** value_str)
{
    dm_thing_manager_t* self = _self;

    assert(thing_id && identifier && (value || value_str));

    self->_thing_id = (void*)thing_id;
    self->_identifier = (void*)identifier;
    self->_get_value = value;
    self->_ret = -1;
    self->_get_value_str = NULL;

    local_thing_list_iterator(self, get_property_value);

    if (value_str) *value_str = self->_get_value_str;

    return self->_ret;
}

static int dm_thing_manager_set_thing_event_output_value(void* _self, const void* thing_id, const void* identifier,
                                                         const void* value, const char* value_str)
{
    dm_thing_manager_t* self = _self;

    assert(thing_id && identifier && (value || value_str));

    self->_thing_id = (void*)thing_id;
    self->_identifier = (void*)identifier;
    self->_set_value = (void*)value;
    self->_set_value_str = (char*)value_str;
    self->_ret = -1;

    local_thing_list_iterator(self, set_event_output_value);

    return self->_ret;
}

static int dm_thing_manager_get_thing_event_output_value(void* _self, const void* thing_id, const void* identifier,
                                                         void* value, char** value_str)
{
    dm_thing_manager_t* self = _self;

    assert(thing_id && identifier && (value || value_str));

    self->_thing_id = (void*)thing_id;
    self->_identifier = (void*)identifier;
    self->_get_value = value;
    self->_ret = -1;

    local_thing_list_iterator(self, get_event_output_value);

    if (value_str) *value_str = self->_get_value_str;

    return self->_ret;
}

static int dm_thing_manager_get_thing_service_input_value(void* _self, const void* thing_id, const void* identifier,
                                                          void* value, char** value_str)
{
    dm_thing_manager_t* self = _self;

    assert(thing_id && identifier && (value || value_str));

    self->_thing_id = (void*)thing_id;
    self->_identifier = (void*)identifier;
    self->_get_value = value;
    self->_ret = -1;

    local_thing_list_iterator(self, get_service_input_output_value);

    if (value_str) *value_str = self->_get_value_str;

    return self->_ret;
}

static int dm_thing_manager_get_thing_service_output_value(void* _self, const void* thing_id, const void* identifier,
                                                           void* value, char** value_str)
{
    dm_thing_manager_t* self = _self;

    return dm_thing_manager_get_thing_service_input_value(self, thing_id, identifier, value, value_str);
}

static int dm_thing_manager_set_thing_service_output_value(void* _self, const void* thing_id, const void* identifier,
                                                           const void* value, const char* value_str)
{
    dm_thing_manager_t* self = _self;

    assert(thing_id && identifier && (value || value_str));

    self->_thing_id = (void*)thing_id;
    self->_identifier = (void*)identifier;
    self->_set_value = (void*)value;
    self->_set_value_str = (char*)value_str;
    self->_ret = -1;

    local_thing_list_iterator(self, set_service_input_output_value);

    return self->_ret;
}

static int install_lite_property_to_message_info(dm_thing_manager_t* _thing_manager, message_info_t** _message_info, lite_property_t* _lite_property)
{
    lite_property_t* lite_property = _lite_property;
    message_info_t** message_info = _message_info;
    dm_thing_manager_t* dm_thing_manager = _thing_manager;
    thing_t** thing = dm_thing_manager->_thing_id;
    char* p = NULL;

    char* q = NULL;
    dm_thing_t* dm_thing;
    size_t params_buffer_len = 0;
    size_t params_val_len = 0;
    int ret, i;
    char property_key_value_buff[PROPERTY_KEY_VALUE_BUFF_MAX_LENGTH] = {0};

    ret = (*thing)->get_lite_property_value(thing, lite_property, NULL, &dm_thing_manager->_get_value_str);

    if ((lite_property->data_type.type == data_type_type_text || lite_property->data_type.type == data_type_type_date) && dm_thing_manager->_get_value_str) {
        p = dm_lite_calloc(1, strlen(dm_thing_manager->_get_value_str) + 3);
        assert(p);

        dm_sprintf(p, "\"%s\"", dm_thing_manager->_get_value_str);
        dm_thing_manager->_get_value_str = p;
    }

    if (ret != 0 && lite_property->data_type.type == data_type_type_array) {
        dm_thing = (dm_thing_t*)thing;
        property_key_value_buff[0] = '[';
        property_key_value_buff[1] = '\0';

        params_buffer_len = sizeof(property_key_value_buff);
        for (i = 0; i < lite_property->data_type.value.data_type_array_t.size; i++) {
            dm_thing->_arr_index = i;
            dm_thing_manager->_get_value_str = NULL;
            ret = (*thing)->get_lite_property_value(thing, lite_property, NULL, &dm_thing_manager->_get_value_str);
            params_val_len = strlen(property_key_value_buff);

            if (ret != 0 || !dm_thing_manager->_get_value_str) {
                dm_thing_manager->_get_value_str = NULL;
            }
            if (lite_property->data_type.value.data_type_array_t.item_type == data_type_type_text) {
                dm_snprintf(property_key_value_buff + params_val_len, params_buffer_len - params_val_len, "\"%s\",", dm_thing_manager->_get_value_str ? dm_thing_manager->_get_value_str : "");
            } else {
                dm_snprintf(property_key_value_buff + params_val_len, params_buffer_len - params_val_len, "%s,", dm_thing_manager->_get_value_str ? dm_thing_manager->_get_value_str : "0");
            }
        }
        q = property_key_value_buff + strlen(property_key_value_buff) - 1;
        if (*q == '[') *(q+1) = ']';
        if (*q == ',') *(q) = ']';

        dm_thing->_arr_index = -1;

        (*message_info)->add_params_data_item(message_info, lite_property->identifier, property_key_value_buff);

        return 0;
    }

    if (ret == 0 && lite_property->identifier && dm_thing_manager->_get_value_str) {
        (*message_info)->add_params_data_item(message_info, lite_property->identifier, dm_thing_manager->_get_value_str);
    }

    if (p) dm_lite_free(p);

    return ret;
}

static void get_product_key_device_name(char* _product_key, char* _device_name, void* _thing, void* _dm_thing_manager)
{
    char* product_key = _product_key;
    char* device_name = _device_name;

    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager = _dm_thing_manager;
    if (thing && *thing) {
        dm_thing_manager_install_product_key_device_name(dm_thing_manager, thing, product_key, device_name);
    } else {
        strcpy(product_key, dm_thing_manager->_product_key);
        strcpy(device_name, dm_thing_manager->_device_name);
    }
}

static void clear_and_set_message_info(message_info_t** _message_info, dm_thing_manager_t* _dm_thing_manager)
{
    dm_thing_manager_t* dm_thing_manager = _dm_thing_manager;
    message_info_t** message_info = _message_info;
    thing_t** thing;

    char product_key[PRODUCT_KEY_MAXLEN] = {0};
    char device_name[DEVICE_NAME_MAXLEN] = {0};

    assert(message_info && *message_info && dm_thing_manager);

    if (message_info == NULL || *message_info == NULL || dm_thing_manager == NULL) return;

    thing = dm_thing_manager->_thing_id;

    get_product_key_device_name(product_key, device_name, thing, dm_thing_manager);
    if (0 == dm_thing_manager->_id) dm_thing_manager->_id++;

    (*message_info)->clear(message_info);

    (*message_info)->set_product_key(message_info, product_key);
    (*message_info)->set_device_name(message_info, device_name);
    (*message_info)->set_version(message_info, dm_thing_manager->_dm_version);
    (*message_info)->set_id(message_info, dm_thing_manager->_id);
    dm_thing_manager->_id = (dm_thing_manager->_id + 1) % INT32_MAX;
    (*message_info)->set_method(message_info, dm_thing_manager->_method);
}

static void install_property_to_message_info(void* _item, int index, va_list* params)
{
    property_t* property = _item;
    lite_property_t* lite_property;
    lite_property_t* struct_lite_property;
    dm_thing_manager_t* dm_thing_manager;
    thing_t** thing;
    message_info_t** message_info;
    size_t params_buffer_len = 0;
    size_t params_val_len = 0;
    char* target_property_identifier;
    int ret, i;
    char property_key_value_buff[PROPERTY_KEY_VALUE_BUFF_MAX_LENGTH] = {0};
    char* p;
    char* q = NULL;

    dm_thing_manager = va_arg(*params, dm_thing_manager_t*);
    thing = va_arg(*params, thing_t**);
    message_info = va_arg(*params, message_info_t**);
    target_property_identifier = va_arg(*params, char*);

    index = index;

    assert(property && dm_thing_manager && thing && *thing && message_info && *message_info);

    lite_property = (lite_property_t*)property;

    /* post all value, or specify identifier. */
    if (property && (target_property_identifier == NULL || (property->identifier && strcmp(property->identifier, target_property_identifier) == 0))) {
        ret = install_lite_property_to_message_info(dm_thing_manager, message_info, lite_property);

        params_buffer_len = sizeof(property_key_value_buff);
        if (ret == -1) {
            if(property->data_type.type == data_type_type_struct) {
                property_key_value_buff[0] = '{';
                property_key_value_buff[1] = '\0';
                for (i = 0; i < property->data_type.data_type_specs_number; ++i) {
                    struct_lite_property = (lite_property_t*)property->data_type.specs + i;
                    ret = (*thing)->get_lite_property_value(thing, struct_lite_property, NULL, &dm_thing_manager->_get_value_str);
                    if (ret == 0 && dm_thing_manager->_get_value_str) {
                        params_val_len = strlen(property_key_value_buff);
                        p = property_key_value_buff + params_val_len - 1;

                        /* not the last item, chang from '}' to ','. */
                        if (p && *p == '}') *p = ',';

                        if (struct_lite_property->data_type.type == data_type_type_text && dm_thing_manager->_get_value_str) {
                            q = dm_lite_calloc(1, strlen(dm_thing_manager->_get_value_str) + 3);
                            assert(q);

                            dm_snprintf(q, strlen(dm_thing_manager->_get_value_str) + 3, "\"%s\"", dm_thing_manager->_get_value_str);
                            dm_thing_manager->_get_value_str = q;
                        }

                        dm_snprintf(property_key_value_buff + params_val_len, params_buffer_len - params_val_len, "\"%s\":%s}",
                                   struct_lite_property->identifier, dm_thing_manager->_get_value_str);

                        if (q) dm_lite_free(q);
                    }
                }

            }
            (*message_info)->add_params_data_item(message_info, property->identifier, property_key_value_buff);
        }
        dm_thing_manager->_ret = 0;
    }
}

static void handle_event_key_value(void* _item, int index, va_list* params)
{
    event_t* event;
    lite_property_t* lite_property;
    dm_thing_manager_t* dm_thing_manager;
    message_info_t** message_info;
    thing_t** thing;
    char method_buff[METHOD_MAX_LENGH] = {0};
    char uri_buff[URI_MAX_LENGH] = {0};
    char product_key[PRODUCT_KEY_MAXLEN] = {0};
    char device_name[DEVICE_NAME_MAXLEN] = {0};

    char* p;
    int output_data_numb;
    int i;

    (void)index;
    dm_thing_manager = va_arg(*params, void*);
    message_info = va_arg(*params, message_info_t**);
    thing = va_arg(*params, thing_t**);

    assert(dm_thing_manager && message_info && *message_info && (*message_info)->set_message_type && thing && *thing);

    event = _item;
    if (strcmp(event->identifier, dm_thing_manager->_identifier) != 0) return;

    dm_thing_manager->_method = event->method;

    clear_and_set_message_info(message_info, dm_thing_manager);

    (*message_info)->set_message_type(message_info, CMP_MESSAGE_INFO_MESSAGE_TYPE_REQUEST);

    strcpy(method_buff, event->method);
    /* subtitute '.' by '/' */
    do {
        p = strchr(method_buff, '.');
        if (p) *p = '/';
    } while (p);

    dm_thing_manager_install_product_key_device_name(dm_thing_manager,thing,product_key,device_name);

    dm_snprintf(uri_buff, sizeof(uri_buff), "/sys/%s/%s/%s", product_key, device_name, method_buff);
    (*message_info)->set_uri(message_info, uri_buff);

    if (strcmp(dm_thing_manager->_method, string_thing_event_property_post) != 0) {
        output_data_numb = event->event_output_data_num;
        for (i = 0; i < output_data_numb; ++i) {
            lite_property = event->event_output_data + i;
            assert(lite_property);
            install_lite_property_to_message_info(dm_thing_manager, message_info, lite_property);
        }
        dm_thing_manager->_ret = 0;
    } else {
        property_iterator(thing, install_property_to_message_info, dm_thing_manager, thing,
                          dm_thing_manager->_message_info, dm_thing_manager->_property_identifier_post);
    }

}

static void install_service_property_get_to_message_info(void* _string, va_list* params)
{
    char* identifier_to_get = _string;
    dm_thing_manager_t* dm_thing_manager;
    thing_t** thing;
    message_info_t** message_info;

    dm_thing_manager = va_arg(*params, dm_thing_manager_t*);
    thing = va_arg(*params, thing_t**);
    message_info = va_arg(*params, message_info_t**);

    assert(identifier_to_get);

    property_iterator(thing, install_property_to_message_info, dm_thing_manager, thing, message_info, identifier_to_get);
}

static void handle_service_key_value(void* _item, int index, va_list* params)
{
    service_t* service;
    lite_property_t* lite_property;
    dm_thing_manager_t* dm_thing_manager;
    message_info_t** message_info;
    thing_t** thing;
    list_t** list;
    int output_data_numb;
    char method_buff[METHOD_MAX_LENGH] = {0};
    char uri_buff[URI_MAX_LENGH] = {0};
    char product_key[PRODUCT_KEY_MAXLEN] = {0};
    char device_name[DEVICE_NAME_MAXLEN] = {0};

    char* p;
    int i;

    (void)index;
    dm_thing_manager = va_arg(*params, void*);
    message_info = va_arg(*params, message_info_t**);
    thing = va_arg(*params, thing_t**);

    assert(dm_thing_manager && message_info && *message_info && thing && *thing);

    service = _item;
    list = dm_thing_manager->_service_property_get_identifier_list;

    if (service && service->identifier && strcmp(service->identifier, dm_thing_manager->_identifier) != 0) return;

    dm_thing_manager->_method = service->method;

    clear_and_set_message_info(message_info, dm_thing_manager);

    strcpy(method_buff, service->method);
    /* subtitute '.' by '/' */
    do {
        p = strchr(method_buff, '.');
        if (p) *p = '/';
    } while (p);

    dm_thing_manager_install_product_key_device_name(dm_thing_manager,thing,product_key,device_name);
#ifdef RRPC_ENABLED
    if (dm_thing_manager->_rrpc) {
        dm_snprintf(uri_buff, sizeof(uri_buff), "/sys/%s/%s/rrpc/response/%d", product_key, device_name, dm_thing_manager->_rrpc_message_id);
    } else {
        dm_snprintf(uri_buff, sizeof(uri_buff), "/sys/%s/%s/%s_reply", product_key, device_name, method_buff);
    }
#else
    dm_snprintf(uri_buff, sizeof(uri_buff), "/sys/%s/%s/%s_reply", product_key, device_name, method_buff);
#endif /* RRPC_ENABLED */

    (*message_info)->set_uri(message_info, uri_buff);

    (*message_info)->set_message_type(message_info, CMP_MESSAGE_INFO_MESSAGE_TYPE_RESPONSE);
    (*message_info)->set_id(message_info, dm_thing_manager->_response_id);
    (*message_info)->set_code(message_info, dm_thing_manager->_code);

    if (strcmp(dm_thing_manager->_method, string_thing_service_property_set) == 0) {
        /* set property */
    } else if (strcmp(dm_thing_manager->_method, string_thing_service_property_get) == 0) {
        assert((*list)->get_size(list));
        /* get property */
        list_iterator(list, install_service_property_get_to_message_info, dm_thing_manager, thing, dm_thing_manager->_message_info);
        /* clear after use. */
        list_iterator(list, free_list_string, dm_thing_manager);
        (*list)->clear(list);
    } else {
        /* normal service */
        output_data_numb = service->service_output_data_num;
        for (i = 0; i < output_data_numb; ++i) {
            lite_property = service->service_output_data + i;
            assert(lite_property);
            install_lite_property_to_message_info(dm_thing_manager, message_info, lite_property);
        }
    }
}

static void get_event_key_value(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    assert(dm_thing_manager && thing && *thing);

    if (dm_thing_manager->_thing_id == thing) {
        event_iterator(thing, handle_event_key_value, dm_thing_manager, dm_thing_manager->_message_info, thing);
    }
}

static void get_service_key_value(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    assert(dm_thing_manager && thing && *thing);

    if (dm_thing_manager->_thing_id == thing) {
        service_iterator(thing, handle_service_key_value, dm_thing_manager, dm_thing_manager->_message_info, thing);
    }
}

static int dm_thing_manager_trigger_event(void* _self, const void* thing_id, const void* event_identifier, const char* property_identifier)
{
    dm_thing_manager_t* self = _self;
    message_info_t** message_info = self->_message_info;
    cmp_abstract_t** cmp = self->_cmp;
    int ret;

    assert(thing_id && event_identifier && cmp && *cmp);

    self->_thing_id = (void*)thing_id;
    self->_identifier = (void*)event_identifier;
    self->_property_identifier_post = (void*)property_identifier;
    self->_ret = -1;
    self->_get_value_str = NULL;

    local_thing_list_iterator(self, get_event_key_value);

    if(-1 != self->_ret) {
        dm_log_debug("event(%s) triggered, method(%s)", self->_identifier, self->_method);
        ret = (*message_info)->serialize_to_payload_request(message_info);
        if (ret == -1) {
            dm_log_err("serialize_to_payload_request FAIL");
            return ret;
        }

        self->_ret = (*cmp)->send(cmp, message_info, NULL);
    }

    return self->_ret;
}

#ifdef DEVICEINFO_ENABLED
static void check_thing_id(void* _thing, va_list* params)
{
    thing_t** thing = _thing;
    dm_thing_manager_t* dm_thing_manager;

    dm_thing_manager = va_arg(*params, void*);

    assert(dm_thing_manager && thing && *thing);

    if (dm_thing_manager->_thing_id == thing) {
        dm_thing_manager->_ret = 0;
    }
}

static int check_deviceinfo_params(const char* params)
{
    cJSON* params_obj;
    int array_size;

    params_obj = cJSON_Parse(params);

    if (!cJSON_IsArray(params_obj)) {
        dm_log_err("input params format errorm, MUST be json array type");
        return -1;
    }

    array_size = cJSON_GetArraySize(params_obj);

    if (array_size <= 0) return -1;

    cJSON_Delete(params_obj);

    return 0;
}

static int dm_thing_manager_trigger_deviceinfo_update(void* _self, const void* thing_id, const char* params)
{
    dm_thing_manager_t* self = _self;
    message_info_t** message_info = self->_message_info;
    cmp_abstract_t** cmp = self->_cmp;
    thing_t** thing;

    char method_buff[METHOD_MAX_LENGH] = {0};
    char uri_buff[URI_MAX_LENGH] = {0};

    char product_key[PRODUCT_KEY_MAXLEN] = {0};
    char device_name[DEVICE_NAME_MAXLEN] = {0};

    char* p;

    assert(thing_id && cmp && *cmp && message_info && *message_info);

    self->_thing_id = (void*)thing_id;
    self->_ret = -1;
    self->_get_value_str = NULL;

    check_deviceinfo_params(params);

    local_thing_list_iterator(self, check_thing_id);

    if (self->_ret != 0) return -1;

    thing = (thing_t**)thing_id;

    strcpy(method_buff, string_method_name_deviceinfo_update);
    /* subtitute '/' by '.' */
    do {
        p = strchr(method_buff, '/');
        if (p) *p = '.';
    } while (p);

    self->_method = method_buff;

    clear_and_set_message_info(message_info, self);

    (*message_info)->set_message_type(message_info, CMP_MESSAGE_INFO_MESSAGE_TYPE_REQUEST);

    dm_thing_manager_install_product_key_device_name(self,thing,product_key,device_name);

    dm_snprintf(uri_buff, sizeof(uri_buff), "/sys/%s/%s/%s", product_key, device_name, string_method_name_deviceinfo_update);
    (*message_info)->set_uri(message_info, uri_buff);

    (*message_info)->set_params_data(message_info, (char*)params);

    return (*cmp)->send(cmp, message_info, NULL);
}

static int dm_thing_manager_trigger_deviceinfo_delete(void* _self, const void* thing_id, const char* params)
{
    dm_thing_manager_t* self = _self;
    message_info_t** message_info = self->_message_info;
    cmp_abstract_t** cmp = self->_cmp;
    thing_t** thing;

    char method_buff[METHOD_MAX_LENGH] = {0};
    char uri_buff[URI_MAX_LENGH] = {0};
    char product_key[PRODUCT_KEY_MAXLEN] = {0};
    char device_name[DEVICE_NAME_MAXLEN] = {0};
    char* p;

    assert(thing_id && cmp && *cmp && message_info && *message_info);

    self->_thing_id = (void*)thing_id;
    self->_ret = -1;
    self->_get_value_str = NULL;

    check_deviceinfo_params(params);

    local_thing_list_iterator(self, check_thing_id);

    if (self->_ret != 0) return -1;

    thing = (thing_t**)thing_id;

    strcpy(method_buff, string_method_name_deviceinfo_delete);
    /* subtitute '/' by '.' */
    do {
        p = strchr(method_buff, '/');
        if (p) *p = '.';
    } while (p);

    self->_method = method_buff;

    clear_and_set_message_info(message_info, self);

    (*message_info)->set_message_type(message_info, CMP_MESSAGE_INFO_MESSAGE_TYPE_REQUEST);

    dm_thing_manager_install_product_key_device_name(self,thing,product_key,device_name);

    dm_snprintf(uri_buff, sizeof(uri_buff), "/sys/%s/%s/%s", product_key, device_name, string_method_name_deviceinfo_delete);
    (*message_info)->set_uri(message_info, uri_buff);

    (*message_info)->set_params_data(message_info, (char*)params);

    return (*cmp)->send(cmp, message_info, NULL);
}
#endif /* DEVICEINFO_ENABLED*/
static int generate_raw_message_info(void* _dm_thing_manager, void* _thing, void* _message_info, const char* raw_topic)
{
    dm_thing_manager_t* dm_thing_manager = _dm_thing_manager;
    thing_t** thing = _thing;
    message_info_t** message_info = _message_info;

    char product_key[PRODUCT_KEY_MAXLEN] = {0};
    char device_name[DEVICE_NAME_MAXLEN] = {0};

    char uri_buff[URI_MAX_LENGH] = {0};
    int ret;

    dm_thing_manager_install_product_key_device_name(dm_thing_manager,thing,product_key,device_name);

    dm_snprintf(uri_buff, sizeof(uri_buff), "/sys/%s/%s/%s", product_key, device_name, raw_topic);

    clear_and_set_message_info(message_info, dm_thing_manager);

    ret = (*message_info)->set_uri(message_info, uri_buff);
    if (ret == -1) return ret;

    (*message_info)->set_message_type(message_info, CMP_MESSAGE_INFO_MESSAGE_TYPE_RAW);

    ret = (*message_info)->set_raw_data_and_length(message_info, dm_thing_manager->_raw_data, dm_thing_manager->_raw_data_length);
    if (ret == -1) return ret;

    return 0;
}

static int generate_raw_up_message_info(void* _dm_thing_manager, void* _thing, void* _message_info)
{
    return generate_raw_message_info(_dm_thing_manager, _thing, _message_info, string_method_name_up_raw);
}

static int generate_raw_down_reply_message_info(void* _dm_thing_manager, void* _thing, void* _message_info)
{
    return generate_raw_message_info(_dm_thing_manager, _thing, _message_info, string_method_name_down_raw_reply);
}

#ifdef RRPC_ENABLED
static int dm_thing_manager_answer_service(void* _self, const void* thing_id, const void* identifier, int response_id, int code, int rrpc)
#else
static int dm_thing_manager_answer_service(void* _self, const void* thing_id, const void* identifier, int response_id, int code)
#endif /* RRPC_ENABLED */
{
    dm_thing_manager_t* self = _self;
    message_info_t** message_info = self->_message_info;
    cmp_abstract_t** cmp = self->_cmp;
    int ret;

    assert(thing_id && identifier && cmp && *cmp);

    self->_thing_id = (void*)thing_id;
    self->_identifier = (void*)identifier;
    self->_ret = 0;
    self->_get_value_str = NULL;
    self->_code = code;
    self->_response_id = response_id;
    self->_method = NULL;

    /* response id matches request id. */
    if (self->_response_id == self->_request_id) {
        dm_log_debug("response id(%d) matches request id(%d).", self->_response_id, self->_request_id);
        self->_request_id = 0;
    } else {
        dm_log_warning("response id(%d) not matche request id(%d).", self->_response_id, self->_request_id);
    }
#ifdef RRPC_ENABLED
    self->_rrpc = rrpc;
#endif /* RRPC_ENABLED */
    local_thing_list_iterator(self, get_service_key_value);

    dm_log_debug("answer normal service(%s), method(%s)", self->_identifier, self->_method ? self->_method : "NULL");

    ret = (*message_info)->serialize_to_payload_response(message_info);

    if (ret == -1) {
        dm_log_err("serialize_to_payload_response FAIL");
        return ret;
    }

    self->_ret = (*cmp)->send(cmp, message_info, NULL);

    return self->_ret;
}

static int dm_thing_manager_invoke_raw_service(void* _self, const void* thing_id, void* raw_data, int raw_data_length)
{
    dm_thing_manager_t* self = _self;
    message_info_t** message_info = self->_message_info;
    cmp_abstract_t** cmp = self->_cmp;

    assert(thing_id);

    self->_thing_id = (void*)thing_id;
    self->_ret = 0;
    self->_raw_data = raw_data;
    self->_raw_data_length = raw_data_length;

    dm_log_debug("invoke up raw service");

    if (generate_raw_up_message_info(self, self->_thing_id, message_info) == -1)
    {
        dm_log_err("invoke up raw service FAIL!");
        return -1;
    }

    self->_ret = (*cmp)->send(cmp, message_info, NULL);

    self->_raw_data = NULL;
    self->_raw_data_length = 0;

    return self->_ret;
}

static int dm_thing_manager_answer_raw_service(void* _self, const void* thing_id, void* raw_data, int raw_data_length)
{
    dm_thing_manager_t* self = _self;
    message_info_t** message_info = self->_message_info;
    cmp_abstract_t** cmp = self->_cmp;

    assert(thing_id);

    self->_thing_id = (void*)thing_id;
    self->_ret = 0;
    self->_raw_data = raw_data;
    self->_raw_data_length = raw_data_length;

    dm_log_debug("invoke down raw reply service");

    if (generate_raw_down_reply_message_info(self, self->_thing_id, message_info) == -1)
    {
        dm_log_err("invoke down raw reply service FAIL!");
        return -1;
    }

    self->_ret = (*cmp)->send(cmp, message_info, NULL);

    self->_raw_data = NULL;
    self->_raw_data_length = 0;

    return self->_ret;
}
#ifndef CMP_SUPPORT_MULTI_THREAD
static int dm_thing_manager_yield(void* _self, int timeout_ms)
{
    dm_thing_manager_t* self = _self;
    cmp_abstract_t** cmp = self->_cmp;

    return (*cmp)->yield(cmp, timeout_ms);
}
#endif


static int dm_thing_manager_install_product_key_device_name(void *_self, const void* thing_id, char *product_key, char *device_name)
{
    char *dsl_product_key;
    char *dsl_device_name;
    char hal_product_key[PRODUCT_KEY_MAXLEN] = {0};
    char hal_device_name[DEVICE_NAME_MAXLEN] = {0};

    dm_thing_manager_t* self = _self;
    const thing_t** thing = (const thing_t**)thing_id;

    self->_ret = -1;

    if(HAL_GetProductKey(hal_product_key) <= 0 ||
        (HAL_GetDeviceName(hal_device_name) <= 0)) {
        dm_log_err("get HAL DeviceInfo failed!");
    }

    dsl_product_key = (*thing)->return_product_key(thing);
    dsl_device_name = (*thing)->return_device_name(thing);

    if(dsl_product_key && (0 == strcmp(dsl_product_key, hal_product_key))) {
        strcpy(product_key, dsl_product_key);
    }else {
        dm_log_err("hal pk(%s) != dsl pk(%s)", hal_product_key, dsl_product_key ? dsl_product_key : "NULL");
        strcpy(product_key, hal_product_key);
    }
    if(dsl_device_name && (0 == strcmp(dsl_device_name, hal_device_name))) {
        strcpy(device_name, dsl_device_name);
    }else {
        dm_log_err("hal dn(%s) != dsl dn(%s)", hal_device_name, dsl_device_name ? dsl_device_name : "NULL");
        strcpy(device_name, hal_device_name);
    }

    self->_ret = 0;

    return self->_ret;
}


static thing_manager_t _dm_thing_manager_class = {
    sizeof(dm_thing_manager_t),
    string_dm_thing_manager_class_name,
    dm_thing_manager_ctor,
    dm_thing_manager_dtor,
    dm_thing_manager_generate_new_local_thing,
    dm_thing_manager_add_callback_function,
    dm_thing_manager_set_thing_property_value,
    dm_thing_manager_get_thing_property_value,
    dm_thing_manager_set_thing_event_output_value,
    dm_thing_manager_get_thing_event_output_value,
    dm_thing_manager_get_thing_service_input_value,
    dm_thing_manager_get_thing_service_output_value,
    dm_thing_manager_set_thing_service_output_value,
    dm_thing_manager_trigger_event,
#ifdef DEVICEINFO_ENABLED
    dm_thing_manager_trigger_deviceinfo_update,
    dm_thing_manager_trigger_deviceinfo_delete,
#endif /* DEVICEINFO_ENABLED*/
    dm_thing_manager_answer_service,
    dm_thing_manager_invoke_raw_service,
    dm_thing_manager_answer_raw_service,
#ifndef CMP_SUPPORT_MULTI_THREAD
    dm_thing_manager_yield,
#endif
    dm_thing_manager_install_product_key_device_name,

};

const void* get_dm_thing_manager_class()
{
    return &_dm_thing_manager_class;
}

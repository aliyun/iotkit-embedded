#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmp_message_info.h"
#include "logger.h"
#include "dm_import.h"
#include "iot_export_dm.h"
#include "interface/list_abstract.h"
#include "single_list.h"
#include "class_interface.h"

#define CMP_MESSAGE_INFO_EXTENTED_ROOM_FOR_STRING_MALLOC 1

static const char string_cmp_message_info_class_name[] __DM_READ_ONLY__ = "cmp_msg_info_cls";
static const char string_cmp_message_info_param_list_object_name[] __DM_READ_ONLY__ = "cmp message info param list";

static void cmp_message_info_clear(void* _self);
static void cmp_message_info_set_params_data(void* _self, char* params_buf);

static void list_insert(void* _list, void* _data)
{
    list_t** list = (list_t**)_list;

    (*list)->insert(list, _data);
}

static void* cmp_message_info_ctor(void* _self, va_list* params)
{
    cmp_message_info_t* self = _self;

    self->uri = NULL;
    self->payload_buf = NULL;
    self->params_data_buf = NULL;
    self->params_data_buf = NULL;
    self->raw_data_length = 0;
    self->product_key = NULL;
    self->device_name = NULL;
    self->id = 0;
    self->code = 0;
    self->version = NULL;
    self->param_list = new_object(SINGLE_LIST_CLASS, string_cmp_message_info_param_list_object_name);
    self->method = NULL;
    self->message_type = CMP_MESSAGE_INFO_MESSAGE_TYPE_REQUEST;
    self->ret = -1;
    (void)params;

    return self;
}

static int deep_strcpy(char** _dst, const void* _src)
{
    char** dst = _dst;
    const char* src = _src;
    int size = strlen(src) + CMP_MESSAGE_INFO_EXTENTED_ROOM_FOR_STRING_MALLOC;

    *dst = dm_lite_calloc(1, size);

    assert(*dst);
    if (*dst == NULL) return -1;

    strcpy(*dst, src);

    return 0;
}

static void* cmp_message_info_dtor(void* _self)
{
    cmp_message_info_t* self = _self;

    cmp_message_info_clear(self);
    delete_object(self->param_list);

    return self;
}

static int cmp_message_info_set_uri(void* _self, char* uri)
{
    cmp_message_info_t* self = _self;

    if (self->uri) {
        dm_lite_free(self->uri);
        self->uri = NULL;
    }
    assert(uri);
    if (uri) {
        return deep_strcpy(&self->uri, uri);
    }

    return -1;
}

static void cmp_message_info_set_payload(void* _self, char* payload_buf, unsigned int payload_len)
{
    cmp_message_info_t* self = _self;

    if (self->payload_buf) {
        dm_lite_free(self->payload_buf);
        self->payload_buf = NULL;
    }
    assert(payload_buf);
    if (payload_buf) deep_strcpy(&self->payload_buf, payload_buf);
}

static void clear_req_rsp_params(void* _req_rsp_param, va_list* params)
{
    req_rsp_param_t* req_rsp_param = _req_rsp_param;
    cmp_message_info_t* cmp_message_info;

    cmp_message_info = va_arg(*params, void*);

    assert(cmp_message_info);

    if (req_rsp_param) {
        if (req_rsp_param->key) dm_lite_free(req_rsp_param->key);
        if (req_rsp_param->value) dm_lite_free(req_rsp_param->value);

        dm_lite_free(req_rsp_param);

        req_rsp_param = NULL;
    }
}

static void cmp_message_info_clear(void* _self)
{
    cmp_message_info_t* self = _self;

    if (self->uri) {
        dm_lite_free(self->uri);
        self->uri = NULL;
    }

    if (self->payload_buf) {
        dm_lite_free(self->payload_buf);
        self->payload_buf = NULL;
    }

    if (self->product_key) {
        dm_lite_free(self->product_key);
        self->product_key = NULL;
    }

    if (self->device_name) {
        dm_lite_free(self->device_name);
        self->device_name = NULL;
    }
#ifdef MEMORY_NO_COPY
    self->params_data_buf = NULL;
    self->raw_data_buf = NULL;
    self->raw_data_length = 0;
#else
    if (self->params_data_buf) {
        dm_lite_free(self->params_data_buf);
        self->params_data_buf = NULL;
    }

    if (self->raw_data_buf) {
        dm_lite_free(self->raw_data_buf);
        self->raw_data_buf = NULL;
        self->raw_data_length = 0;
    }
#endif
    if (self->version) {
        dm_lite_free(self->version);
        self->version = NULL;
    }

    self->id = 0;

    if (self->param_list) {
        list_t** list = self->param_list;
        list_iterator(list, clear_req_rsp_params, self);
        (*list)->clear(list);
    }

    if (self->method) {
        dm_lite_free(self->method);
        self->method = NULL;
    }
}

static void* cmp_message_info_get_uri(void* _self)
{
    cmp_message_info_t* self = _self;

    return self->uri;
}

static void* cmp_message_info_get_payload(void* _self)
{
    cmp_message_info_t* self = _self;

    return self->payload_buf;
}

static void cmp_message_info_set_id(void* _self, int id)
{
    cmp_message_info_t* self = _self;
    self->id = id;
}

static int cmp_message_info_get_id(void* _self)
{
    cmp_message_info_t* self = _self;
    return self->id;
}

static void cmp_message_info_set_version(void* _self, const char* version)
{
    cmp_message_info_t* self = _self;

    assert(version);

    if (self->version) {
        dm_lite_free(self->version);
        self->version = NULL;
    }
    assert(version);
    if (version) deep_strcpy(&self->version, version);
}

static char* cmp_message_info_get_version(void* _self)
{
    cmp_message_info_t* self = _self;

    return self->version;
}

static void cmp_message_info_add_params_data_item(void* _self, const char* key, const char* value)
{
    cmp_message_info_t* self = _self;
    list_t** list = self->param_list;
    req_rsp_param_t* req_rsp_param;

    req_rsp_param = (req_rsp_param_t*)dm_lite_calloc(1, sizeof(req_rsp_param_t));

    assert(req_rsp_param);

    if (req_rsp_param == NULL) return;

    if (key && value) {
        deep_strcpy(&req_rsp_param->key, key);
        deep_strcpy(&req_rsp_param->value, value);
    }

    list_insert(list, req_rsp_param);
}

static void cmp_message_info_set_method(void* _self, const char* method)
{
    cmp_message_info_t* self = _self;

    assert(method);

    if (self->method) {
        dm_lite_free(self->method);
        self->method = NULL;
    }
    assert(method);
    if (method) deep_strcpy(&self->method, method);
}

static char* cmp_message_info_get_method(void* _self)
{
    cmp_message_info_t* self = _self;

    return self->method;
}

static void serialize_params_data(void* _req_rsp_param, va_list* _params)
{
    req_rsp_param_t* req_rsp_param = _req_rsp_param;
    cmp_message_info_t* cmp_message_info;
    char* params;
    int len = 6; /* "key":"value"\0 */

    cmp_message_info = va_arg(*_params, void*);
    params = va_arg(*_params, char*);

    if(0 != cmp_message_info->ret) return;
    assert(req_rsp_param && cmp_message_info && req_rsp_param->key && req_rsp_param->value && params);

    if (strcmp(params, "{}") == 0) {
        *(params + 1) = 0;
    }
    else {
        *(params + strlen(params) - 1) = ','; /* change from '}' to ',' */
    }

    if (req_rsp_param && cmp_message_info && req_rsp_param->key && req_rsp_param->value) {
        len += strlen(req_rsp_param->key);
        len += strlen(req_rsp_param->value);

        /* check if there is enough room for new key-value. */
        if (len > (CMP_MESSAGE_INFO_PARAMS_LENGTH_MAX - strlen(params))) {
            cmp_message_info->ret = -1;

            dm_printf("\n[err] param buffer is short,len(%d) available(%lu)\n", len,(CMP_MESSAGE_INFO_PARAMS_LENGTH_MAX - strlen(params)));
            return;
        }

        dm_sprintf(params + strlen(params), "\"%s\":%s}", req_rsp_param->key, req_rsp_param->value);
    }
}

static int cmp_message_info_serialize_to_payload_request(void* _self)
{
    cmp_message_info_t* self = _self;
    const list_t** list = self->param_list;
    char params[CMP_MESSAGE_INFO_PARAMS_LENGTH_MAX] = {'{', '}', 0};
#if 0
    char request[CMP_MESSAGE_INFO_PARAMS_LENGTH_MAX + 32] = {0};
#endif
    int ret = -1;

    assert(self->version && self->method && list && (*list));
    if (self->version && self->method && list && (*list)) {
        self->ret = 0;
        list_iterator(list, serialize_params_data, self, params);
#if 0
        dm_snprintf(request, CMP_MESSAGE_INFO_PARAMS_LENGTH_MAX + 32, "{\"id\":%d,\"version\":\"%s\",\"params\":%s,\"method\":\"%s\"}",
                    self->id, self->version, params, self->method);
        cmp_message_info_set_payload(self, request, strlen(request));
#endif
        if(0 == self->ret) {
        cmp_message_info_set_params_data(self, params);

        /* for debug only. */
        dm_printf("\nrequest params:\n%s\n\n", params);

        ret = 0;
        }
        self->ret = 0;
    }

    return ret;
}

static int cmp_message_info_serialize_to_payload_response(void* _self)
{
    cmp_message_info_t* self = _self;
    const list_t** list = self->param_list;
    char data[CMP_MESSAGE_INFO_PARAMS_LENGTH_MAX] = {'{', '}', 0};
#if 0
    char response[CMP_MESSAGE_INFO_PARAMS_LENGTH_MAX + 32] = {0};
#endif
    int ret = -1;

    assert(list && (*list));
    if (list && (*list)) {
        self->ret = 0;
        list_iterator(list, serialize_params_data, self, data);
#if 0
        dm_snprintf(response, CMP_MESSAGE_INFO_PARAMS_LENGTH_MAX + 32, "{\"id\":%d,\"code\":%d,\"data\":%s}",
                    self->id, self->code, data);
        cmp_message_info_set_payload(self, response, strlen(response));
#endif
        if(0 == self->ret) {
        cmp_message_info_set_params_data(self, data);

        /* for debug only. */
        dm_printf("\nresponse data:\n%s\n\n", data);

        ret = 0;
        }
        self->ret = 0;
    }

    return ret;
}

static void cmp_message_info_set_message_type(void* _self, int message_type)
{
    cmp_message_info_t* self = _self;

    self->message_type = message_type;
}

static int cmp_message_info_get_message_type(void* _self)
{
    cmp_message_info_t* self = _self;

    return self->message_type;
}
#ifdef MEMORY_NO_COPY
static char* cmp_message_info_get_params_data(void* _self, int start)
{
    cmp_message_info_t* self = _self;

    if (start) {
        return self->params_data_buf;
    } else {
        return self->params_data_buf + self->params_data_buf_prefix_len;
    }
}
#else
static char* cmp_message_info_get_params_data(void* _self)
{
    cmp_message_info_t* self = _self;

    return self->params_data_buf;
}
#endif


/* malloc mem and copy payload. */
static void cmp_message_info_set_params_data(void* _self, char* params_data_buf)
{
    cmp_message_info_t* self = _self;
#ifdef MEMORY_NO_COPY
    int param_data_buf_length = 0;
    char temp_buf[128] = {0};
    int prefix_len = 0;
#endif
    assert(params_data_buf);

    if (self->params_data_buf) {
        dm_lite_free(self->params_data_buf);
        self->params_data_buf = NULL;
    }
#ifdef MEMORY_NO_COPY
    if (params_data_buf) {
        if (self->message_type == CMP_MESSAGE_INFO_MESSAGE_TYPE_REQUEST) {
            snprintf(temp_buf, sizeof(temp_buf), "{\"id\":%d,\"version\":\"%s\",\"method\":\"%s\",\"params\":",
                     self->id, self->version, self->method);
        } else if (self->message_type == CMP_MESSAGE_INFO_MESSAGE_TYPE_RESPONSE) {
            snprintf(temp_buf, sizeof(temp_buf), "{\"id\":%d,\"code\":%d,\"data\":",
                     self->id, self->code);
        } else {
            return;
        }
        param_data_buf_length = strlen(params_data_buf);
        prefix_len = strlen(temp_buf);
        self->params_data_buf = dm_lite_calloc(1, param_data_buf_length + prefix_len + 1 + 1);
        strcpy(self->params_data_buf + prefix_len, params_data_buf);
        memset(self->params_data_buf, 0, prefix_len);
        self->params_data_buf_prefix_len = prefix_len;
    }
#else
    if (params_data_buf) deep_strcpy(&self->params_data_buf, params_data_buf);
#endif
}

/* malloc mem and copy payload. */
static int cmp_message_info_set_raw_data_and_length(void* _self, void* raw_data, int raw_data_length)
{
    cmp_message_info_t* self = _self;

    if (self->raw_data_buf) {
        dm_lite_free(self->params_data_buf);
        self->raw_data_buf = NULL;
    }

    self->raw_data_buf = dm_lite_calloc(1, raw_data_length);
    assert(self->raw_data_buf);
    if (self->raw_data_buf == NULL) return -1;
    self->raw_data_length = raw_data_length;

    memcpy(self->raw_data_buf, raw_data, raw_data_length);

    return 0;
}


static void* cmp_message_info_get_raw_data(void* _self)
{
    cmp_message_info_t* self = _self;

    return self->raw_data_buf;
}

static int cmp_message_info_get_raw_data_length(void* _self)
{
    cmp_message_info_t* self = _self;

    return self->raw_data_length;
}

static char* cmp_message_info_get_product_key(void* _self)
{
    cmp_message_info_t* self = _self;

    return self->product_key;
}

/* malloc mem and copy payload. */
static void cmp_message_info_set_product_key(void* _self, char* product_key)
{
    cmp_message_info_t* self = _self;

    if (self->product_key) {
        dm_lite_free(self->product_key);
        self->product_key = NULL;
    }
    assert(product_key);
    if (product_key) deep_strcpy(&self->product_key, product_key);
}

static char* cmp_message_info_get_device_name(void* _self)
{
    cmp_message_info_t* self = _self;

    return self->device_name;
}

/* malloc mem and copy payload. */
static void cmp_message_info_set_device_name(void* _self, char* device_name)
{
    cmp_message_info_t* self = _self;

    if (self->device_name) {
        dm_lite_free(self->device_name);
        self->device_name = NULL;
    }
    assert(device_name);
    if (device_name) deep_strcpy(&self->device_name, device_name);
}

static void cmp_message_info_set_code(void* _self, int code)
{
    cmp_message_info_t* self = _self;
    self->code = code;
}

static int cmp_message_info_get_code(void* _self)
{
    cmp_message_info_t* self = _self;
    return self->code;
}

static const message_info_t _cmp_message_info_class = {
    sizeof(cmp_message_info_t),
    string_cmp_message_info_class_name,
    cmp_message_info_ctor,
    cmp_message_info_dtor,
    cmp_message_info_set_uri,
    cmp_message_info_set_payload,
    cmp_message_info_clear,
    cmp_message_info_get_uri,
    cmp_message_info_get_payload,
    cmp_message_info_set_id,
    cmp_message_info_get_id,
    cmp_message_info_set_version,
    cmp_message_info_get_version,
    cmp_message_info_add_params_data_item,
    cmp_message_info_set_method,
    cmp_message_info_get_method,
    cmp_message_info_serialize_to_payload_request,
    cmp_message_info_serialize_to_payload_response,
    cmp_message_info_set_message_type,
    cmp_message_info_get_message_type,
    cmp_message_info_get_params_data,
    cmp_message_info_set_params_data,
    cmp_message_info_set_raw_data_and_length,
    cmp_message_info_get_raw_data,
    cmp_message_info_get_raw_data_length,
    cmp_message_info_get_product_key,
    cmp_message_info_set_product_key,
    cmp_message_info_get_device_name,
    cmp_message_info_set_device_name,
    cmp_message_info_set_code,
    cmp_message_info_get_code,
};

const void* get_cmp_message_info_class()
{
    return &_cmp_message_info_class;
}

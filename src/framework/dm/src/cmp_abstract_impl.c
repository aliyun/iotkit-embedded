#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "interface/cmp_abstract.h"
#include "interface/log_abstract.h"
#include "cmp_abstract_impl.h"
#include "logger.h"
#include "cmp_message_info.h"

#include "dm_import.h"

#define CMP_IMPL_EXTENTED_ROOM_FOR_STRING_MALLOC 1
static int cmp_impl_deinit(void* _self, const void* option);

static const char string_cmp_abstrct_impl_class_name[] __DM_READ_ONLY__ =  "cmp_impl_cls";
static const char string_down_raw[] __DM_READ_ONLY__ = "down_raw";
static const char string_down_raw_reply[] __DM_READ_ONLY__ = "down_raw_reply";
static const char string_up_raw[] __DM_READ_ONLY__ = "up_raw";
static const char string_up_raw_reply[] __DM_READ_ONLY__ = "up_raw_reply";
static const char string__reply[] __DM_READ_ONLY__ = "_reply";

static void* cmp_impl_ctor(void* _self, va_list* params)
{
    cmp_abstract_impl_t* self = _self;

    self->cmp_inited = 0;

    return self;
}

static void* cmp_impl_dtor(void* _self)
{
    cmp_abstract_impl_t* self = _self;

    if (self->cmp_inited) cmp_impl_deinit(self, NULL);

    return self;
}

static int cmp_impl_init(void* _self, const char* _product_key, const char* _device_name, const char* _device_secret,
                         const char* _device_id, iotx_cmp_event_handle_func_fpt event_cb, void* pcontext, dm_cloud_domain_type_t domain_type)
{
    cmp_abstract_impl_t* self = _self;
    iotx_cmp_init_param_t init_param;

    int ret = SUCCESS_RETURN;

    if (!_product_key || !_device_name || !_device_secret || !_device_id || !event_cb || !pcontext) return -1;

    if (self->cmp_inited) return ret;

    init_param.event_func = event_cb;
    init_param.user_data = pcontext;

    init_param.domain_type = (iotx_cmp_cloud_domain_types_t)domain_type;
    init_param.secret_type = IOTX_CMP_DEVICE_SECRET_DEVICE;

    ret = IOT_CMP_Init(&init_param, NULL);

    dm_log_debug("ret = IOT_CMP_Init() = %d\n", ret);

    if (FAIL_RETURN == ret) {
        dm_printf("init fail\n");
    } else {
        self->cmp_inited = 1;
    }

    return ret;
}

static int cmp_impl_deinit(void* _self, const void* option)
{
    cmp_abstract_impl_t* self = _self;

    (void)option; /* prevent build warning. */

    self->cmp_inited = 0;

    return IOT_CMP_Deinit(NULL);;
}

static int cmp_impl_regist(void* _self, char* uri, iotx_cmp_register_func_fpt register_cb, void* pcontext, void* option)
{
    iotx_cmp_register_param_t register_param;
    int ret;

    if(!uri || !register_cb || !pcontext) {
        dm_log_err("invalid param!");
        return FAIL_RETURN;
    }

    /* Subscribe the specific topic */
    register_param.URI_type = IOTX_CMP_URI_UNDEFINE;
    register_param.URI = uri;

    if (strstr(uri, string_down_raw) || strstr(uri, string_down_raw_reply) || strstr(uri, string_up_raw) || strstr(uri, string_up_raw_reply)) {
        register_param.message_type = IOTX_CMP_MESSAGE_RAW;
    } else if (strstr(uri, string__reply)) {
        register_param.message_type = IOTX_CMP_MESSAGE_RESPONSE;
    } else {
        register_param.message_type = IOTX_CMP_MESSAGE_REQUEST;
    }

    register_param.register_func = register_cb;
    register_param.user_data = pcontext;
    ret = IOT_CMP_Register(&register_param, NULL);

    dm_log_debug("ret = IOT_CMP_Register() = %d\n", ret);

    if (FAIL_RETURN == ret) {
        dm_printf("register fail\n");
    }

    return ret;
}

static int cmp_impl_unregist(void* _self, char* uri, void* option)
{
    cmp_abstract_impl_t* self = _self;
    iotx_cmp_unregister_param_t unregister_param;
    int ret = -1;

    (void)option; /* prevent build warning. */

    if (!uri || !self->cmp_inited) return -1;

    unregister_param.URI_type = IOTX_CMP_URI_UNDEFINE;
    unregister_param.URI = uri;

    ret = IOT_CMP_Unregister(&unregister_param, NULL);

    dm_log_debug("ret = IOT_CMP_Unregister() = %d\n", ret);

    if (FAIL_RETURN == ret) {
        dm_printf("unregister fail\n");
    }

    return ret;
}

#ifdef MEMORY_NO_COPY
void recycle_memory(void* _user_data)
{
    void* p = _user_data;
    dm_lite_free(p);
}
#endif

static int cmp_impl_send(void* _self, message_info_t** msg, void* option)
{
    message_info_t** message_info = msg;
    iotx_cmp_message_info_t iotx_cmp_message_info = {0};
    iotx_cmp_send_peer_t send_peer;
    char* device_name;
    char* product_key;
    int ret;
    int message_type;
    (void)option; /* prevent build warning. */

    if (!msg) return -1;

    iotx_cmp_message_info.id = (*message_info)->get_id(message_info);
    iotx_cmp_message_info.code = (*message_info)->get_code(message_info);
    message_type = (*message_info)->get_message_type(message_info);
    iotx_cmp_message_info.message_type = message_type == CMP_MESSAGE_INFO_MESSAGE_TYPE_REQUEST ? IOTX_CMP_MESSAGE_REQUEST : (message_type == CMP_MESSAGE_INFO_MESSAGE_TYPE_RESPONSE ? IOTX_CMP_MESSAGE_RESPONSE : IOTX_CMP_MESSAGE_RAW);
    iotx_cmp_message_info.URI = (*message_info)->get_uri(message_info);
    iotx_cmp_message_info.URI_type = IOTX_CMP_URI_UNDEFINE;
    iotx_cmp_message_info.method = (*message_info)->get_method(message_info);
#ifdef MEMORY_NO_COPY
    iotx_cmp_message_info.parameter = iotx_cmp_message_info.message_type == IOTX_CMP_MESSAGE_RAW ? (*message_info)->get_raw_data(message_info) : (*message_info)->get_params_data(message_info, 0);
#else
    iotx_cmp_message_info.parameter = iotx_cmp_message_info.message_type == IOTX_CMP_MESSAGE_RAW ? (*message_info)->get_raw_data(message_info) : (*message_info)->get_params_data(message_info);
#endif
    iotx_cmp_message_info.parameter_length = iotx_cmp_message_info.message_type == IOTX_CMP_MESSAGE_RAW ? (*message_info)->get_raw_data_length(message_info) : strlen(iotx_cmp_message_info.parameter);
#ifdef MEMORY_NO_COPY
    iotx_cmp_message_info.recycle_memory_fp = recycle_memory;
    iotx_cmp_message_info.user_data = iotx_cmp_message_info.message_type == IOTX_CMP_MESSAGE_RAW ? (*message_info)->get_raw_data(message_info) : (*message_info)->get_params_data(message_info, 1);;
#endif
    product_key = (*message_info)->get_product_key(message_info);
    device_name = (*message_info)->get_device_name(message_info);

    memset(&send_peer, 0, sizeof(iotx_cmp_send_peer_t));

    strncpy(send_peer.device_name, device_name, sizeof(send_peer.device_name));
    strncpy(send_peer.product_key, product_key, sizeof(send_peer.product_key));

    ret = IOT_CMP_Send(&send_peer, &iotx_cmp_message_info, NULL);

    dm_log_debug("ret = IOT_CMP_Send() = %d\n", ret);

    (*message_info)->clear(message_info);

    return ret;
}

#ifndef CMP_SUPPORT_MULTI_THREAD
static int cmp_impl_yield(void* _self, int timeout_ms)
{
    return IOT_CMP_Yield(timeout_ms, NULL);
}
#endif

static const cmp_abstract_t _cmp_impl_class = {
    sizeof(cmp_abstract_impl_t),
    string_cmp_abstrct_impl_class_name,
    cmp_impl_ctor,
    cmp_impl_dtor,
    cmp_impl_init,
    cmp_impl_deinit,
    cmp_impl_regist,
    cmp_impl_unregist,
    cmp_impl_send,
#ifndef CMP_SUPPORT_MULTI_THREAD
    cmp_impl_yield,
#endif
};

const void* get_cmp_impl_class()
{
    return &_cmp_impl_class;
}

#ifndef CMP_ABSTRACT_H
#define CMP_ABSTRACT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <stdarg.h>

#include "iot_import.h"
#include "iot_export_cmp.h"
#include "iot_export_dm.h"

#include "message_info_abstract.h"

typedef struct {
    size_t size;
    const char*  _class_name;
    void* (*ctor)(void* _self, va_list* params);
    void* (*dtor)(void* _self);
    int   (*init)(void* _self, const char* product_key, const char* device_name, const char* device_secret,
                  const char* device_id, iotx_cmp_event_handle_func_fpt event_cb, void* pcontext, dm_cloud_domain_type_t domain_type);
    int   (*deinit)(void* _self, const void* option);
    int   (*regist)(void* _self, char* uri, iotx_cmp_register_func_fpt register_cb, void* pcontext, void* option);
    int   (*unregist)(void* _self, char* uri, void* option);
    int   (*send)(void* _self, message_info_t** msg, void* option);
#ifndef CMP_SUPPORT_MULTI_THREAD
    int   (*yield)(void* _self, int timeout_ms);
#endif
} cmp_abstract_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_ABSTRACT_H */

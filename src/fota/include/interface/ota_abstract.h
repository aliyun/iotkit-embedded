#ifndef OTA_ABSTRACT_H
#define OTA_ABSTRACT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdarg.h>
#include <stdlib.h>

typedef enum {
    service_fota_callback_type_new_version_detected = 10,

    service_fota_callback_type_number,
} service_fota_callback_type_t;

typedef void (*handle_service_fota_callback_fp_t)(service_fota_callback_type_t callback_type, const char* version);

typedef struct {
    size_t size;
    char*  _class_name;
    void* (*ctor)(void* _self, va_list* params);
    void* (*dtor)(void* _self);
    void  (*start)(void* _self);
    int   (*write)(void* _self, void* data, int data_length);
    int   (*end)(void* _self);
    int   (*perform_ota_service)(void* _self, void* _data_buf, int _data_buf_length);
    void  (*install_callback_function)(void* _self, handle_service_fota_callback_fp_t linkkit_callback_fp);
} fota_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OTA_ABSTRACT_H */

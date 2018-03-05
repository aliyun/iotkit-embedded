#ifndef SERVICE_OTA_H
#define SERVICE_OTA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SERVICE_FOTA_MODULE_NAME "service_fota"

#define SERVICE_FOTA_CLASS get_service_ota_class()

void* service_ota_lite_malloc(size_t size);
void* service_ota_lite_calloc(size_t nmemb, size_t size);
void  service_ota_lite_free_func(void* ptr);

#define service_ota_lite_free(ptr)              \
    do { \
        if(!ptr) { \
            log_err("service_ota_lite_free(%p) aborted.", ptr); \
            break; \
        } \
        \
        LITE_free_internal((void *)ptr); \
        ptr = NULL; \
    } while(0)

typedef struct {
    const void* _;
    void*       _data_buf;
    char*       _version;
    int         _data_buf_length;
    int         _total_len;
    char*       _ota_version;
    void*       _linkkit_callback_fp;
    char*       _current_verison;
    int         _ota_inited;
    int         _destructing;
} service_ota_t;

extern const void* get_service_ota_class();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SERVICE_OTA_H */

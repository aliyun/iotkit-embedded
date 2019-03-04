/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef IOT_EXPORT_HTTP2_STREAM_H
#define IOT_EXPORT_HTTP2_STREAM_H

#include "infra_types.h"
#include "infra_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOT_HTTP2_RES_OVERTIME_MS                 (10000)
#define IOT_HTTP2_KEEP_ALIVE_CNT                  (2)
#define IOT_HTTP2_KEEP_ALIVE_TIME                 (30*1000) /* in seconds */

#define MAKE_HEADER(NAME, VALUE)                                             \
    {                                                                        \
        (char *) NAME, (char *)VALUE, sizeof(NAME) - 1, sizeof(VALUE) - 1    \
    }

#define MAKE_HEADER_CS(NAME, VALUE)                                          \
    {                                                                        \
        (char *) NAME, (char *)VALUE, strlen(NAME) , strlen(VALUE)           \
    }

typedef struct {
    char  *product_key;
    char  *device_name;
    char  *device_secret;
    char  *url;
    int   port;
} device_conn_info_t;

typedef struct http2_header_struct {
    char *name;     /* header name */
    char *value;    /* the value of name */
    int  namelen;   /* the length of header name */
    int  valuelen;  /* the length of value */
} http2_header;

typedef struct {
    http2_header      *nva;
    int               num;
} header_ext_info_t;

typedef enum {
    STREAM_TYPE_DOWNLOAD,
    STREAM_TYPE_UPLOAD,
    STREAM_TYPE_AUXILIARY,
    STREAM_TYPE_NUM
} stream_type_t;

typedef void (*on_stream_header_callback)(uint32_t stream_id, char *channel_id, int cat, const uint8_t *name,
        uint32_t namelen, const uint8_t *value, uint32_t valuelen, uint8_t flags, void *user_data);

typedef void (*on_stream_chunk_recv_callback)(uint32_t stream_id, char *channel_id,
        const uint8_t *data, uint32_t len, uint8_t flags, void *user_data);

typedef void (*on_stream_close_callback)(uint32_t stream_id, char *channel_id, uint32_t error_code, void *user_data);

typedef void (*on_stream_frame_send_callback)(uint32_t stream_id, char *channel_id, int type, uint8_t flags, void *user_data);

typedef void (*on_stream_frame_recv_callback)(uint32_t stream_id, char *channel_id, int type, uint8_t flags,void *user_data);

typedef void (*on_reconnect_callback)();
typedef void (*on_disconnect_callback)();

typedef struct {
    on_stream_header_callback       on_stream_header_cb;
    on_stream_chunk_recv_callback   on_stream_chunk_recv_cb;
    on_stream_close_callback        on_stream_close_cb;
    on_stream_frame_send_callback   on_stream_frame_send_cb;
    on_stream_frame_recv_callback   on_stream_frame_recv_cb;
    on_reconnect_callback           on_reconnect_cb;
    on_disconnect_callback          on_disconnect_cb;
} http2_stream_cb_t;

typedef struct {
    char               *stream;             /* point to stream data buffer */
    uint32_t            stream_len;         /* file content length */
    uint32_t            send_len;           /* data had sent length */
    uint32_t            packet_len;         /* one packet length */
    const char          *identify;          /* path string to identify a stream service */
    int                 h2_stream_id;       /* stream identifier which is a field in HTTP2 frame */
    char                *channel_id;        /* string return by server to identify a specific stream channel,
                                            different from stream identifier which is a field in HTTP2 frame */
    void                *user_data;         /* user data brought in at stream open */
} stream_data_info_t;

#ifdef FS_ENABLED
/* bit define of file override option */
#define UPLOAD_FILE_OPT_BIT_OVERWRITE       (0x00000001)
#define UPLOAD_FILE_OPT_BIT_RESUME          (0x00000002)
#define UPLOAD_FILE_OPT_BIT_CRC64           (0x00000004)
#define UPLOAD_FILE_OPT_BIT_SPECIFIC_LEN    (0x00000008)

/* file upload option define */
typedef struct {
    const char *file_path;
    const char *upload_id;      /* a specific id used to indicate one upload session, only required when UPLOAD_FILE_OPT_BIT_RESUME option set */
    uint32_t upload_len;        /* used to indicate the upload length, only required when UPLOAD_FILE_OPT_BIT_SPECIFIC_LEN option set */
    uint32_t opt_bit_map;
} http2_file_upload_params_t;

/* error code for file upload */
typedef enum {
    UPLOAD_STOP_BY_IOCTL        = -14,
    UPLOAD_HTTP2_HANDLE_NULL    = -13,
    UPLOAD_LEN_IS_ZERO          = -12,
    UPLOAD_FILE_PATH_IS_NULL    = -11,
    UPLOAD_ID_IS_NULL           = -10,
    UPLOAD_FILE_NOT_EXIST     = -9,
    UPLOAD_FILE_READ_FAILED   = -8,
    UPLOAD_STREAM_OPEN_FAILED = -7,
    UPLOAD_STREAM_SEND_FAILED = -6,
    UPLOAD_MALLOC_FAILED      = -5,
    UPLOAD_NULL_POINT         = -2,
    UPLOAD_ERROR_COMMON       = -1,
    UPLOAD_SUCCESS            = 0,
} http2_file_upload_result_t;

/* callback function type define */
typedef void (* http2_file_upload_cb_t)(const char *file_path, const char *upload_id, int result, void *user_data);

DLL_IOT_API int IOT_HTTP2_Stream_UploadFile(void *http2_handle, http2_file_upload_params_t *params, http2_file_upload_cb_t cb, void *user_data);
#endif /* #ifdef FS_ENABLED */

DLL_IOT_API void *IOT_HTTP2_Connect(device_conn_info_t *conn_info, http2_stream_cb_t *user_cb);
DLL_IOT_API int IOT_HTTP2_Stream_Open(void *handle, stream_data_info_t *info, header_ext_info_t *header);
DLL_IOT_API int IOT_HTTP2_Stream_Send(void *handle, stream_data_info_t *info, header_ext_info_t *header);
DLL_IOT_API int IOT_HTTP2_Stream_Query(void *handle, stream_data_info_t *info, header_ext_info_t *header);
DLL_IOT_API int IOT_HTTP2_Stream_Close(void *handle, stream_data_info_t *info);
DLL_IOT_API int IOT_HTTP2_Stream_Send_Message(void *handle, const char *identify,char *channel_id, char *data,
                                              uint32_t data_len, header_ext_info_t *header);
DLL_IOT_API int IOT_HTTP2_Disconnect(void *handle);

#ifdef __cplusplus
}
#endif

#endif /* IOT_EXPORT_FILE_UPLOADER_H */

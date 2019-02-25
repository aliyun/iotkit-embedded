/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "http2_api.h"
#include "http2_wrapper.h"


#define HTTP2_ONLINE_SERVER_URL       "100.69.92.182"
#define HTTP2_ONLINE_SERVER_PORT      9999
#define HTTP2_PRODUCT_KEY             "a19L84Ao5he"
#define HTTP2_DEVICE_NAME             "http2_01"
#define HTTP2_DEVICE_SECRET           "DLvc4RJjnWFpDx30TyS2beU4UvVTcacv"

#define EXAMPLE_TRACE(fmt, ...)                        \
    do {                                               \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__);                \
        HAL_Printf("%s", "\r\n");                      \
    } while (0)

static int upload_end = 0;
static void on_header(uint32_t stream_id, char *channel_id,int cat,const uint8_t *name,uint32_t namelen,
                              const uint8_t *value,uint32_t valuelen, uint8_t flags ,void *user_data)
{
    EXAMPLE_TRACE("~~~~~stream_id = %d, channel_id=%s, name = %s, value = %s, flag = %d user_data =%p\n", stream_id,channel_id,name,value,flags ,user_data);
}

static void on_chunk_recv(uint32_t stream_id, char *channel_id,const uint8_t *data, uint32_t len,uint8_t flags,void *user_data)
{
    EXAMPLE_TRACE("~~~~~stream_id = %d, channel_id=%s, data = %.*s, len = %d flag = %d\n", stream_id, channel_id, len, data, len, flags);
}
static void on_stream_close(uint32_t stream_id, char *channel_id,uint32_t error_code,void *user_data)
{
    EXAMPLE_TRACE("~~~~~stream_id = %d channel_id=%s, error_code = %d\n", stream_id,channel_id,error_code);
}
static void on_stream_frame_send(uint32_t stream_id, char *channel_id, int type, uint8_t flags,void *user_data){
    EXAMPLE_TRACE("~~~~~stream_id = %d user_data =%p, type = %d\n", stream_id,user_data,type);
}

static http2_stream_cb_t my_cb = {
    .on_stream_header_cb = on_header,
    .on_stream_chunk_recv_cb = on_chunk_recv,
    .on_stream_close_cb = on_stream_close,
    .on_stream_frame_send_cb = on_stream_frame_send,
};

void upload_file_result(const char * path,int result, void * user_data)
{
    upload_end ++;
    EXAMPLE_TRACE("===========filename = %s,result =%d,finish num =%d=========", path,result,upload_end);

}

static int http2_stream_test(char **argv,int argc)
{
    int ret;
    device_conn_info_t conn_info;
    void *handle;
    int goal_num = 0;
    int i;
    memset(&conn_info, 0, sizeof( device_conn_info_t));
    conn_info.product_key = HTTP2_PRODUCT_KEY;
    conn_info.device_name = HTTP2_DEVICE_NAME;
    conn_info.device_secret = HTTP2_DEVICE_SECRET;
    conn_info.url = HTTP2_ONLINE_SERVER_URL;
    conn_info.port = HTTP2_ONLINE_SERVER_PORT;

    handle = IOT_HTTP2_Connect(&conn_info,&my_cb);
    if(handle == NULL) {
        return -1;
    }

    for (i=1; i<argc; i++) {
        http2_file_upload_opt_t opt = {
            UPLOAD_FILE_OPT_BIT_OVERWRITE   /* setup to 0 if you wouldn't overrite the file */
        };
        ret = IOT_HTTP2_Stream_UploadFile(handle, argv[i], "c/iot/sys/thing/file/upload", upload_file_result, &opt, NULL);
        if(ret == 0) {
            goal_num++;
        }
    }
    while(upload_end != goal_num) {
        HAL_SleepMs(200);
    }
    ret = IOT_HTTP2_Disconnect(handle);
    EXAMPLE_TRACE("close connect %d\n",ret);
    return 0;
}

int main(int argc, char **argv)
{
    int ret;

    if (argc < 2) {
        HAL_Printf("no file name input!\n");
        return 0;
    }

    ret = http2_stream_test(argv, argc);
    return ret;
}

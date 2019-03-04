/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "http2_api.h"
#include "http2_wrapper.h"


#define HTTP2_ONLINE_SERVER_URL       "100.67.141.158"
#define HTTP2_ONLINE_SERVER_PORT      8443
#define HTTP2_PRODUCT_KEY             "a1IgnOND7vI"
#define HTTP2_DEVICE_NAME             "H2_FS01"
#define HTTP2_DEVICE_SECRET           "r9XSyeZOu6swgV8etCcCGmgQhBSpGBau"


#define EXAMPLE_TRACE(fmt, ...)                        \
    do {                                               \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__);                \
        HAL_Printf("%s", "\r\n");                      \
    } while (0)

static int upload_end = 0;
static char g_upload_id[50] = {0};

void upload_file_result(const char *file_path, const char *upload_id, int result, void *user_data)
{
    upload_end++;
    EXAMPLE_TRACE("=========== file_path = %s, upload_id = %s, result = %d, finish num = %d ===========", file_path, upload_id, result, upload_end);

    memcpy(g_upload_id, upload_id, strlen(upload_id));
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

    handle = IOT_HTTP2_Connect(&conn_info, NULL);
    if(handle == NULL) {
        return -1;
    }

    for (i=1; i<argc; i++) {
        http2_file_upload_params_t fs_params;
        memset(&fs_params, 0, sizeof(fs_params));
        fs_params.file_path = argv[i];
        fs_params.upload_len = 1024 * 1024 * 100;
        fs_params.opt_bit_map = UPLOAD_FILE_OPT_BIT_OVERWRITE;

        ret = IOT_HTTP2_Stream_UploadFile(handle, &fs_params, upload_file_result, NULL);
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

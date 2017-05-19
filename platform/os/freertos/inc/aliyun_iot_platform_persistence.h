#ifndef ALIYUN_IOT_PLATFORM_PERSISTENCE_H
#define ALIYUN_IOT_PLATFORM_PERSISTENCE_H

#include <stdio.h>
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"

typedef struct ALIYUN_IOT_FILE_HANDLE
{
    FILE *fd;
}ALIYUN_IOT_FILE_HANDLE_S;

typedef enum ALIYUN_IOT_FILE_FLAG
{
    RD_FLAG                = 0x00000001,
    RDWR_FALG              = 0x00000002,
    CREAT_WR_FALG          = 0x00000004,
    CREAT_RDWR_FLAG        = 0x00000008,
    CREAT_APPEND_FLAG      = 0x0000000A,
    CREAT_APPEND_RDWR_FLAG = 0x00000010,
}ALIYUN_IOT_FILE_FLAG_E;

aliot_err_t aliyun_iot_file_open(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* filename,ALIYUN_IOT_FILE_FLAG_E flags);

INT32 aliyun_iot_file_close(ALIYUN_IOT_FILE_HANDLE_S*handle);

INT32 aliyun_iot_file_delete(INT8* filename);

INT32 aliyun_iot_file_write(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size,INT32 count);

INT32 aliyun_iot_file_read(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size,INT32 count);

INT32 aliyun_iot_file_fgets(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size);

INT32 aliyun_iot_file_fputs(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf);

INT32 aliyun_iot_file_whether_exist(INT8* filename);

#endif

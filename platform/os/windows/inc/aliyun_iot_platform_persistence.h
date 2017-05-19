#ifndef ALIYUN_IOT_PLATFORM_PERSISTENCE_H
#define ALIYUN_IOT_PLATFORM_PERSISTENCE_H

//#include <sys/types.h>
//#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"

typedef struct ALIYUN_IOT_FILE_HANDLE
{
    FILE* fd;
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

aliot_err_t aliyun_iot_file_open(ALIYUN_IOT_FILE_HANDLE_S*handle,int8_t* filename,ALIYUN_IOT_FILE_FLAG_E flags);

int32_t aliyun_iot_file_close(ALIYUN_IOT_FILE_HANDLE_S*handle);

int32_t aliyun_iot_file_delete(int8_t* filename);

int32_t aliyun_iot_file_write(ALIYUN_IOT_FILE_HANDLE_S*handle,int8_t* buf,int32_t size,int32_t count);

int32_t aliyun_iot_file_read(ALIYUN_IOT_FILE_HANDLE_S*handle,int8_t* buf,int32_t size,int32_t count);

int32_t aliyun_iot_file_fgets(ALIYUN_IOT_FILE_HANDLE_S*handle,int8_t* buf,int32_t size);

int32_t aliyun_iot_file_fputs(ALIYUN_IOT_FILE_HANDLE_S*handle,int8_t* buf);

int32_t aliyun_iot_file_whether_exist(int8_t* filename);

uint32_t aliyun_iot_file_ftell(ALIYUN_IOT_FILE_HANDLE_S*handle);

#endif

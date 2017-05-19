#include "../../os/windows/inc/aliyun_iot_platform_persistence.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"

aliot_err_t aliyun_iot_file_open(ALIYUN_IOT_FILE_HANDLE_S*handle,int8_t* filename,ALIYUN_IOT_FILE_FLAG_E flags)
{
    if(handle == NULL || filename == NULL)
    {
        return FAIL_RETURN;
    }

    char* flagslinux = NULL;

    switch (flags)
    {
        case RD_FLAG:
            flagslinux = "r";
            break;
        case RDWR_FALG:
            flagslinux = "r+";
            break;
        case CREAT_WR_FALG:
            flagslinux = "w";
            break;
        case CREAT_RDWR_FLAG:
            flagslinux = "w+";
            break;
        case CREAT_APPEND_FLAG:
            flagslinux = "a";
            break;
        case CREAT_APPEND_RDWR_FLAG:
            flagslinux = "a+";
            break;
        default:
            return FAIL_RETURN;
            break;
    }

    handle->fd = fopen(filename,flagslinux);
    if (handle->fd < 0)
    {
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

int32_t aliyun_iot_file_close(ALIYUN_IOT_FILE_HANDLE_S*handle)
{
    return fclose(handle->fd);
}

int32_t aliyun_iot_file_delete(int8_t* filename)
{
    return remove(filename);
}

int32_t aliyun_iot_file_write(ALIYUN_IOT_FILE_HANDLE_S*handle,int8_t* buf,int32_t size,int32_t count)
{
    int32_t lens = fwrite(buf, size,count,handle->fd);

    return lens;
}

int32_t aliyun_iot_file_read(ALIYUN_IOT_FILE_HANDLE_S*handle,int8_t* buf,int32_t size,int32_t count)
{
    int32_t lens = fread(buf, size,count,handle->fd);

    return lens;
}

int32_t aliyun_iot_file_fgets(ALIYUN_IOT_FILE_HANDLE_S*handle,int8_t* buf,int32_t size)
{
    if(NULL == fgets(buf,size,handle->fd))
    {
        return FAIL_RETURN;
    }
    return SUCCESS_RETURN;
}

int32_t aliyun_iot_file_fputs(ALIYUN_IOT_FILE_HANDLE_S*handle,int8_t* buf)
{
    int32_t result = 0;
    result = fputs(buf,handle->fd);
    return result;
}


int32_t aliyun_iot_file_whether_exist(int8_t* filename)
{
    return access(filename, 0);
}

/***********************************************************
* 函数名称: aliyun_iot_file_ftell
* 描       述: 获取文大小
* 输入参数: INT8* filename文件名称
* 输出参数:
* 返 回  值: 同linux系统下access返回值
* 说       明: linux系统下的实现判断文件是否存在
*           源码中使用
************************************************************/
uint32_t aliyun_iot_file_ftell(ALIYUN_IOT_FILE_HANDLE_S*handle)
{
    return ftell(handle->fd);
}


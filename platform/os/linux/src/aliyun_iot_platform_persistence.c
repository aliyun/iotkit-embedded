#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_platform_persistence.h"

/***********************************************************
* 函数名称: aliyun_iot_file_open
* 描       述: 文件打开
* 输入参数: ALIYUN_IOT_FILE_HANDLE_S*handle 文件句柄
*           INT8* filename文件名
*           ALIYUN_IOT_FILE_FLAG_E flags打开形式
* 输出参数:
* 返 回  值: 0：成功，非零：失败
* 说       明: linux系统下的实现打开文件
*           源码中使用
************************************************************/
aliot_err_t aliyun_iot_file_open(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* filename,ALIYUN_IOT_FILE_FLAG_E flags)
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

/***********************************************************
* 函数名称: aliyun_iot_file_close
* 描       述: 文件关闭
* 输入参数: ALIYUN_IOT_FILE_HANDLE_S*handle 文件句柄
* 输出参数:
* 返 回  值: 同linux系统下fclose返回值
* 说       明: linux系统下的实现文件关闭
*           源码中使用
************************************************************/
INT32 aliyun_iot_file_close(ALIYUN_IOT_FILE_HANDLE_S*handle)
{
    return fclose(handle->fd);
}

/***********************************************************
* 函数名称: aliyun_iot_file_delete
* 描       述: 文件删除
* 输入参数: ALIYUN_IOT_FILE_HANDLE_S*handle 文件句柄
* 输出参数:
* 返 回  值: 同linux系统下fclose返回值
* 说       明: linux系统下的实现文件关闭
*           源码中使用
************************************************************/
INT32 aliyun_iot_file_delete(INT8* filename)
{
    return remove(filename);
}

/***********************************************************
* 函数名称: aliyun_iot_file_write
* 描       述: 文件写
* 输入参数: ALIYUN_IOT_FILE_HANDLE_S*handle 文件句柄
*          INT8* buf 缓存
*          INT32 size 要写的每个数据项的字节数
*          INT32 count 要写count个数据项
* 输出参数:
* 返 回  值: 同linux系统下fwrite返回值
* 说       明: linux系统下的实现写入文件数据
*           源码中使用
************************************************************/
INT32 aliyun_iot_file_write(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size,INT32 count)
{
    INT32 lens = fwrite(buf, size,count,handle->fd);

    return lens;
}

/***********************************************************
* 函数名称: aliyun_iot_file_read
* 描       述: 文件读
* 输入参数: ALIYUN_IOT_FILE_HANDLE_S*handle 文件句柄
*          INT8* buf 缓存
*          INT32 size 要读的每个数据项的字节数
*          INT32 count 要读count个数据项
* 输出参数:
* 返 回  值: 同linux系统下fread返回值
* 说       明: linux系统下的实现从文件中数据读取操作
*           源码中使用
************************************************************/
INT32 aliyun_iot_file_read(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size,INT32 count)
{
    INT32 lens = fread(buf, size,count,handle->fd);

    return lens;
}

/***********************************************************
* 函数名称: aliyun_iot_file_fgets
* 描       述: 行读取文件
* 输入参数: ALIYUN_IOT_FILE_HANDLE_S*handle 文件句柄
*          INT8* buf 缓存
*          INT32 size 缓存大小
* 输出参数:
* 返 回  值: 0：成功，非零：失败
* 说       明: linux系统下的实现从文件中按行获取
*           源码中使用
************************************************************/
INT32 aliyun_iot_file_fgets(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size)
{
    if(NULL == fgets(buf,size,handle->fd))
    {
        return FAIL_RETURN;
    }
    return SUCCESS_RETURN;
}

/***********************************************************
* 函数名称: aliyun_iot_file_fputs
* 描       述: 行输入文件
* 输入参数: ALIYUN_IOT_FILE_HANDLE_S*handle 文件句柄
*          INT8* buf 输入数据
* 输出参数:
* 返 回  值: 同linux系统下fputs返回值
* 说       明: linux系统下的实现数据按行输入文件
*           源码中使用
************************************************************/
INT32 aliyun_iot_file_fputs(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf)
{
    INT32 result = 0;
    result = fputs(buf,handle->fd);
    return result;
}

/***********************************************************
* 函数名称: aliyun_iot_file_whether_exist
* 描       述: 文件是否存在
* 输入参数: INT8* filename文件名称
* 输出参数:
* 返 回  值: 同linux系统下access返回值
* 说       明: linux系统下的实现判断文件是否存在
*           源码中使用
************************************************************/
INT32 aliyun_iot_file_whether_exist(INT8* filename)
{
    return access(filename, F_OK);
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
UINT32 aliyun_iot_file_ftell(ALIYUN_IOT_FILE_HANDLE_S*handle)
{
    return ftell(handle->fd);
}

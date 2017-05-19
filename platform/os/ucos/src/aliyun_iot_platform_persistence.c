#include "aliyun_iot_platform_persistence.h"

aliot_err_t aliyun_iot_file_open(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* filename,ALIYUN_IOT_FILE_FLAG_E flags)
{
    return ERROR_NO_PERSISTENCE;
}

INT32 aliyun_iot_file_close(ALIYUN_IOT_FILE_HANDLE_S*handle)
{
    return ERROR_NO_PERSISTENCE;
}

INT32 aliyun_iot_file_delete(INT8* filename)
{
    return ERROR_NO_PERSISTENCE;
}

INT32 aliyun_iot_file_write(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size,INT32 count)
{
    return ERROR_NO_PERSISTENCE;
}

INT32 aliyun_iot_file_read(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size,INT32 count)
{
    return ERROR_NO_PERSISTENCE;
}

INT32 aliyun_iot_file_fgets(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf,INT32 size)
{
    return ERROR_NO_PERSISTENCE;
}

INT32 aliyun_iot_file_fputs(ALIYUN_IOT_FILE_HANDLE_S*handle,INT8* buf)
{
    return ERROR_NO_PERSISTENCE;
}

INT32 aliyun_iot_file_whether_exist(INT8* filename)
{
    return ERROR_NO_PERSISTENCE;
}

#ifndef ALIYUN_IOT_PLATFORM_NETWORK_H
#define ALIYUN_IOT_PLATFORM_NETWORK_H

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_platform_timer.h"

/*******************************************
 * socket协议类型
*******************************************/
typedef enum IOT_NET_PROTOCOL_TYPE
{
    IOT_NET_PROTOCOL_TCP = 0,
    IOT_NET_PROTOCOL_UDP = 1,
}IOT_NET_PROTOCOL_TYPE;

/*******************************************
 * 读写类型
*******************************************/
typedef enum IOT_NET_TRANS_TYPE
{
    IOT_NET_TRANS_RECV = 0,
    IOT_NET_TRANS_SEND = 1,
}IOT_NET_TRANS_TYPE_E;

/*******************************************
 * select的结果类型，是否可读写
*******************************************/
typedef enum IOT_NET_FD_ISSET
{
    IOT_NET_FD_NO_ISSET = 0,
    IOT_NET_FD_ISSET = 1,
}IOT_NET_FD_ISSET_E;

/*******************************************
 * send接口标志
*******************************************/
typedef enum IOT_NET_TRANS_FLAGS
{
    IOT_NET_FLAGS_DEFAULT = 0,  //默认
    IOT_NET_FLAGS_DONTWAIT = 1, //非阻塞调用
}IOT_NET_TRANS_FLAGS_E;

/*******************************************
 * 系统SDK自定义errno的值
*******************************************/
#define EINTR_IOT              -2
#define EBADF_IOT              -3
#define EAGAIN_IOT   -4
#define EFAULT_IOT   -5
#define EBUSY_IOT    -6
#define EINVAL_IOT   -7
#define ENFILE_IOT   -8
#define EMFILE_IOT   -9
#define ENOSPC_IOT   -10
#define EPIPE_IOT    -11
#define EWOULDBLOCK_IOT   -12
#define ENOTSOCK_IOT      -13
#define ENOPROTOOPT_IOT   -14
#define EADDRINUSE_IOT    -15
#define EADDRNOTAVAIL_IOT  -16
#define ENETDOWN_IOT      -17
#define ENETUNREACH_IOT   -18
#define ENETRESET_IOT     -19
#define ECONNRESET_IOT    -20
#define ENOBUFS_IOT       -21
#define EISCONN_IOT       -22
#define ENOTCONN_IOT      -23
#define ETIMEDOUT_IOT     -24
#define ECONNREFUSED_IOT  -25
#define EHOSTDOWN_IOT     -26
#define EHOSTUNREACH_IOT  -27
#define ENOMEM_IOT        -28
#define EMSGSIZE_IOT      -29

typedef enum ALIYUN_NETWORK_ERROR
{
    NETWORK_PARAM_ERROR = -31,
    NETWORK_MSG_TOO_LONG = -30,
    NETWORK_OUT_OF_MEMORY = -29,
    NETWORK_SIGNAL_INTERRUPT = -28,
    NETWORK_BAD_FILEFD = -27,
    NETWORK_TRYAGAIN = -26,
    NETWORK_BADADDRESS = -25,
    NETWORK_RESOURCE_BUSY = -24,
    NETWORK_INVALID_ARGUMENT = -23,
    NETWORK_FILE_TABLE_OVERFLOW = -22,
    NETWORK_TOO_MANY_OPEN_FILES = -21,
    NETWORK_NO_SPACE_LEFT_ON_DEVICE = -20,
    NETWORK_BROKEN_PIPE = -19,
    NETWORK_OPERATION_BLOCK = -18,
    NETWORK_OPERATION_ON_NONSOCKET = -17,
    NETWORK_PROTOCOL_NOT_AVAILABLE = -16,
    NETWORK_ADDRESS_ALREADY_IN_USE = -15,
    NETWORK_CANNOT_ASSIGN_REQUESTED_ADDRESS = -14,
    NETWORK_NETWORK_IS_DOWN = -13,
    NETWORK_NETWORK_IS_UNREACHABLE = -12,
    NETWORK_CONNECT_RESET = -11,
    NETWORK_CONNECT_RESET_BY_PEER = -10,
    NETWORK_NO_BUFFER_SPACE = -9,
    NETWORK_ALREADY_CONNECTED = -8,
    NETWORK_IS_NOT_CONNECTED = -7,
    NETWORK_CONNECTION_TIMED_OUT = -6,
    NETWORK_CONNECTION_REFUSED = -5,
    NETWORK_HOST_IS_DOWN = -4,
    NETWORK_NO_ROUTE_TO_HOST = -3,
    NETWORK_TIMEOUT = -2,
    NETWORK_FAIL = -1,
    NETWORK_SUCCESS = 0,

}ALIYUN_NETWORK_ERROR_E;

/***********************************************************
* 函数名称: aliyun_iot_network_create
* 描       述: 客户端网络资源创建、连接
* 输入参数: INT8*host 域名或主机IP
*           INT8*service 端口或服务名
*           IOT_NET_PROTOCOL_TYPE type 协议类型
* 输出参数:
* 返 回  值:  0：成功，非零：失败
* 说       明: 使用系统接口创建socket，获取域名对应主机IP，
*           并且建立TCP连接
*           源码以及mbedtls中使用
************************************************************/
INT32 aliyun_iot_network_create(const INT8*host,const INT8*service,IOT_NET_PROTOCOL_TYPE type);

/***********************************************************
* 函数名称: aliyun_iot_network_bind
* 描       述: 服务端网络资源创建、连接
* 输入参数: INT8*host 域名或主机IP
*           INT8*service 端口或服务名
*           IOT_NET_PROTOCOL_TYPE type 协议类型
* 输出参数:
* 返 回  值:  0：成功，非零：失败
* 说       明: 使用系统接口创建socket，获取域名对应主机IP，
*           并且建立TCP连接
*           mbedtls中使用
************************************************************/
INT32 aliyun_iot_network_bind(const INT8*host,const INT8*service,IOT_NET_PROTOCOL_TYPE type);

/***********************************************************
* 函数名称: aliyun_iot_network_send
* 描       述: 发送网络数据接口
* 输入参数: INT32 sockFd socket描述符
*           void *buf 数据缓存
*           INT32 nbytes 缓存区大小
*           IOT_NET_TRANS_FLAGS_E flags 操作标志
* 输出参数:
* 返 回  值: 同linux系统下标准socket返回值
* 说       明: 网络数据发送，linux系统下的实现
*           源码和mbedtls中使用
************************************************************/
INT32 aliyun_iot_network_send(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags);

/***********************************************************
* 函数名称: aliyun_iot_network_recv
* 描       述: 接收网络数据接口
* 输入参数: INT32 sockFd socket描述符
*           void *buf 数据缓存
*           INT32 nbytes 缓存区大小
*           IOT_NET_TRANS_FLAGS_E flags 操作标志
* 输出参数:
* 返 回  值: 同linux系统下标准socket返回值
* 说       明: 网络数据发送，linux系统下的实现
*           源码和mbedtls中使用
************************************************************/
INT32 aliyun_iot_network_recv(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags);

/***********************************************************
* 函数名称: aliyun_iot_network_select
* 描       述: 有可读可写事件判断接口
* 输入参数: INT32 fd socket描述符
*           IOT_NET_TRANS_TYPE_E type 判断读写事件类型
*           int timeoutMs 超时时间
* 输出参数: IOT_NET_FD_ISSET_E* result 可读可写事件结果
* 返 回  值: 同linux系统下标准select的返回值
* 说       明: 判断socket是否有可读写事件，使用linux系统下select接口进行实现
*           源码和mbedtls中使用
************************************************************/
INT32 aliyun_iot_network_select(INT32 fd,IOT_NET_TRANS_TYPE_E type,int timeoutMs,IOT_NET_FD_ISSET_E* result);

/***********************************************************
* 函数名称: aliyun_iot_network_settimeout
* 描       述: 设置socketopt的超时参数
* 输入参数: INT32 fd socket描述符
*           int timeoutMs 超时时间
*           IOT_NET_TRANS_TYPE_E type 读写类型
* 输出参数:
* 返 回  值: 0：成功 非零：失败
* 说       明: linux系统下通过setsocketopt接口设置收发数据的超时时间
*           源码和mbedtls中使用
************************************************************/
INT32 aliyun_iot_network_settimeout(INT32 fd,int timeoutMs,IOT_NET_TRANS_TYPE_E type);

/***********************************************************
* 函数名称: aliyun_iot_network_get_nonblock
* 描       述: 获取socket非阻塞状态
* 输入参数: INT32 fd socket描述符
* 输出参数:
* 返 回  值: 0：阻塞  非零：非阻塞
* 说       明: linux系统下通过fcntl接口获取当前socket是否是非阻塞接口
*           mbedtls会使用此接口
*           mbedtls中使用
************************************************************/
INT32 aliyun_iot_network_get_nonblock(INT32 fd);

/***********************************************************
* 函数名称: aliyun_iot_network_set_nonblock
* 描       述: 设置socket非阻塞状态
* 输入参数: INT32 fd socket描述符
* 输出参数:
* 返 回  值: 0：成功  非零：失败
* 说       明: linux系统下通过fcntl接口设置当前socket为非阻塞接口
*           mbedtls中使用
************************************************************/
INT32 aliyun_iot_network_set_nonblock(INT32 fd);

/***********************************************************
* 函数名称: aliyun_iot_network_set_block
* 描       述: 设置socket阻塞状态
* 输入参数: INT32 fd socket描述符
* 输出参数:
* 返 回  值: 0：成功  非零：失败
* 说       明: linux系统下通过fcntl接口设置当前socket为阻塞接口
*           mbedtls中使用
************************************************************/
INT32 aliyun_iot_network_set_block(INT32 fd);

/***********************************************************
* 函数名称: aliyun_iot_network_close
* 描       述: 网络socket关闭
* 输入参数: INT32 fd socket描述符
* 输出参数:
* 返 回  值: 0：成功  非零：失败
* 说       明: linux系统socket关闭
*           mbedtls中使用
************************************************************/
INT32 aliyun_iot_network_close(INT32 fd);

/***********************************************************
* 函数名称: aliyun_iot_network_shutdown
* 描       述: 网络socket的shutdown
* 输入参数: INT32 fd socket描述符
* 输出参数:
* 返 回  值: 0：成功  非零：失败
* 说       明: linux系统socket的shutdown
*           mbedtls中使用
************************************************************/
INT32 aliyun_iot_network_shutdown(INT32 fd,INT32 how);

/***********************************************************
* 函数名称: aliyun_iot_get_errno
* 描       述: 获取SDK定义的errno
* 输入参数:
* 输出参数:
* 返 回  值: 自定义errno
* 说       明: 将linux系统下的errno转换为sdk自定义的errno
************************************************************/
INT32 aliyun_iot_get_errno(void);

#endif



#ifndef ALIYUN_IOT_PLATFORM_NETWORK_H
#define ALIYUN_IOT_PLATFORM_NETWORK_H

#include "qcom/base.h"
#include "qcom/select_api.h"
//#include "qcom/basetypes.h"
//#include "qcom/timetype.h"
#include "qcom/stdint.h"
#include "qcom/socket.h"
#include "qcom/socket_api.h"
#include "qcom/qcom_network.h"

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_platform_timer.h"
#include "aliyun_iot_platform_memory.h"

typedef enum IOT_NET_PROTOCOL_TYPE
{
    IOT_NET_PROTOCOL_TCP = 0,
    IOT_NET_PROTOCOL_UDP = 1,
}IOT_NET_PROTOCOL_TYPE;

typedef enum IOT_NET_TRANS_TYPE
{
    IOT_NET_TRANS_RECV = 0,
    IOT_NET_TRANS_SEND = 1,
}IOT_NET_TRANS_TYPE_E;

typedef enum IOT_NET_FD_ISSET
{
    IOT_NET_FD_NO_ISSET = 0,
    IOT_NET_FD_ISSET = 1,
}IOT_NET_FD_ISSET_E;

typedef enum IOT_NET_TRANS_FLAGS
{
    IOT_NET_FLAGS_DEFAULT = 0,
    IOT_NET_FLAGS_DONTWAIT = 1,
}IOT_NET_TRANS_FLAGS_E;

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

INT32 aliyun_iot_network_create(const INT8*host,const INT8*service,IOT_NET_PROTOCOL_TYPE type);

INT32 aliyun_iot_network_bind(const INT8*host,const INT8*service,IOT_NET_PROTOCOL_TYPE type);

INT32 aliyun_iot_network_send(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags);

INT32 aliyun_iot_network_recv(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags);

INT32 aliyun_iot_network_select(INT32 fd,IOT_NET_TRANS_TYPE_E type,int timeoutMs,IOT_NET_FD_ISSET_E* result);

INT32 aliyun_iot_network_settimeout(INT32 fd,int timeoutMs,IOT_NET_TRANS_TYPE_E type);

INT32 aliyun_iot_network_get_nonblock(INT32 fd);

INT32 aliyun_iot_network_set_nonblock(INT32 fd);

INT32 aliyun_iot_network_set_block(INT32 fd);

INT32 aliyun_iot_network_close(INT32 fd);

INT32 aliyun_iot_network_shutdown(INT32 fd,INT32 how);

INT32 aliyun_iot_get_errno(void);

#endif



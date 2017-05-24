#include "aliyun_iot_platform_network.h"

#include <errno.h>
#include <string.h>
#include "aliyun_iot_common_log.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#include "aliyun_iot_platform_memory.h"
#pragma comment(lib,"Ws2_32.lib")

#define CANONNAME_MAX 128

typedef struct NETWORK_ERRNO_TRANS
{
    int32_t systemData;
    ALIYUN_NETWORK_ERROR_E netwokErrno;
    int32_t privateData;
}NETWORK_ERRNO_TRANS_S;

static NETWORK_ERRNO_TRANS_S g_networkErrnoTrans[]=
{
	{ WSAEINTR, NETWORK_SIGNAL_INTERRUPT, EINTR_IOT },
	{ WSAEBADF, NETWORK_BAD_FILEFD, EBADF_IOT },
	{ WSAEFAULT, NETWORK_BADADDRESS, EFAULT_IOT },
	{ WSAEINVAL, NETWORK_INVALID_ARGUMENT, EINVAL_IOT },
	{ WSAEMFILE, NETWORK_TOO_MANY_OPEN_FILES, EMFILE_IOT },
	{ WSAEWOULDBLOCK, NETWORK_OPERATION_BLOCK, EWOULDBLOCK_IOT },
	{ WSAENOTSOCK, NETWORK_OPERATION_ON_NONSOCKET, ENOTSOCK_IOT },
	{ WSAENOPROTOOPT, NETWORK_PROTOCOL_NOT_AVAILABLE, ENOPROTOOPT_IOT },
	{ WSAEADDRINUSE, NETWORK_ADDRESS_ALREADY_IN_USE, EADDRINUSE_IOT },
	{ WSAEADDRNOTAVAIL, NETWORK_CANNOT_ASSIGN_REQUESTED_ADDRESS, EADDRNOTAVAIL_IOT },
	{ WSAENETDOWN, NETWORK_NETWORK_IS_DOWN, ENETDOWN_IOT },
	{ WSAENETUNREACH, NETWORK_NETWORK_IS_UNREACHABLE, ENETUNREACH_IOT },
	{ WSAENETRESET, NETWORK_CONNECT_RESET, ENETRESET_IOT },
	{ WSAECONNRESET, NETWORK_CONNECT_RESET_BY_PEER, ECONNRESET_IOT },
	{ WSAENOBUFS, NETWORK_NO_BUFFER_SPACE, ENOBUFS_IOT },
	{ WSAEISCONN, NETWORK_ALREADY_CONNECTED, EISCONN_IOT },
	{ WSAENOTCONN, NETWORK_IS_NOT_CONNECTED, ENOTCONN_IOT },
	{ WSAETIMEDOUT, NETWORK_CONNECTION_TIMED_OUT, ETIMEDOUT_IOT },
	{ WSAECONNREFUSED, NETWORK_CONNECTION_REFUSED, ECONNREFUSED_IOT },
	{ WSAEHOSTUNREACH, NETWORK_NO_ROUTE_TO_HOST, EHOSTUNREACH_IOT },
	{ WSAEMSGSIZE, NETWORK_MSG_TOO_LONG, EMSGSIZE_IOT }
};

int32_t errno_transform(int32_t systemErrno,ALIYUN_NETWORK_ERROR_E *netwokErrno,int32_t *privateErrno)
{
    int32_t num = sizeof(g_networkErrnoTrans);
    int32_t i = 0;
    for(i = 0;i<num;i++)
    {
        if(g_networkErrnoTrans[i].systemData == systemErrno)
        {
            *netwokErrno = g_networkErrnoTrans[i].netwokErrno;
            *privateErrno = g_networkErrnoTrans[i].privateData;
            return NETWORK_SUCCESS;
        }
    }

    return NETWORK_FAIL;
}

int32_t aliyun_iot_get_errno(void)
{
    ALIYUN_NETWORK_ERROR_E networkErrno = NETWORK_FAIL;
    int32_t private = 0;
	int32_t result = errno_transform(GetLastError(), &networkErrno, &private);
    if(0 != result)
    {
        ALIOT_LOG_ERROR("network errno = %d",errno);
        return NETWORK_FAIL;
    }

    return private;
}

int32_t aliyun_iot_network_send(int32_t sockFd, void *buf, int32_t nbytes, IOT_NET_TRANS_FLAGS_E flags)
{
    uint32_t flag = 0;

    if( sockFd < 0 )
    {
        return NETWORK_FAIL;
    }

    if(IOT_NET_FLAGS_DEFAULT == flags)
    {
        flag = 0;
    }

    return send(sockFd,buf,nbytes,flag);
}

int32_t aliyun_iot_network_recv(int32_t sockFd, void *buf, int32_t nbytes, IOT_NET_TRANS_FLAGS_E flags)
{
    uint32_t flag = 0;
	int32_t ret = 0;
	unsigned long ul = 1;

    if( sockFd < 0 )
    {
        return NETWORK_FAIL;
    }

	if (IOT_NET_FLAGS_DONTWAIT == flags)
	{
		ul = 1;
		ret = ioctlsocket(sockFd, FIONBIO, (unsigned long *)&ul);
	}
	else
	{
		ul = 0;
		ret = ioctlsocket(sockFd, FIONBIO, (unsigned long *)&ul);
	}

	return recv(sockFd, buf, nbytes, flag);
}

int32_t aliyun_iot_network_select(int32_t fd, IOT_NET_TRANS_TYPE_E type, int timeoutMs, IOT_NET_FD_ISSET_E *result)
{
    struct timeval timeout, *ptimeout;
    fd_set *rd_set = NULL;
    fd_set *wr_set = NULL;
    fd_set *ep_set = NULL;
    int rc = 0;
    fd_set sets;

    *result = IOT_NET_FD_NO_ISSET;

    if( fd < 0 ) {
        return NETWORK_FAIL;
    }

    FD_ZERO(&sets);
    FD_SET(fd, &sets);

    if(IOT_NET_TRANS_RECV == type) {
        rd_set = &sets;
    } else {
        wr_set = &sets;
    }

    if (0 == timeoutMs) {
        ptimeout = NULL;
    } else {
        ptimeout = &timeout;
        ptimeout->tv_sec = timeoutMs / 1000;
        ptimeout->tv_usec = (timeoutMs % 1000) * 1000;
    }

    ALIOT_LOG_DEBUG("time_s=%u, time_us=%u.", ptimeout->tv_sec, ptimeout->tv_usec);

    rc = select(fd + 1, rd_set, wr_set, ep_set, ptimeout);
    if(rc > 0) {
        if (0 != FD_ISSET(fd, &sets)) {
            *result = IOT_NET_FD_ISSET;
        }
    }

    ALIOT_LOG_DEBUG("select return");

    return rc;
}

int32_t aliyun_iot_network_settimeout(int32_t fd,int timeoutMs,IOT_NET_TRANS_TYPE_E type)
{
    struct timeval timeout = {timeoutMs/1000, (timeoutMs%1000)*1000};

    int optname = type == IOT_NET_TRANS_RECV ? SO_RCVTIMEO:SO_SNDTIMEO;

    if( fd < 0 )
    {
        return NETWORK_FAIL;
    }

    if(0 != setsockopt(fd, SOL_SOCKET, optname, (char *)&timeout, sizeof(timeout)))
    {
        ALIOT_LOG_ERROR("setsockopt error, errno = %d",errno);
        return ERROR_NET_SETOPT_TIMEOUT;
    }

    return SUCCESS_RETURN;
}

int32_t aliyun_iot_network_get_nonblock(int32_t fd)
{
    if( fd < 0 )
    {
        return NETWORK_FAIL;
    }

    return 0 ;
}

int32_t aliyun_iot_network_set_nonblock(int32_t fd)
{
	int rc = 0;
	u_long mode = 1;

	if( fd < 0 )
    {
        return NETWORK_FAIL;
    }

	rc = ioctlsocket(fd, FIONBIO, &mode);
	if (rc != NO_ERROR)
	{
		return NETWORK_FAIL;
	}

	return NETWORK_SUCCESS;
}

int32_t aliyun_iot_network_set_block(int32_t fd)
{
	int rc = 0;
	u_long mode = 0;

    if( fd < 0 )
    {
        return NETWORK_FAIL;
    }

	rc = ioctlsocket(fd, FIONBIO, &mode);
	if (rc != NO_ERROR)
    {
		return NETWORK_FAIL;
	}

	return NETWORK_SUCCESS;
}

int32_t aliyun_iot_network_close(int32_t fd)
{
    closesocket(fd);
    WSACleanup();
    return 0;
}

int32_t aliyun_iot_network_shutdown(int32_t fd,int32_t how)
{
    return shutdown(fd,how);
}

int32_t aliyun_iot_network_create(const int8_t*host,const int8_t*service,IOT_NET_PROTOCOL_TYPE type)
{
	WSADATA wsaData;
    struct addrinfo hints;
    struct addrinfo *addrInfoList = NULL;
    struct addrinfo *cur = NULL;
    int fd = 0;
    int rc = ERROR_NET_UNKNOWN_HOST;

	// Initialize Winsock
	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc != 0)
	{
		ALIOT_LOG_ERROR("WSAStartup failed: %d", rc);
		return FAIL_RETURN;
	}

    memset( &hints, 0, sizeof(hints));

    //默认支持IPv4的服务
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if ((rc = getaddrinfo( host, service, &hints, &addrInfoList ))!= 0 )
    {
        ALIOT_LOG_ERROR("getaddrinfo error! rc = %d, errno = %d",rc,errno);
        return ERROR_NET_UNKNOWN_HOST;
    }

    for( cur = addrInfoList; cur != NULL; cur = cur->ai_next )
    {
        //默认只支持IPv4
        if (cur->ai_family != AF_INET)
        {
            ALIOT_LOG_ERROR("socket type error");
            rc = ERROR_NET_SOCKET;
            continue;
        }

        fd = (int) socket( cur->ai_family, cur->ai_socktype,cur->ai_protocol );
        if( fd < 0 )
        {
            ALIOT_LOG_ERROR("create socket error,fd = %d, errno = %d",fd,errno);
            rc = ERROR_NET_SOCKET;
            continue;
        }

        if( connect( fd,cur->ai_addr,cur->ai_addrlen ) == 0 )
        {
            rc = fd;
            break;
        }

        closesocket( fd );
        ALIOT_LOG_ERROR("connect error,errno = %d",errno);
        rc = ERROR_NET_CONNECT;
    }

    freeaddrinfo(addrInfoList);

    return rc;
}

int32_t aliyun_iot_network_bind(const int8_t*host,const int8_t*service,IOT_NET_PROTOCOL_TYPE type)
{
    int fd = 0;
    int n = 0;
    int ret = FAIL_RETURN;
    struct addrinfo hints, *addrList, *cur;

    /* Bind to IPv6 and/or IPv4, but only in the desired protocol */
    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = type == IOT_NET_PROTOCOL_UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = type == IOT_NET_PROTOCOL_UDP ? IPPROTO_UDP : IPPROTO_TCP;
    if( host == NULL )
    {
        hints.ai_flags = AI_PASSIVE;
    }

    if( getaddrinfo( host, service, &hints, &addrList ) != 0 )
    {
        return( ERROR_NET_UNKNOWN_HOST );
    }

    for( cur = addrList; cur != NULL; cur = cur->ai_next )
    {
        fd = (int) socket( cur->ai_family, cur->ai_socktype,cur->ai_protocol );
        if( fd < 0 )
        {
            ret = ERROR_NET_SOCKET;
            continue;
        }

        n = 1;
        if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,(const char *) &n, sizeof( n ) ) != 0 )
        {
            closesocket(fd);
            ret = ERROR_NET_SOCKET;
            continue;
        }

        if( bind(fd, cur->ai_addr, cur->ai_addrlen ) != 0 )
        {
            closesocket( fd );
            ret = ERROR_NET_BIND;
            continue;
        }

        /* Listen only makes sense for TCP */
        if(type == IOT_NET_PROTOCOL_TCP)
        {
            if( listen( fd, 10 ) != 0 )
            {
                closesocket( fd );
                ret = ERROR_NET_LISTEN;
                continue;
            }
        }

        /* I we ever get there, it's a success */
        ret = fd;
        break;
    }

    freeaddrinfo( addrList );

    return( ret );
}


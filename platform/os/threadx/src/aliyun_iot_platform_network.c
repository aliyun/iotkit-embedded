//#include <unistd.h>
//#include <fcntl.h>
//#include <string.h>
//#include <errno.h>
#include "aliyun_iot_platform_network.h"


#define CANONNAME_MAX 128
static int errno = 0;

typedef struct NETWORK_ERRNO_TRANS
{
    INT32 systemData;
    ALIYUN_NETWORK_ERROR_E netwokErrno;
    INT32 privateData;
}NETWORK_ERRNO_TRANS_S;

static NETWORK_ERRNO_TRANS_S g_networkErrnoTrans[]=
{
/*
    {EINTR,NETWORK_SIGNAL_INTERRUPT,EINTR_IOT},
    {EBADF,NETWORK_BAD_FILEFD,EBADF_IOT},
    {EAGAIN,NETWORK_TRYAGAIN,EAGAIN_IOT},
    {EFAULT,NETWORK_BADADDRESS,EFAULT_IOT},
    {EBUSY,NETWORK_RESOURCE_BUSY, EBUSY_IOT},
    {EINVAL,NETWORK_INVALID_ARGUMENT, EINVAL_IOT},
    {ENFILE,NETWORK_FILE_TABLE_OVERFLOW, ENFILE_IOT},
    {EMFILE,NETWORK_TOO_MANY_OPEN_FILES, EMFILE_IOT},
    {ENOSPC,NETWORK_NO_SPACE_LEFT_ON_DEVICE, ENOSPC_IOT},
    {EPIPE,NETWORK_BROKEN_PIPE, EPIPE_IOT},
    {EWOULDBLOCK,NETWORK_OPERATION_BLOCK, EWOULDBLOCK_IOT},
    {ENOTSOCK,NETWORK_OPERATION_ON_NONSOCKET, ENOTSOCK_IOT},
    {ENOPROTOOPT,NETWORK_PROTOCOL_NOT_AVAILABLE, ENOPROTOOPT_IOT},
    {EADDRINUSE,NETWORK_ADDRESS_ALREADY_IN_USE, EADDRINUSE_IOT},
    {EADDRNOTAVAIL,NETWORK_CANNOT_ASSIGN_REQUESTED_ADDRESS, EADDRNOTAVAIL_IOT},
    {ENETDOWN,NETWORK_NETWORK_IS_DOWN, ENETDOWN_IOT},
    {ENETUNREACH,NETWORK_NETWORK_IS_UNREACHABLE, ENETUNREACH_IOT},
    {ENETRESET,NETWORK_CONNECT_RESET, ENETRESET_IOT},
    {ECONNRESET,NETWORK_CONNECT_RESET_BY_PEER, ECONNRESET_IOT},
    {ENOBUFS,NETWORK_NO_BUFFER_SPACE, ENOBUFS_IOT},
    {EISCONN,NETWORK_ALREADY_CONNECTED, EISCONN_IOT},
    {ENOTCONN,NETWORK_IS_NOT_CONNECTED, ENOTCONN_IOT},
    {ETIMEDOUT,NETWORK_CONNECTION_TIMED_OUT, ETIMEDOUT_IOT},
    {ECONNREFUSED,NETWORK_CONNECTION_REFUSED, ECONNREFUSED_IOT},
    {EHOSTDOWN,NETWORK_HOST_IS_DOWN, EHOSTDOWN_IOT},
    {EHOSTUNREACH,NETWORK_NO_ROUTE_TO_HOST, EHOSTUNREACH_IOT},
    {ENOMEM ,NETWORK_OUT_OF_MEMORY, ENOMEM_IOT},
    {EMSGSIZE,NETWORK_MSG_TOO_LONG, EMSGSIZE_IOT}
    */
};

INT32 errno_transform(INT32 systemErrno, ALIYUN_NETWORK_ERROR_E *netwokErrno, INT32 *privateErrno)
{
    INT32 num = sizeof(g_networkErrnoTrans);
    INT32 i = 0;
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

INT32 aliyun_iot_get_errno(void)
{
/*
    ALIYUN_NETWORK_ERROR_E networkErrno = NETWORK_FAIL;
    INT32 private = 0;
    INT32 result = errno_transform(errno, &networkErrno, &private);
    if(0 != result)
    {
        WRITE_IOT_ERROR_LOG("network errno = %d",errno);
        return NETWORK_FAIL;
    }

    return private;
    */
    return EAGAIN_IOT;
}

INT32 aliyun_iot_network_send(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags)
{
    UINT32 flag = 0;

    if(IOT_NET_FLAGS_DEFAULT == flags)
    {
        flag = 0;
    }

    return qcom_send(sockFd, buf, nbytes, flag);
}

INT32 aliyun_iot_network_recv(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags)
{
    UINT32 flag = 0;

    if(IOT_NET_FLAGS_DEFAULT == flags)
    {
        flag = 0;
		//qcom_socket_set_non_blocking(sockFd,0);
    }
    else
    {
        flag = 0;
		//qcom_socket_set_non_blocking(sockFd,1);
    }

    return qcom_recv(sockFd, buf, nbytes, flag);
}

INT32 aliyun_iot_network_select(INT32 fd, IOT_NET_TRANS_TYPE_E type, int timeoutMs, IOT_NET_FD_ISSET_E *result)
{
    struct timeval *timePointer = NULL;
    q_fd_set *rd_set = NULL;
    q_fd_set *wr_set = NULL;
    q_fd_set *ep_set = NULL;
	
    int rc = 0;

    q_fd_set sets;
    FD_ZERO(&sets);
     _QCA_FD_SET(fd, &sets);
	
    *result = IOT_NET_FD_NO_ISSET;

    if(IOT_NET_TRANS_RECV == type)
    {
        rd_set = &sets;
    }
    else
    {
        wr_set = &sets;
    }

    struct timeval timeout = {timeoutMs/1000, (timeoutMs%1000)*1000};
    if(0 != timeoutMs)
    {
        timePointer = &timeout;
    }
    else
    {
        timePointer = NULL;
    }

    rc = qcom_select(fd+1, rd_set, wr_set, ep_set, timePointer);
    if(rc > 0)
    {
        if (0 != _QCA_FD_ISSET(fd, &sets))
        {
            *result = IOT_NET_FD_ISSET;
        }
    }
	else
	{
		WRITE_IOT_ERROR_LOG("qcom_select rc = %d, timeout = %d:%d", rc, timePointer->tv_sec, timePointer->tv_usec);
	}

    return rc;
}

INT32 aliyun_iot_network_settimeout(INT32 fd,int timeoutMs,IOT_NET_TRANS_TYPE_E type)
{
	int ret = 0;
    struct timeval timeout = {timeoutMs/1000, (timeoutMs%1000)*1000};

    int optname = ((type == IOT_NET_TRANS_RECV) ? SO_RCVTIMEO : SO_SNDTIMEO);

	ret = qcom_setsockopt(fd, SOL_SOCKET, optname, &timeout, sizeof(timeout));
    if(0 != ret)
    {
        WRITE_IOT_ERROR_LOG("setsockopt error, errno = %d ret = %d fd = %d",errno, ret, fd);
        return ERROR_NET_SETOPT_TIMEOUT;
    }

    return SUCCESS_RETURN;
}

INT32 aliyun_iot_network_get_nonblock(INT32 fd)
{
/*
    if( ( fcntl( fd, F_GETFL, 0 ) & O_NONBLOCK ) != O_NONBLOCK )
    {
        return 0;
    }

    if(errno == EAGAIN || errno == EWOULDBLOCK)
    {
        return 1;
    }
*/
    return 0 ;
}

INT32 aliyun_iot_network_set_nonblock(INT32 fd)
{
	qcom_socket_set_non_blocking(fd, 1);
    return NETWORK_SUCCESS;
}

INT32 aliyun_iot_network_set_block(INT32 fd)
{
	qcom_socket_set_non_blocking(fd, 0);
    return NETWORK_SUCCESS;
}

INT32 aliyun_iot_network_close(INT32 fd)
{
    return qcom_socket_close(fd);
}

INT32 aliyun_iot_network_shutdown(INT32 fd, INT32 how)
{
    return qcom_socket_close(fd);
}

INT32 aliyun_iot_network_create(const INT8 *host, const INT8 *service, IOT_NET_PROTOCOL_TYPE type)
{
	int family = 0;
	int socktype = 0;
	int protocol = 0;
	int fd = 0;
	int rc = 0;

	struct sockaddr_in address;
    memset(&address, 0x0, sizeof(struct sockaddr_in));
	UINT32 ipAddress = 0;

	if(IOT_NET_PROTOCOL_TCP == type)
	{
		family = AF_INET;
		socktype = SOCK_STREAM;
		protocol = 0;
	}
	else
	{
		family = PF_INET;
		socktype = SOCK_DGRAM;
		protocol = 0;
	}

	qcom_dnsc_enable(1);
	if (qcom_dnsc_get_host_by_name((INT8 *)host, &ipAddress) != 0)
	{
		WRITE_IOT_ERROR_LOG("DNS resolve error");
		return NETWORK_FAIL;
	}
	
	address.sin_addr.s_addr = htonl(ipAddress);
	address.sin_port = htons(atoi(service));
	address.sin_family = AF_INET;

	fd = qcom_socket( family, socktype, protocol );
	if(fd < 0)
	{
		WRITE_IOT_ERROR_LOG("create socket error,fd = %d, errno = %d", fd, errno);
		rc = ERROR_NET_SOCKET;
		return rc;
	}

	rc =  qcom_connect( fd, (struct sockaddr *) &address, sizeof (struct sockaddr_in));
	if(rc < 0)
	{
		WRITE_IOT_ERROR_LOG(" socket connect error,fd = %d, errno = %d rc = %d", fd, errno, rc);
		qcom_socket_close( fd );
		rc = ERROR_NET_CONNECT;
		return rc;
	}

	rc = fd;
	return rc;
}


INT32 aliyun_iot_network_bind(const INT8 *host, const INT8 *service, IOT_NET_PROTOCOL_TYPE type)
{
	int fd = 0;
	int optval = 0;
	int rc = 0;

	int family = 0;
	int socktype = 0;
	int protocol = 0;

	struct sockaddr_in address;
	memset(&address,0x0,sizeof(struct sockaddr_in));
	UINT32 ipAddress = 0;
	
	family = AF_INET;
	socktype = ((type == IOT_NET_PROTOCOL_UDP) ? SOCK_DGRAM : SOCK_STREAM);
	protocol = 0;


	if (qcom_dnsc_get_host_by_name((INT8 *)host, &ipAddress) != 0)
	{
		WRITE_IOT_ERROR_LOG("DNS resolve error");
		return NETWORK_FAIL;
	}

	address.sin_addr.s_addr = ipAddress;
	address.sin_port = htons(atoi(service));
	address.sin_family = AF_INET;

	fd = qcom_socket( family, socktype, protocol );
	if( fd < 0 )
	{
		WRITE_IOT_ERROR_LOG("create socket error,fd = %d, errno = %d", fd, errno);
		rc = ERROR_NET_SOCKET;
		return rc;
	}

	optval = 1;
	if( qcom_setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof( optval ) ) != 0 )
	{
		WRITE_IOT_ERROR_LOG("setsockopt error, fd = %d, errno = %d", fd, errno);
		qcom_socket_close(fd);
		rc = ERROR_NET_SOCKET;
		return rc;
	}

	if( qcom_bind(fd, (struct sockaddr *)&address, sizeof(struct sockaddr_in) ) != 0 )
	{
		WRITE_IOT_ERROR_LOG("bind socket error, fd = %d, errno = %d", fd, errno);
		qcom_socket_close( fd );
		rc = ERROR_NET_BIND;
		return rc;
	}

	/* Listen only makes sense for TCP */
	if(type == IOT_NET_PROTOCOL_TCP)
	{
		if( qcom_listen( fd, 10 ) != 0 )
		{			
			WRITE_IOT_ERROR_LOG("listen socket error, fd = %d, errno = %d", fd, errno);
			qcom_socket_close( fd );
			rc = ERROR_NET_LISTEN;
			return rc;
		}
	}
	
	return rc;
}



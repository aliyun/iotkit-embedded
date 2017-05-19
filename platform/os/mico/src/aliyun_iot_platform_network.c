#include <string.h>
#include "aliyun_iot_platform_network.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_platform_memory.h"

#define CANONNAME_MAX 128
extern int errno;

typedef struct NETWORK_ERRNO_TRANS
{
    INT32 systemData;
    ALIYUN_NETWORK_ERROR_E netwokErrno;
    INT32 privateData;
}NETWORK_ERRNO_TRANS_S;

static NETWORK_ERRNO_TRANS_S g_networkErrnoTrans[]=
{
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
};

INT32 errno_transform(INT32 systemErrno,ALIYUN_NETWORK_ERROR_E *netwokErrno,INT32 *privateErrno)
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
    ALIYUN_NETWORK_ERROR_E networkErrno = NETWORK_FAIL;
    INT32 private = 0;
    INT32 result = errno_transform(errno,&networkErrno,&private);
    if(0 != result)
    {
        WRITE_IOT_ERROR_LOG("network errno = %d",errno);
        return NETWORK_FAIL;
    }

    return private;
}

INT32 aliyun_iot_network_send(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags)
{
    UINT32 flag = 0;

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

INT32 aliyun_iot_network_recv(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags)
{
    UINT32 flag = 0;

    if( sockFd < 0 )
    {
        return NETWORK_FAIL;
    }

    if(IOT_NET_FLAGS_DEFAULT == flags)
    {
        flag = 0;
    }
    else
    {
   	#ifndef MICO_SDK_V3
		struct timeval_t timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval_t));
		flag = 0;

	#else
        struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
        flag = 0;
	#endif
    }

    return recv(sockFd,buf,nbytes,flag);
}

INT32 aliyun_iot_network_select(INT32 fd,IOT_NET_TRANS_TYPE_E type,int timeoutMs,IOT_NET_FD_ISSET_E* result)
{
#ifndef MICO_SDK_V3
	struct timeval_t *timePointer = NULL;
#else
    struct timeval *timePointer = NULL;
#endif

    fd_set *rd_set = NULL;
    fd_set *wr_set = NULL;
    fd_set *ep_set = NULL;
    int rc = 0;
    fd_set sets;

    *result = IOT_NET_FD_NO_ISSET;

    if( fd < 0 )
    {
        return NETWORK_FAIL;
    }

    FD_ZERO(&sets);
    FD_SET(fd, &sets);

    if(IOT_NET_TRANS_RECV == type)
    {
        rd_set = &sets;
    }
    else
    {
        wr_set = &sets;
    }

#ifndef MICO_SDK_V3
	struct timeval_t timeout = {timeoutMs/1000, (timeoutMs%1000)*1000};
#else
    struct timeval timeout = {timeoutMs/1000, (timeoutMs%1000)*1000};
#endif
    if(0 != timeoutMs)
    {
        timePointer = &timeout;
    }
    else
    {
        timePointer = NULL;
    }

    rc = select(fd+1,rd_set,wr_set,ep_set,timePointer);
    if(rc > 0)
    {
	    if( fd < 0 )
        {
            return NETWORK_FAIL;
        }
		
        if (0 != FD_ISSET(fd, &sets))
        {
            *result = IOT_NET_FD_ISSET;
        }
    }

    return rc;
}

INT32 aliyun_iot_network_settimeout(INT32 fd,int timeoutMs,IOT_NET_TRANS_TYPE_E type)
{
#ifndef MICO_SDK_V3
	struct timeval_t timeout = {timeoutMs/1000, (timeoutMs%1000)*1000};
#else
    struct timeval timeout = {timeoutMs/1000, (timeoutMs%1000)*1000};
#endif
    int optname = ((type == IOT_NET_TRANS_RECV) ? SO_RCVTIMEO:SO_SNDTIMEO);

    if( fd < 0 )
    {
        return NETWORK_FAIL;
    }

    if(0 != setsockopt(fd, SOL_SOCKET, optname, &timeout, sizeof(timeout)))
    {
        WRITE_IOT_ERROR_LOG("setsockopt error, errno = %d",errno);
        return ERROR_NET_SETOPT_TIMEOUT;
    }

    return SUCCESS_RETURN;
}

INT32 aliyun_iot_network_get_nonblock(INT32 fd)
{
    if( fd < 0 )
    {
        return NETWORK_FAIL;
    }

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
    if( fd < 0 )
    {
        return NETWORK_FAIL;
    }

/*
    INT32 flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, (flags | O_NONBLOCK)) < 0)
    {
        return NETWORK_FAIL;
    }
*/
    return NETWORK_SUCCESS;
}

INT32 aliyun_iot_network_set_block(INT32 fd)
{
    if( fd < 0 )
    {
        return NETWORK_FAIL;
    }

/*
    INT32 flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, (flags & (~O_NONBLOCK))) < 0)
    {
        return NETWORK_FAIL;
    }
*/
    return NETWORK_SUCCESS;
}

INT32 aliyun_iot_network_close(INT32 fd)
{
    return close(fd);
}

INT32 aliyun_iot_network_shutdown(INT32 fd,INT32 how)
{
    return close(fd);
}

INT32 aliyun_iot_network_create(const INT8*host,const INT8*service,IOT_NET_PROTOCOL_TYPE type)
{
	char ipstr[16] = {0};
	int family = 0;
	int socktype = 0;
	int protocol = 0;
    int fd = 0;
    int rc = 0;
#ifndef MICO_SDK_V3
	struct sockaddr_t address;
#else
	struct sockaddr_in address;
	struct hostent* hostent_content = NULL;
    struct in_addr in_addr;
	char name[20];
	char **pptr = NULL;
#endif
    if(IOT_NET_PROTOCOL_TCP == type)
    {
        family = AF_INET;
        socktype = SOCK_STREAM;
        protocol = IPPROTO_TCP;
    }
    else
    {
        family = AF_INET;
	#ifndef MICO_SDK_V3
		socktype = SOCK_DGRM;
	#else
        socktype = SOCK_DGRAM;
	#endif
        protocol = IPPROTO_UDP;
    }

#ifndef MICO_SDK_V3

	if(0 != gethostbyname(host, (uint8_t *)ipstr, 16))
	{
		WRITE_IOT_ERROR_LOG("getaddrinfo error! rc = %d, errno = %d", rc, errno);
		return ERROR_NET_UNKNOWN_HOST;
	}

	address.s_ip = inet_addr(ipstr);
	address.s_port = atoi(service);

#else
	hostent_content = gethostbyname( host );
	if(NULL == hostent_content)
	{
		WRITE_IOT_ERROR_LOG("getaddrinfo error! errno = %d", errno);
        return ERROR_NET_UNKNOWN_HOST;
	}
	
    pptr = hostent_content->h_addr_list;
    strcpy(name, hostent_content->h_name);
    WRITE_IOT_DEBUG_LOG("name is %s",name);
	
    in_addr.s_addr = *(UINT32 *)(*pptr);
    strcpy( ipstr, inet_ntoa(in_addr));
    WRITE_IOT_DEBUG_LOG("address: host:%s, ip: %s", host, ipstr);

	address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr( ipstr );
    address.sin_port = htons(atoi(service));
#endif

	fd = socket( family, socktype, protocol );
	if( fd < 0 )
	{
		WRITE_IOT_ERROR_LOG("create socket error,fd = %d, errno = %d",fd, errno);
		rc = ERROR_NET_SOCKET;
		return rc;
	}

#ifndef MICO_SDK_V3	
	if( connect( fd, (struct sockaddr_t *)&address, sizeof(address) ) != 0 )
#else
	if( connect( fd, (struct sockaddr *)&address, sizeof(address) ) != 0 )
#endif
	{
		WRITE_IOT_ERROR_LOG(" socket connect error,fd = %d, errno = %d",fd, errno);
		close( fd );
        rc = ERROR_NET_CONNECT;
		return rc;
	}

	rc = fd;
    return rc;
}

INT32 aliyun_iot_network_bind(const INT8*host,const INT8*service,IOT_NET_PROTOCOL_TYPE type)
{
    int fd = 0;
    int optval = 0;
    int rc = 0;

	int family = 0;
	int socktype = 0;
	int protocol = 0;

	char ipstr[16] = {0};
#ifndef MICO_SDK_V3
	struct sockaddr_t address;
#else
	struct sockaddr_in address;
	struct hostent* hostent_content = NULL;
    struct in_addr in_addr;
	char name[20];
	char **pptr = NULL;
#endif

    family = AF_INET;
#ifndef MICO_SDK_V3
	socktype = ((type == IOT_NET_PROTOCOL_UDP) ? SOCK_DGRM : SOCK_STREAM);
#else
    socktype = ((type == IOT_NET_PROTOCOL_UDP) ? SOCK_DGRAM : SOCK_STREAM);
#endif
    protocol = ((type == IOT_NET_PROTOCOL_UDP) ? IPPROTO_UDP : IPPROTO_TCP);

#ifndef MICO_SDK_V3

	if(0 != gethostbyname(host, (uint8_t *)ipstr, 16))
	{
		WRITE_IOT_ERROR_LOG("getaddrinfo error! rc = %d, errno = %d", rc, errno);
		return ERROR_NET_UNKNOWN_HOST;
	}

	address.s_ip = inet_addr(ipstr);
	address.s_port = atoi(service);

#else
	hostent_content = gethostbyname( host );
	if(NULL == hostent_content)
	{
		WRITE_IOT_ERROR_LOG("getaddrinfo error! errno = %d", errno);
        return ERROR_NET_UNKNOWN_HOST;
	}
	
    pptr = hostent_content->h_addr_list;
    strcpy(name, hostent_content->h_name);
    WRITE_IOT_DEBUG_LOG("name is %s",name);
	
    in_addr.s_addr = *(UINT32 *)(*pptr);
    strcpy( ipstr, inet_ntoa(in_addr));
    WRITE_IOT_DEBUG_LOG("address: host:%s, ip: %s", host, ipstr);

	address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr( ipstr );
    address.sin_port = htons(atoi(service));
#endif

	fd = socket( family, socktype, protocol );
	if( fd < 0 )
	{
		WRITE_IOT_ERROR_LOG("create socket error,fd = %d, errno = %d",fd, errno);
		rc = ERROR_NET_SOCKET;
		return rc;
	}

	optval = 1;
	if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof( optval ) ) != 0 )
	{
		WRITE_IOT_ERROR_LOG("setsockopt error, fd = %d, errno = %d", fd, errno);
		close(fd);
		rc = ERROR_NET_SOCKET;
		return rc;
	}

#ifndef MICO_SDK_V3
	if( bind(fd, (struct sockaddr_t *)&address, sizeof(address) ) != 0 )
#else
	if( bind(fd, (struct sockaddr *)&address, sizeof(address) ) != 0 )
#endif
	{
		WRITE_IOT_ERROR_LOG("bind socket error, fd = %d, errno = %d", fd, errno);
		close( fd );
		rc = ERROR_NET_BIND;
		return rc;
	}

	/* Listen only makes sense for TCP */
	if(type == IOT_NET_PROTOCOL_TCP)
	{
		if( listen( fd, 10 ) != 0 )
		{			
			WRITE_IOT_ERROR_LOG("listen socket error, fd = %d, errno = %d", fd, errno);
			close( fd );
			rc = ERROR_NET_LISTEN;
			return rc;
		}
	}
	
    return rc;
}


/*
 *  TCP/IP or UDP/IP networking functions
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_NET_C)

#include "mbedtls/net.h"

#include <string.h>
#include "aliyun_iot_platform_signal.h"



#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && \
    !defined(EFI32)

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
/* Enables getaddrinfo() & Co */
#define _WIN32_WINNT 0x0501
#include <ws2tcpip.h>

#include <winsock2.h>
#include <windows.h>

#if defined(_MSC_VER)
#if defined(_WIN32_WCE)
#pragma comment( lib, "ws2.lib" )
#else
#pragma comment( lib, "ws2_32.lib" )
#endif
#endif /* _MSC_VER */


//#define read(fd,buf,len)        recv(fd,(char*)buf,(int) len,0)
//#define write(fd,buf,len)       send(fd,(char*)buf,(int) len,0)
//#define close(fd)               closesocket(fd)

static int wsa_init_done = 0;

#else /* ( _WIN32 || _WIN32_WCE ) && !EFIX64 && !EFI32 */

#ifdef SUPPORT_MBEDTLS   //MT7687 mbedTLS enable
//#include <sockets.h>
//#include <sockets_mbedtls.h>
//#include <inet.h>
#else
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <sys/time.h>
#endif /* SUPPORT_MBEDTLS */


//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <sys/time.h>

//#include <unistd.h>
//#include <signal.h>
//#include <fcntl.h>
//#include <netdb.h>
//#include <errno.h>

#endif /* ( _WIN32 || _WIN32_WCE ) && !EFIX64 && !EFI32 */

/* Some MS functions want int and MSVC warns if we pass size_t,
 * but the standard fucntions use socklen_t, so cast only for MSVC */
#if defined(_MSC_VER)
#define MSVC_INT_CAST   (int)
#else
#define MSVC_INT_CAST
#endif

#include <stdlib.h>
#include <stdio.h>

//#include <time.h>

#include <stdint.h>
#include "aliyun_iot_platform_network.h"
#include "aliyun_iot_platform_signal.h"
#include "aliyun_iot_platform_timer.h"
#include "aliyun_iot_platform_pthread.h"

/*
 * Prepare for using the sockets interface
 */
static int net_prepare( void )
{
#if ( defined(_WIN32) || defined(_WIN32_WCE) ) && !defined(EFIX64) && \
    !defined(EFI32)
    WSADATA wsaData;

    if( wsa_init_done == 0 )
    {
        if( WSAStartup( MAKEWORD(2,0), &wsaData ) != 0 )
            return( MBEDTLS_ERR_NET_SOCKET_FAILED );

        wsa_init_done = 1;
    }
#else
#if !defined(EFIX64) && !defined(EFI32)
    aliyun_iot_send_SIGPIPE_signal();
#endif
#endif
    return( 0 );
}

/*
 * Initialize a context
 */
void mbedtls_net_init( mbedtls_net_context *ctx )
{
    ctx->fd = -1;
}

/*
 * Initiate a TCP connection with host:port and the given protocol
 */
int mbedtls_net_connect( mbedtls_net_context *ctx, const char *host, const char *port, int proto )
{
    int ret;
    IOT_NET_PROTOCOL_TYPE type;

    if( ( ret = net_prepare() ) != 0 )
    {
        return( ret );
    }

    type = (proto == MBEDTLS_NET_PROTO_UDP) ? IOT_NET_PROTOCOL_UDP : IOT_NET_PROTOCOL_TCP;

    ctx->fd = aliyun_iot_network_create(host,port,type);
    if(ctx->fd < 0)
    {
        return ctx->fd;
    }

    return 0;
}

/*
 * Create a listening socket on bind_ip:port
 */
int mbedtls_net_bind( mbedtls_net_context *ctx, const char *bind_ip, const char *port, int proto )
{
    int ret;
    IOT_NET_PROTOCOL_TYPE type;

    if( ( ret = net_prepare() ) != 0 )
    {
        return( ret );
    }

    type = proto == MBEDTLS_NET_PROTO_UDP ? IOT_NET_PROTOCOL_UDP : IOT_NET_PROTOCOL_TCP;

    ctx->fd = aliyun_iot_network_bind(bind_ip,port,type);
    if(ctx->fd < 0 )
    {
        return ctx->fd;
    }

    return 0;
}

#if ( defined(_WIN32) || defined(_WIN32_WCE) ) && !defined(EFIX64) && \
    !defined(EFI32)
/*
 * Check if the requested operation would be blocking on a non-blocking socket
 * and thus 'failed' with a negative return value.
 */
static int net_would_block( const mbedtls_net_context *ctx )
{
    ((void) ctx);
    return( WSAGetLastError() == WSAEWOULDBLOCK );
}
#else
/*
 * Check if the requested operation would be blocking on a non-blocking socket
 * and thus 'failed' with a negative return value.
 *
 * Note: on a blocking socket this function always returns 0!
 */
static int net_would_block( const mbedtls_net_context *ctx )
{
    return aliyun_iot_network_get_nonblock(ctx->fd);
//    /*
//     * Never return 'WOULD BLOCK' on a non-blocking socket
//     */
//    if( ( fcntl( ctx->fd, F_GETFL, 0 ) & O_NONBLOCK ) != O_NONBLOCK )
//        return( 0 );
//
//    switch( errno )
//    {
//#if defined EAGAIN
//        case EAGAIN:
//#endif
//#if defined EWOULDBLOCK && EWOULDBLOCK != EAGAIN
//        case EWOULDBLOCK:
//#endif
//            return( 1 );
//    }
//    return( 0 );
}
#endif /* ( _WIN32 || _WIN32_WCE ) && !EFIX64 && !EFI32 */

/*
 * Accept a connection from a remote client
 */
int mbedtls_net_accept( mbedtls_net_context *bind_ctx,
                        mbedtls_net_context *client_ctx,
                        void *client_ip, size_t buf_size, size_t *ip_len )
{
    bind_ctx = bind_ctx;
    client_ctx = client_ctx;
    client_ip = client_ip;
    buf_size = buf_size;
    ip_len = ip_len;
    return( MBEDTLS_ERR_NET_ACCEPT_FAILED );
#if 0
    int ret;
    int type;

    struct sockaddr_storage client_addr;

#if defined(__socklen_t_defined) || defined(_SOCKLEN_T) ||  \
    defined(_SOCKLEN_T_DECLARED) || defined(__DEFINED_socklen_t)
    socklen_t n = (socklen_t) sizeof( client_addr );
    socklen_t type_len = (socklen_t) sizeof( type );
#else
    int n = (int) sizeof( client_addr );
    int type_len = (int) sizeof( type );
#endif

    /* Is this a TCP or UDP socket? */
    if( getsockopt( bind_ctx->fd, SOL_SOCKET, SO_TYPE,
                    (void *) &type, &type_len ) != 0 ||
        ( type != SOCK_STREAM && type != SOCK_DGRAM ) )
    {
        return( MBEDTLS_ERR_NET_ACCEPT_FAILED );
    }

    if( type == SOCK_STREAM )
    {
        /* TCP: actual accept() */
        ret = client_ctx->fd = (int) accept( bind_ctx->fd,
                                         (struct sockaddr *) &client_addr, &n );
    }
    else
    {
        /* UDP: wait for a message, but keep it in the queue */
        char buf[1] = { 0 };

        ret = (int) recvfrom( bind_ctx->fd, buf, sizeof( buf ), MSG_PEEK,
                        (struct sockaddr *) &client_addr, &n );

#if defined(_WIN32)
        if( ret == SOCKET_ERROR &&
            WSAGetLastError() == WSAEMSGSIZE )
        {
            /* We know buf is too small, thanks, just peeking here */
            ret = 0;
        }
#endif
    }

    if( ret < 0 )
    {
        if( net_would_block( bind_ctx ) != 0 )
            return( MBEDTLS_ERR_SSL_WANT_READ );

        return( MBEDTLS_ERR_NET_ACCEPT_FAILED );
    }

    /* UDP: hijack the listening socket to communicate with the client,
     * then bind a new socket to accept new connections */
    if( type != SOCK_STREAM )
    {
        struct sockaddr_storage local_addr;
        int one = 1;

        if( connect( bind_ctx->fd, (struct sockaddr *) &client_addr, n ) != 0 )
            return( MBEDTLS_ERR_NET_ACCEPT_FAILED );

        client_ctx->fd = bind_ctx->fd;
        bind_ctx->fd   = -1; /* In case we exit early */

        n = sizeof( struct sockaddr_storage );
        if( getsockname( client_ctx->fd,
                         (struct sockaddr *) &local_addr, &n ) != 0 ||
            ( bind_ctx->fd = (int) socket( local_addr.ss_family,
                                           SOCK_DGRAM, IPPROTO_UDP ) ) < 0 ||
            setsockopt( bind_ctx->fd, SOL_SOCKET, SO_REUSEADDR,
                        (const char *) &one, sizeof( one ) ) != 0 )
        {
            return( MBEDTLS_ERR_NET_SOCKET_FAILED );
        }

        if( bind( bind_ctx->fd, (struct sockaddr *) &local_addr, n ) != 0 )
        {
            return( MBEDTLS_ERR_NET_BIND_FAILED );
        }
    }

    if( client_ip != NULL )
    {
        if( client_addr.ss_family == AF_INET )
        {
            struct sockaddr_in *addr4 = (struct sockaddr_in *) &client_addr;
            *ip_len = sizeof( addr4->sin_addr.s_addr );

            if( buf_size < *ip_len )
                return( MBEDTLS_ERR_NET_BUFFER_TOO_SMALL );

            memcpy( client_ip, &addr4->sin_addr.s_addr, *ip_len );
        }
        else
        {
        #if LWIP_IPV6
            struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) &client_addr;
            *ip_len = sizeof( addr6->sin6_addr.s6_addr );

            if( buf_size < *ip_len )
                return( MBEDTLS_ERR_NET_BUFFER_TOO_SMALL );

            memcpy( client_ip, &addr6->sin6_addr.s6_addr, *ip_len);
        #endif
        }
    }

    return( 0 );
#endif
}

/*
 * Set the socket blocking or non-blocking
 */
int mbedtls_net_set_block( mbedtls_net_context *ctx )
{
#if ( defined(_WIN32) || defined(_WIN32_WCE) ) && !defined(EFIX64) && \
    !defined(EFI32)
    u_long n = 0;
    return( ioctlsocket( ctx->fd, FIONBIO, &n ) );
#else
//    return( fcntl( ctx->fd, F_SETFL, fcntl( ctx->fd, F_GETFL, 0 ) & ~O_NONBLOCK ) );
    return (aliyun_iot_network_set_block(ctx->fd));
#endif
}

int mbedtls_net_set_nonblock( mbedtls_net_context *ctx )
{
#if ( defined(_WIN32) || defined(_WIN32_WCE) ) && !defined(EFIX64) && \
    !defined(EFI32)
    u_long n = 1;
    return( ioctlsocket( ctx->fd, FIONBIO, &n ) );
#else
//    return( fcntl( ctx->fd, F_SETFL, fcntl( ctx->fd, F_GETFL, 0 ) | O_NONBLOCK ) );
    return (aliyun_iot_network_set_nonblock(ctx->fd));
#endif
}

/*
 * Portable usleep helper
 */
void mbedtls_net_usleep( unsigned long usec )
{
#if defined(_WIN32)
    Sleep( ( usec + 999 ) / 1000 );
#else
//    struct timeval tv;
//    tv.tv_sec  = usec / 1000000;
#if defined(__unix__) || defined(__unix) || \
    ( defined(__APPLE__) && defined(__MACH__) )
//    tv.tv_usec = (suseconds_t) usec % 1000000;
#else
//    tv.tv_usec = usec % 1000000;
#endif

    aliyun_iot_pthread_taskdelay(usec/1000);
#endif
}

/*
 * Read at most 'len' characters
 */
int mbedtls_net_recv( void *ctx, unsigned char *buf, size_t len )
{
    int ret;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if( fd < 0 )
        return( MBEDTLS_ERR_NET_INVALID_CONTEXT );

    ret = (int) aliyun_iot_network_recv( fd, buf, len,IOT_NET_FLAGS_DEFAULT);
    if( ret < 0 )
    {
        int32_t err = aliyun_iot_get_errno();

        if( net_would_block( ctx ) != 0 )
        {
            return( MBEDTLS_ERR_SSL_WANT_READ );
        }

        if(err == EPIPE_IOT || err == ECONNRESET_IOT)
        {
            return( MBEDTLS_ERR_NET_CONN_RESET );
        }

        if(err == EINTR_IOT)
        {
            return( MBEDTLS_ERR_SSL_WANT_READ );
        }
        return( MBEDTLS_ERR_NET_RECV_FAILED );
    }

    return( ret );
}

/*
 * Read at most 'len' characters, blocking for at most 'timeout' ms
 */
int mbedtls_net_recv_timeout( void *ctx, unsigned char *buf, size_t len, uint32_t timeout )
{
    IOT_NET_FD_ISSET_E result;
    int ret;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if( fd < 0 )
    {
        return( MBEDTLS_ERR_NET_INVALID_CONTEXT );
    }

    ret = aliyun_iot_network_select(fd,IOT_NET_TRANS_RECV,timeout,&result);
    if(ret == 0 )
    {
        return( MBEDTLS_ERR_SSL_TIMEOUT );
    }
    else if(ret < 0 )
    {
        int32_t err = aliyun_iot_get_errno();
        if(err == EINTR_IOT)
        {
            return( MBEDTLS_ERR_SSL_WANT_READ );
        }
        else
        {
            return( MBEDTLS_ERR_NET_RECV_FAILED );
        }
    }
    else
    {
        aliyun_iot_network_settimeout(fd,50,IOT_NET_TRANS_RECV);

        /* This call will not block */
        return( mbedtls_net_recv( ctx, buf, len ) );
    }
}

/*
 * Write at most 'len' characters
 */
int mbedtls_net_send( void *ctx, const unsigned char *buf, size_t len )
{
    int32_t timeout = 3000;
    ALIYUN_IOT_TIME_TYPE_S endTime;
    int32_t lefttime = 0;
    int ret = 0;
    int sendlen = 0;
    IOT_NET_FD_ISSET_E result;
    int fd = -1;

    fd = ((mbedtls_net_context *) ctx)->fd;

    if( fd < 0 )
    {
        return( MBEDTLS_ERR_NET_INVALID_CONTEXT );
    }

    aliyun_iot_timer_cutdown(&endTime,timeout);

    do
    {
        lefttime = aliyun_iot_timer_remain(&endTime);
        if(lefttime <= 0)
        {
            return MBEDTLS_ERR_SSL_TIMEOUT;
        }

        ret = aliyun_iot_network_select(fd,IOT_NET_TRANS_SEND,lefttime,&result);
        if(ret == 0 )
        {
            return( MBEDTLS_ERR_SSL_TIMEOUT );
        }
        else if(ret < 0)
        {
            int32_t err = aliyun_iot_get_errno();
            if(err == EINTR_IOT)
            {
                continue;
            }
            else
            {
                return( MBEDTLS_ERR_NET_SEND_FAILED );
            }
        }
        else
        {
            if(IOT_NET_FD_NO_ISSET == result)
            {
                continue;
            }

            aliyun_iot_network_settimeout(fd,50,IOT_NET_TRANS_SEND);

            ret = (int) aliyun_iot_network_send( fd, (void*)buf, len ,IOT_NET_FLAGS_DEFAULT);
            if( ret < 0 )
            {
                int32_t err = aliyun_iot_get_errno();

                if( net_would_block( ctx ) != 0 )
                {
                    return( MBEDTLS_ERR_SSL_WANT_WRITE );
                }

                if(err == EPIPE_IOT || err == ECONNRESET_IOT)
                {
                    return( MBEDTLS_ERR_NET_CONN_RESET );
                }

                if(err == EINTR_IOT)
                {
                    continue;
                }

                return( MBEDTLS_ERR_NET_SEND_FAILED );
            }
            else if( ret == 0 )
            {
                return MBEDTLS_ERR_NET_SEND_FAILED;
            }
            else
            {
                sendlen += ret;
            }
        }

    }while(sendlen < (int)len);

    return sendlen;
}

/*
 * Gracefully close the connection
 */
void mbedtls_net_free( mbedtls_net_context *ctx )
{
    if( ctx->fd == -1 )
        return;

    aliyun_iot_network_shutdown( ctx->fd, 2 );
    aliyun_iot_pthread_taskdelay(20);
    aliyun_iot_network_close( ctx->fd );

    ctx->fd = -1;
}

#endif /* MBEDTLS_NET_C */

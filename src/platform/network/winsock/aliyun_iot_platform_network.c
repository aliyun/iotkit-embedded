
#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>


#include "aliot_platform_network.h"

static uint64_t time_left(uint64_t t_end, uint64_t t_now)
{
    uint64_t t_left;

    if (t_end > t_now) {
        t_left = t_end - t_now;
    } else {
        t_left = 0;
    }

    return t_left;
}

intptr_t aliot_platform_tcp_establish(const char *host, uint16_t port)
{
    WSADATA wsaData;
    struct addrinfo hints;
    struct addrinfo *addrInfoList = NULL;
    struct addrinfo *cur = NULL;
    int fd = 0;
    int rc = 0;
    char service[6];

    //Initialize Winsock
    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0) {
        printf("WSAStartup failed: %d", rc);
        return -1;
    }

    memset(&hints, 0, sizeof(hints));

    //默认支持IPv4的服务
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(service, "%u", port);

    if ((rc = getaddrinfo(host, service, &hints, &addrInfoList)) != 0) {
        printf("getaddrinfo error! rc = %d, errno = %d", rc, errno);
        return -1;
    }

    for (cur = addrInfoList; cur != NULL; cur = cur->ai_next) {
        //默认只支持IPv4
        if (cur->ai_family != AF_INET) {
            printf("socket type error");
            rc = 0;
            continue;
        }

        fd = (int) socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0) {
            printf("create socket error,fd = %d, errno = %d", fd, errno);
            rc = -1;
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            rc = fd;
            break;
        }

        closesocket(fd);
        printf("connect error, errno = %d", WSAGetLastError());
        rc = -1;
    }

    freeaddrinfo(addrInfoList);

    return (intptr_t)rc;
}


int aliot_platform_tcp_destroy(intptr_t fd)
{
    int rc;

    //Shutdown both send and receive operations.
    rc = shutdown((int) fd, 2);
    if (0 != rc) {
        printf("shutdown error, errno = %d", WSAGetLastError());
        return -1;
    }

    rc = closesocket((int) fd);
    if (0 != rc) {
        printf("closesocket error, errno = %d", WSAGetLastError());
        return -1;
    }

    rc = WSACleanup();
    if (0 != rc) {
        printf("WSACleanup error, errno = %d", WSAGetLastError());
        return -1;
    }

    return 0;
}



int32_t aliot_platform_tcp_write(intptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    int rc, ret;
    uint32_t len_sent;
    uint64_t t_end, t_left;
    fd_set sets;

    t_end = GetTickCount64( ) + timeout_ms;
    len_sent = rc = 0,
    ret = 1; //send one time if timeout_ms is value 0

    do {
        t_left = time_left(t_end, GetTickCount64( ));

        if (0 != t_left) {
            struct timeval timeout;

            FD_ZERO( &sets );
            FD_SET(fd, &sets);

            timeout.tv_sec = t_left / 1000;
            timeout.tv_usec = (t_left % 1000) * 1000;

            ret = select(0, NULL, &sets, NULL, &timeout);

            if (ret < 0) {
                //TODO WSAEINTR
                printf("select-write fail, errno=%d\n", WSAGetLastError());
                break;
            } else if (0 == ret) {
                printf("select-write timeout\n");
                break;
            } else {
                if (0 == FD_ISSET(fd, &sets)) {
                    //TODO
                    //not this fd, continue;
                    //If timeout in next loop, it will not sent any data
                    ret = 0;
                    continue;
                }
            }
        }

        if (ret > 0) {
            rc = send(fd, buf + len_sent, len - len_sent, 0);
            if (SOCKET_ERROR == rc) { //error occur
                printf("send fail, errno=%d\n", WSAGetLastError());
            } else {
                len_sent += rc;
            }
        }
    } while((len_sent < len) && (time_left(t_end, GetTickCount64()) > 0));

    return len_sent;
}


int32_t aliot_platform_tcp_read(intptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    int rc, ret;
    uint32_t len_recv;
    uint64_t t_end, t_left;
    fd_set sets;
    struct timeval timeout;

    t_end = GetTickCount64( ) + timeout_ms;
    len_recv = rc = 0;

    do {
        t_left = time_left(t_end, GetTickCount64( ));
        if (0 == t_left) {
            break;
        }
        FD_ZERO( &sets );
        FD_SET(fd, &sets);

        timeout.tv_sec = t_left / 1000;
        timeout.tv_usec = (t_left % 1000) * 1000;

        ret = select(0, &sets, NULL, NULL, &timeout);
        if (ret < 0) {
            //TODO WSAEINTR
            printf("select-recv fail, errno=%d\n", WSAGetLastError());
            break;
        } else if (0 == ret) {
            break;
        } else {
            rc = recv(fd, buf + len_recv, len - len_recv, 0);
            if (rc > 0) {
                len_recv += rc;
            } else if (0 == rc) {
                printf("connection is closed\n");
                break;
            } else {
                //TODO WSAEINTR
                printf("recv fail, errno=%d\n", WSAGetLastError());
            }
        }
    } while ((len_recv < len));

    return len_recv;
}

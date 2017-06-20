
#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <windows.h>
#include <Winbase.h>
#include <string.h>

#include "aliot_platform_network.h"

#define PLATFORM_WINSOCK_LOG(format, ...) \
    do { \
        printf("WINSOCK LINE=%u FUNC=%s(): "format"\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);\
        fflush(stdout);\
    }while(0);

#define PLATFORM_WINSOCK_PERROR(log) \
    do { \
        char *s = NULL; \
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, \
               NULL, \
               WSAGetLastError(), \
               MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), \
               (LPSTR)&s, \
               0, \
               NULL); \
        printf("WINSOCK LINE=%d FUNC=%s() %s: %s", __LINE__, __FUNCTION__, log, s); \
        fflush(stdout);\
        LocalFree(s); \
    }while(0);

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


uintptr_t aliot_platform_tcp_establish(const char *host, uint16_t port)
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
        PLATFORM_WINSOCK_PERROR("WSAStartup failed");
        return 0;
    }

    memset(&hints, 0, sizeof(hints));

    //默认支持IPv4的服务
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(service, "%u", port);

    if ((rc = getaddrinfo(host, service, &hints, &addrInfoList)) != 0) {
        PLATFORM_WINSOCK_PERROR("getaddrinfo error");
        return 0;
    }

    for (cur = addrInfoList; cur != NULL; cur = cur->ai_next) {
        //默认只支持IPv4
        if (cur->ai_family != AF_INET) {
            PLATFORM_WINSOCK_LOG("socket type error");
            rc = 0;
            continue;
        }

        fd = (int) socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0) {
            PLATFORM_WINSOCK_PERROR("create socket error");
            rc = 0;
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            rc = fd;
            break;
        }

        closesocket(fd);
        PLATFORM_WINSOCK_PERROR("connect error");
        rc = 0;
    }

    freeaddrinfo(addrInfoList);

    return (uintptr_t)rc;
}


int aliot_platform_tcp_destroy(uintptr_t fd)
{
    int rc;

    //Shutdown both send and receive operations.
    rc = shutdown((int) fd, 2);
    if (0 != rc) {
        PLATFORM_WINSOCK_PERROR("shutdown error");
        return -1;
    }

    rc = closesocket((int) fd);
    if (0 != rc) {
        PLATFORM_WINSOCK_PERROR("closesocket error");
        return -1;
    }

    rc = WSACleanup();
    if (0 != rc) {
        PLATFORM_WINSOCK_PERROR("WSACleanup error");
        return -1;
    }

    return 0;
}


int32_t aliot_platform_tcp_write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret, err_code;
    uint32_t len_sent;
    uint64_t t_end, t_left;
    fd_set sets;

    t_end = GetTickCount64( ) + timeout_ms;
    len_sent = 0;
    err_code = 0;
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
            if (ret > 0) {
               if (0 == FD_ISSET(fd, &sets)) {
                    //TODO
                    //not this fd, continue;
                    //If timeout in next loop, it will not sent any data
                    ret = 0;
                    continue;
                }
            } else if (0 == ret) {
                PLATFORM_WINSOCK_LOG("select-write timeout");
                break;
            } else {
                if (WSAEINTR == WSAGetLastError()) {
                    continue;
                }
                PLATFORM_WINSOCK_PERROR("select-write fail");
                err_code = -1;
                break;
            }
        }

        if (ret > 0) {
            ret = send(fd, buf + len_sent, len - len_sent, 0);
            if (ret > 0) {
                len_sent += ret;
            } else if (0 == ret) {
                PLATFORM_WINSOCK_LOG("No any data be sent");
            } else {
                //socket error occur
                PLATFORM_WINSOCK_PERROR("send fail");
                err_code = -1;
                break;
            }
        }
    } while((len_sent < len) && (time_left(t_end, GetTickCount64()) > 0));

    //Priority to return data bytes if any data be sent to TCP connection.
    //It will get error code on next calling
    return (0 == len_sent) ? err_code : len_sent;
}


int32_t aliot_platform_tcp_read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret, err_code;
    uint32_t len_recv;
    uint64_t t_end, t_left;
    fd_set sets;
    struct timeval timeout;

    t_end = GetTickCount64( ) + timeout_ms;
    len_recv = 0;
    err_code = 0;

    do {
        t_left = time_left(t_end, GetTickCount64( ));

        FD_ZERO( &sets );
        FD_SET(fd, &sets);

        timeout.tv_sec = t_left / 1000;
        timeout.tv_usec = (t_left % 1000) * 1000;

        ret = select(0, &sets, NULL, NULL, &timeout);
        if (ret > 0) {
            ret = recv(fd, buf + len_recv, len - len_recv, 0);
            if (ret > 0) {
                len_recv += ret;
            } else if (0 == ret) {
                PLATFORM_WINSOCK_LOG("connection is closed");
                err_code = -1;
                break;
            } else {
                if (WSAEINTR == WSAGetLastError()) {
                    continue;
                }
                PLATFORM_WINSOCK_PERROR("recv fail");
                err_code = -2;
                break;
            }
        } else if (0 == ret) {
            break;
        } else {
            if (WSAEINTR == WSAGetLastError()) {
                continue;
            }
            PLATFORM_WINSOCK_PERROR("select-read fail");
            err_code = -2;
            break;
        }
    } while ((len_recv < len) && (time_left(t_end, GetTickCount64()) > 0));

    //priority to return data bytes if any data be received from TCP connection.
    //It will get error code on next calling
    return (0 != len_recv) ? len_recv : err_code;
}

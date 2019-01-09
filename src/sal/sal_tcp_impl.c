/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "infra_config.h"
#include "sal_export.h"
#include "sal_wrapper.h"
#ifdef ATPARSER_ENABLED
#include "atparser.h"
#endif

static uint64_t _get_time_ms(void)
{
    return HAL_UptimeMs();
}

static uint64_t _time_left(uint64_t t_end, uint64_t t_now)
{
    uint64_t t_left;

    if (t_end > t_now) {
        t_left = t_end - t_now;
    } else {
        t_left = 0;
    }

    return t_left;
}

uintptr_t SAL_TCP_Establish(const char *host, uint16_t port)
{
    struct addrinfo hints;
    struct addrinfo *addrInfoList = NULL;
    struct addrinfo *cur = NULL;
    int fd = 0;
    int rc = 0;
    char service[6];

    memset(&hints, 0, sizeof(hints));

    HAL_Printf("establish tcp connection with server(host='%s', port=[%u])\n", host, port);

    hints.ai_family = AF_INET; /* only IPv4 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    HAL_Snprintf(service, sizeof(service), "%u", port);

    if ((rc = getaddrinfo(host, service, &hints, &addrInfoList)) != 0) {
        HAL_Printf("getaddrinfo error(%d), host = '%s', port = [%d]\n", rc, host, port);
        return -1;
    }

    for (cur = addrInfoList; cur != NULL; cur = cur->ai_next) {
        if (cur->ai_family != AF_INET) {
            HAL_Printf("socket type error\n");
            rc = -1;
            continue;
        }

        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0) {
            HAL_Printf("create socket error\n");
            rc = -1;
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            rc = fd;
            break;
        }

        close(fd);
        HAL_Printf("connect error\n");
        rc = -1;
    }

    if (-1 == rc) {
        HAL_Printf("fail to establish tcp\n");
    } else {
        HAL_Printf("success to establish tcp, fd=%d\n", rc);
    }
    freeaddrinfo(addrInfoList);

    return (uintptr_t)rc;
}

int SAL_TCP_Destroy(uintptr_t fd)
{
    int rc;

    /* Shutdown both send and receive operations. */
    rc = shutdown((int) fd, 2);
    if (0 != rc) {
        HAL_Printf("shutdown error\n");
        return -1;
    }

    rc = close((int) fd);
    if (0 != rc) {
        HAL_Printf("closesocket error\n");
        return -1;
    }

    return 0;
}

int32_t SAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret;
    uint32_t len_sent;
    uint64_t t_end;
    int net_err = 0;

    t_end = _get_time_ms() + timeout_ms;
    len_sent = 0;
    ret = 1; /* send one time if timeout_ms is value 0 */

    do {
        ret = send(fd, buf + len_sent, len - len_sent, 0);
        if (ret > 0) {
            len_sent += ret;
        } else if (0 == ret) {
            HAL_Printf("No data be sent\n");
        } else {
            if (EINTR == errno) {
                HAL_Printf("EINTR be caught\n");
                continue;
            }

            HAL_Printf("send fail, ret = send() = %d\n", ret);
            net_err = 1;
            break;
        }
    } while (!net_err && (len_sent < len) && (_time_left(t_end, _get_time_ms()) > 0));

    if (net_err) {
        return -1;
    } else {
        return len_sent;
    }
}

int32_t SAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret, err_code;
    uint32_t len_recv;
    uint64_t t_end, t_left;


    t_end = _get_time_ms() + timeout_ms;
    len_recv = 0;
    err_code = 0;

    do {
        t_left = _time_left(t_end, _get_time_ms());
        if (0 == t_left) {
            break;
        }

        ret = 1;

#ifdef ATPARSER_ENABLED
#if AT_SINGLE_TASK
        at_yield(NULL, 0, NULL, AT_UART_TIMEOUT_MS);
#endif
#endif
        if (ret > 0) {
            ret = recv(fd, buf + len_recv, len - len_recv, 0);
            if (ret > 0) {
                len_recv += ret;
            } else if (0 == ret) {
                HAL_Printf("connection is closed\n");
                err_code = -1;
                break;
            } else {
                if (EINTR == errno) {
                    HAL_Printf("EINTR be caught\n");
                    continue;
                }
                HAL_Printf("recv fail\n");
                err_code = -2;
                break;
            }
        } else if (0 == ret) {
            break;
        } else {
            HAL_Printf("select-recv fail\n");
            err_code = -2;
            break;
        }
    } while ((len_recv < len));

    /* priority to return data bytes if any data be received from TCP connection. */
    /* It will get error code on next calling */
    return (0 != len_recv) ? len_recv : err_code;
}


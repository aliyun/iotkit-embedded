/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>

#include "infra_types.h"
#include "infra_config.h"

#include "at_conn_mgmt.h"

#include "wrappers.h"

#ifdef AT_PARSER_ENABLED
#include "at_parser.h"
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

uintptr_t AT_SSL_Establish(const char *host,
                           uint16_t port,
                           const char *ca_crt,
                           uint32_t ca_crt_len)
{
    int fd = 0;
    int rc = 0;
    char resultip[16];
    cert_info_t cert_info = {0};

    HAL_Printf("establish ssl connection with server(host='%s', port=[%u])\n", host, port);

    if ((rc = at_conn_getaddrinfo(host, resultip)) != 0) {
        HAL_Printf("getaddrinfo error(%d), host = '%s', port = [%d]\n", rc, host, port);
        return (uintptr_t)(-1);
    }

    fd = at_conn_setup(NETCONN_SSL);
    if (fd < 0) {
        HAL_Printf("create at conn error\n");
        return (uintptr_t)(-1);
    }

    cert_info.cert_len = ca_crt_len;
    cert_info.cert_data = ca_crt;
    cert_info.cert_type = ROOT_CERT;

    if (at_conn_start(fd, resultip, port, &cert_info) == 0) {
        rc = fd;
    } else {
        at_conn_close(fd);
        HAL_Printf("connect error\n");
        rc = -1;
    }

    if (-1 == rc) {
        HAL_Printf("fail to establish ssl\n");
    } else {
        HAL_Printf("success to establish ssl, fd=%d\n", rc);
    }

    return (uintptr_t)rc;
}

int32_t AT_SSL_Destroy(uintptr_t handle)
{
    int rc;

    rc = at_conn_close((int) handle);
    if (0 != rc) {
        HAL_Printf("closesocket error\n");
        return -1;
    }

    return 0;
}

int AT_SSL_Write(uintptr_t handle, const char *buf, int len, int timeout_ms)
{
    int ret;
    uint32_t len_sent;
    uint64_t t_end;
    int net_err = 0;

    t_end = _get_time_ms() + timeout_ms;
    len_sent = 0;
    ret = 1; /* send one time if timeout_ms is value 0 */

    do {
        ret = at_conn_send((int)handle, buf + len_sent, len - len_sent);
        if (ret > 0) {
            len_sent += ret;
        } else if (0 == ret) {
            HAL_Printf("No data be sent\n");
        } else {
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

int AT_SSL_Read(uintptr_t handle, char *buf, int len, int timeout_ms)
{
    int ret;
    int err_code = 0;
    uint32_t len_recv = 0;
    uint64_t t_end, t_left;
    int empty;

    t_end = _get_time_ms() + timeout_ms;

    do {
        t_left = _time_left(t_end, _get_time_ms());
        if (0 == t_left) {
            break;
        }

        while(1) {
#ifdef AT_PARSER_ENABLED
#if AT_SINGLE_TASK
            at_yield(NULL, 0, NULL, 100);
#endif
#endif
            empty = at_conn_recvbufempty((int)handle);
            if (0 == empty) {
                ret = 1;
                break;
            } else if (empty < 0) {
                ret = -1;
            }

            t_left = _time_left(t_end, _get_time_ms());
            if (0 == t_left) {
                ret = 0;
                break;
            }

            HAL_SleepMs(10);
        }

        if (ret > 0) {
            ret = at_conn_recv((int)handle, buf + len_recv, len - len_recv);
            if (ret > 0) {
                len_recv += ret;
            } else if (0 == ret) {
                HAL_Printf("connection is closed\n");
                err_code = -1;
                break;
            } else {
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


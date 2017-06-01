/*
 * aliyun_iot_common_net.c
 *
 *  Created on: May 5, 2017
 *      Author: qibiao.wqb
 */


#include "aliyun_iot_common_net.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_hexdump.h"
#include "aliyun_iot_common_timer.h"
#include "aliyun_iot_network_ssl.h"
#include "aliyun_iot_platform_network.h"
#include "aliyun_iot_platform_memory.h"

/* TCP connection */

static int read_tcp(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    int rc = 0;
    int ret = -1;
    int32_t recvlen = 0;
    aliot_time_t time;
    uint32_t lefttime;
    IOT_NET_FD_ISSET_E result;

    aliyun_iot_timer_cutdown(&time, timeout_ms);

    do {
        lefttime = aliyun_iot_timer_remain(&time);
        if (lefttime <= 0) {
            break;
        }

        //ALIOT_LOG_DEBUG("read left time=%d ms", lefttime);

        ret = aliyun_iot_network_select(pNetwork->handle, IOT_NET_TRANS_RECV, lefttime, &result);
        if (ret < 0) {
            int32_t err = aliyun_iot_get_errno();
            if (err == EINTR_IOT) {
                continue;
            } else {
                ALIOT_LOG_ERROR("read select fail ret=%d", ret);
                return -1;
            }
        } else if (ret == 0) {
            //select timeout
            //ALIOT_LOG_DEBUG("read select timeout");
            break;
        } else if (ret == 1) {
            if (IOT_NET_FD_NO_ISSET == result) {
                ALIOT_LOG_DEBUG("another fd readable!");
                continue;
            }

            aliyun_iot_network_settimeout(pNetwork->handle, 50, IOT_NET_TRANS_RECV);
            rc = aliyun_iot_network_recv(pNetwork->handle, buffer + recvlen, len - recvlen, IOT_NET_FLAGS_DEFAULT);
            if (rc > 0) {
                recvlen += rc;
            } else if (rc == 0) {
                ALIOT_LOG_ERROR("The network is broken!");
                break;
            } else {
                int32_t err = aliyun_iot_get_errno();
                if (err == EINTR_IOT || err == EWOULDBLOCK_IOT || err == EAGAIN_IOT) {
                    continue;
                } else {
                    ALIOT_LOG_ERROR("read fail");
                    return -3;
                }
            }
        }
    } while (recvlen < len);

    //ALIOT_LOG_INFO("%u bytes be received.", recvlen);
    //aliyun_iot_common_hexdump(ALIOT_HEXDUMP_PREFIX_OFFSET, 32, 1, buffer, recvlen, 1);
    return recvlen;
}


static int write_tcp(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    int rc = 0;
    int ret = -1;
    int32_t sendlen = 0;
    aliot_time_t time;
    uint32_t lefttime;
    IOT_NET_FD_ISSET_E result;

    aliyun_iot_timer_cutdown(&time, timeout_ms);

    do {
        lefttime = aliyun_iot_timer_remain(&time);
        if (lefttime <= 0) {
            ALIOT_LOG_ERROR("write timeout");
            break;
        }

        ret = aliyun_iot_network_select(pNetwork->handle, IOT_NET_TRANS_SEND, lefttime, &result);
        if (ret < 0) {
            int32_t err = aliyun_iot_get_errno();
            if (err == EINTR_IOT) {
                continue;
            } else {
                ALIOT_LOG_ERROR("write fail");
                return -1;
            }
        } else if (ret == 0) {
            ALIOT_LOG_DEBUG("write select timeout");
            break;
        } else if (ret == 1) {
            if (IOT_NET_FD_NO_ISSET == result) {
                ALIOT_LOG_DEBUG("another fd readable!");
                continue;
            }

            aliyun_iot_network_settimeout(pNetwork->handle, 50, IOT_NET_TRANS_SEND);

            rc = aliyun_iot_network_send(pNetwork->handle, buffer, len, IOT_NET_FLAGS_DEFAULT);
            if (rc > 0) {
                sendlen += rc;
            } else if (rc == 0) {
                ALIOT_LOG_ERROR("The network is broken!");
                break;
            } else {
                int32_t err = aliyun_iot_get_errno();
                if (err == EINTR_IOT || err == EWOULDBLOCK_IOT
                    || err == EAGAIN_IOT) {
                    continue;
                } else {
                    ALIOT_LOG_ERROR("write fail:");
                    return -3;
                }
            }
        }
    } while ((sendlen < len) && !aliyun_iot_timer_expired(&time));

    ALIOT_LOG_INFO("%u bytes be sent.", sendlen);
    return sendlen;
}

static int disconnect_tcp(pNetwork_t pNetwork)
{
    if (pNetwork->handle < 0) {
        return -1;
    }

    aliyun_iot_network_shutdown(pNetwork->handle, 2);
    aliyun_iot_pthread_taskdelay(20);
    aliyun_iot_network_close(pNetwork->handle);

    pNetwork->handle = -1;

    return 0;
}


static int connect_tcp(pNetwork_t pNetwork)
{
    if (NULL == pNetwork) {
        ALIOT_LOG_ERROR("network is null");
        return 1;
    }

    //todo port should be integer type.
    pNetwork->handle = (intptr_t)aliyun_iot_network_create(pNetwork->pHostAddress, pNetwork->port, IOT_NET_PROTOCOL_TCP);
    if (pNetwork->handle < 0) {
        return -1;
    }

    return 0;
}


/* SSL connection */

static int read_ssl(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork) {
        ALIOT_LOG_ERROR("network is null");
        return 1;
    }

    return aliyun_iot_network_ssl_read((void *)pNetwork->handle, buffer, len, timeout_ms);
}

static int write_ssl(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork) {
        ALIOT_LOG_ERROR("network is null");
        return 1;
    }

    return aliyun_iot_network_ssl_write((void *)pNetwork->handle, buffer, len, timeout_ms);
}

static int disconnect_ssl(pNetwork_t pNetwork)
{
    if (NULL == pNetwork) {
        ALIOT_LOG_ERROR("network is null");
        return 1;
    }

    aliyun_iot_network_ssl_disconnect((void *)pNetwork->handle);
    aliyun_iot_memory_free(pNetwork->handle);
    pNetwork->handle = NULL;

    return 0;
}

static int connect_ssl(pNetwork_t pNetwork)
{
    if (NULL == pNetwork) {
        ALIOT_LOG_ERROR("network is null");
        return 1;
    }

    pNetwork->handle = (intptr_t)aliyun_iot_memory_malloc(sizeof(TLSDataParams));
    if (NULL == (void *)pNetwork->handle) {
        return -1;
    }

    if (0 == aliyun_iot_network_ssl_connect(
                    (void *)pNetwork->handle,
                    pNetwork->pHostAddress,
                    pNetwork->port,
                    pNetwork->ca_crt,
                    strlen(pNetwork->ca_crt) + 1)) {
        return 0;
    } else {
        //TODO SHOLUD not remove this handle space
        // The space will be freed by calling disconnect_ssl()
        //aliyun_iot_memory_free((void *)pNetwork->handle);
        return -1;
    }
}



/****** network interface ******/

int aliyun_iot_net_read(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork->ca_crt) { //TCP connection
        return read_tcp(pNetwork, buffer, len, timeout_ms);
    } else { //SSL connection
        return read_ssl(pNetwork, buffer, len, timeout_ms);
    }
}


int aliyun_iot_net_write(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork->ca_crt) { //TCP connection
        return write_tcp(pNetwork, buffer, len, timeout_ms);
    } else { //SSL connection
        return write_ssl(pNetwork, buffer, len, timeout_ms);
    }
}


int aliyun_iot_net_disconnect(pNetwork_t pNetwork)
{
    if (NULL == pNetwork->ca_crt) { //TCP connection
        return disconnect_tcp(pNetwork);
    } else { //SSL connection
        return disconnect_ssl(pNetwork);
    }
}


intptr_t aliyun_iot_net_connect(pNetwork_t pNetwork)
{
    if (NULL == pNetwork->ca_crt) { //TCP connection
        return connect_tcp(pNetwork);
    } else { //SSL connection
        return connect_ssl(pNetwork);
    }
}


int aliyun_iot_net_init(pNetwork_t pNetwork, char *host, char *port, char *ca_crt)
{
    pNetwork->pHostAddress = host;
    pNetwork->port = port;
    pNetwork->ca_crt = ca_crt;

    pNetwork->handle = -1;
    pNetwork->read = aliyun_iot_net_read;
    pNetwork->write = aliyun_iot_net_write;
    pNetwork->disconnect = aliyun_iot_net_disconnect;
    pNetwork->connect = aliyun_iot_net_connect;

    return 0;
}

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
    int recvlen = 0;
    int ret = -1;

    aliot_time_t endTime;
    aliyun_iot_timer_cutdown(&endTime,timeout_ms);
    do
    {
        int32_t lefttime = aliyun_iot_timer_remain(&endTime);
        if(lefttime <= 0)
        {
            WRITE_IOT_ERROR_LOG("mqtt read timeout");
            return -2;
        }

        WRITE_IOT_DEBUG_LOG("mqtt read left time=%d ms", lefttime);

        IOT_NET_FD_ISSET_E result;
        ret = aliyun_iot_network_select(pNetwork->handle, IOT_NET_TRANS_RECV, lefttime, &result);
        if (ret < 0)
        {
            int32_t err = aliyun_iot_get_errno();
            if(err == EINTR_IOT)
            {
                continue;
            }
            else
            {
                WRITE_IOT_ERROR_LOG("mqtt read(select) fail ret=%d", ret);
                return -1;
            }
        }
        else if (ret == 0)
        {
            WRITE_IOT_DEBUG_LOG("mqtt read(select) timeout");
            return 0;
        }
        else if (ret == 1)
        {
            if(IOT_NET_FD_NO_ISSET == result)
            {
                WRITE_IOT_DEBUG_LOG("another fd readable!");
                continue;
            }

            aliyun_iot_network_settimeout(pNetwork->handle, 50, IOT_NET_TRANS_RECV);

            WRITE_IOT_DEBUG_LOG("mqtt read recv len = %d, recvlen = %d", len, recvlen);
            rc = aliyun_iot_network_recv(pNetwork->handle, buffer + recvlen, len - recvlen, IOT_NET_FLAGS_DEFAULT);
            if (rc > 0)
            {
                recvlen += rc;
                WRITE_IOT_DEBUG_LOG("mqtt read ret=%d, rc = %d, recvlen = %d", ret, rc, recvlen);
            }
            else if(rc == 0)
            {
                WRITE_IOT_ERROR_LOG("The network is broken!,recvlen = %d", recvlen);
                return recvlen;
            }
            else
            {
                int32_t err = aliyun_iot_get_errno();
                if (err == EINTR_IOT || err == EWOULDBLOCK_IOT || err == EAGAIN_IOT)
                {
                    continue;
                }
                else
                {
                    WRITE_IOT_ERROR_LOG("mqtt read fail: ret=%d, rc = %d, recvlen = %d", ret, rc, recvlen);
                    return -3;
                }
            }
        }
    }while(recvlen < len);

    return recvlen;
}


static int write_tcp(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    int rc = 0;
    int ret = -1;
    int32_t sendlen = 0;

    int32_t timeout = timeout_ms;
    aliot_time_t endTime;
    aliyun_iot_timer_cutdown(&endTime,timeout);

    do
    {
        int32_t lefttime = aliyun_iot_timer_remain(&endTime);
        if(lefttime <= 0)
        {
            WRITE_IOT_ERROR_LOG("mqtt write timeout");
            return -2;
        }

        IOT_NET_FD_ISSET_E result;
        ret = aliyun_iot_network_select(pNetwork->handle, IOT_NET_TRANS_SEND, lefttime, &result);
        if (ret < 0)
        {
            int32_t err = aliyun_iot_get_errno();
            if(err == EINTR_IOT)
            {
                continue;
            }
            else
            {
                WRITE_IOT_ERROR_LOG("mqtt write fail");
                return -1;
            }
        }
        else if (ret == 0)
        {
            WRITE_IOT_ERROR_LOG("mqtt write timeout");
            return -2;
        }
        else if (ret == 1)
        {
            if(IOT_NET_FD_NO_ISSET == result)
            {
                WRITE_IOT_DEBUG_LOG("another fd readable!");
                continue;
            }

            aliyun_iot_network_settimeout(pNetwork->handle, 50, IOT_NET_TRANS_SEND);

            rc = aliyun_iot_network_send(pNetwork->handle, buffer, len, IOT_NET_FLAGS_DEFAULT);
            if(rc > 0)
            {
                sendlen += rc;
            }
            else if(rc == 0)
            {
                WRITE_IOT_ERROR_LOG("The network is broken!");
                return -1;
            }
            else
            {
                int32_t err = aliyun_iot_get_errno();
                if(err == EINTR_IOT || err == EWOULDBLOCK_IOT || err == EAGAIN_IOT)
                {
                    continue;
                }
                else
                {
                    WRITE_IOT_ERROR_LOG("mqtt read fail: ret=%d, rc = %d, err = %d", ret, rc,err);
                    return -3;
                }
            }
        }
    }while(sendlen < len);

    return sendlen;
}

static int disconnect_tcp(pNetwork_t pNetwork)
{
    if( pNetwork->handle < 0 ) {
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
    if(NULL == pNetwork)
    {
        WRITE_IOT_ERROR_LOG("network is null");
        return 1;
    }

    //todo port should be integer type.
    pNetwork->handle = (intptr_t)aliyun_iot_network_create(pNetwork->pHostAddress, pNetwork->port, IOT_NET_PROTOCOL_TCP);
    if(pNetwork->handle < 0 )
    {
        return -1;
    }

    return 0;
}


/* SSL connection */

static int read_ssl(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if(NULL == pNetwork)
    {
        WRITE_IOT_ERROR_LOG("network is null");
        return 1;
    }

    return aliyun_iot_network_ssl_read((void *)pNetwork->handle, buffer, len, timeout_ms);
}

static int write_ssl(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if(NULL == pNetwork)
    {
        WRITE_IOT_ERROR_LOG("network is null");
        return 1;
    }

    return aliyun_iot_network_ssl_write((void *)pNetwork->handle, buffer, len, timeout_ms);
}

static int disconnect_ssl(pNetwork_t pNetwork)
{
    if(NULL == pNetwork)
    {
        WRITE_IOT_ERROR_LOG("network is null");
        return 1;
    }

    aliyun_iot_network_ssl_disconnect((void *)pNetwork->handle);

    return 0;
}

static int connect_ssl(pNetwork_t pNetwork)
{
    if(NULL == pNetwork)
    {
        WRITE_IOT_ERROR_LOG("network is null");
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
        aliyun_iot_memory_free((void *)pNetwork->handle);
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
        int ret = disconnect_ssl(pNetwork);
        aliyun_iot_memory_free(pNetwork->handle);
        return ret;
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
    pNetwork->write= aliyun_iot_net_write;
    pNetwork->disconnect = aliyun_iot_net_disconnect;
    pNetwork->connect = aliyun_iot_net_connect;

    return 0;
}

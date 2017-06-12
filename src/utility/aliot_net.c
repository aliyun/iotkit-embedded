
#include <string.h>

#include "aliot_platform.h"
#include "aliot_net.h"
#include "aliot_log.h"
#include "aliot_hexdump.h"


/* TCP connection */
int read_tcp(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    return aliot_platform_tcp_read(pNetwork->handle, buffer, len, timeout_ms);
}


static int write_tcp(pNetwork_t pNetwork, const char *buffer, uint32_t len, uint32_t timeout_ms)
{
    return aliot_platform_tcp_write(pNetwork->handle, buffer, len, timeout_ms);
}

static int disconnect_tcp(pNetwork_t pNetwork)
{
    if (pNetwork->handle < 0) {
        return -1;
    }

    aliot_platform_tcp_destroy(pNetwork->handle);
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
    pNetwork->handle = aliot_platform_tcp_establish(pNetwork->pHostAddress, pNetwork->port);
    if (pNetwork->handle < 0) {
        return -1;
    }

    return 0;
}


/* SSL connection */

static int read_ssl(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
//#ifdef ALIOT_CHANNEL_ENCRYPT_SSL
    if (NULL == pNetwork) {
        ALIOT_LOG_ERROR("network is null");
        return -1;
    }

    return aliot_platform_ssl_read((void *)pNetwork->handle, buffer, len, timeout_ms);
//#else
//    return -1;
//#endif
}

static int write_ssl(pNetwork_t pNetwork, const char *buffer, uint32_t len, uint32_t timeout_ms)
{
//#ifdef ALIOT_CHANNEL_ENCRYPT_SSL
    if (NULL == pNetwork) {
        ALIOT_LOG_ERROR("network is null");
        return -1;
    }

    return aliot_platform_ssl_write((void *)pNetwork->handle, buffer, len, timeout_ms);
//#else
//    return -1;
//#endif
}

static int disconnect_ssl(pNetwork_t pNetwork)
{
//#ifdef ALIOT_CHANNEL_ENCRYPT_SSL
    if (NULL == pNetwork) {
        ALIOT_LOG_ERROR("network is null");
        return -1;
    }

    aliot_platform_ssl_disconnect((void *)pNetwork->handle);
    pNetwork->handle = 0;

    return 0;
//#else
//    return -1;
//#endif
}

static int connect_ssl(pNetwork_t pNetwork)
{
//#ifdef ALIOT_CHANNEL_ENCRYPT_SSL
    if (NULL == pNetwork) {
        ALIOT_LOG_ERROR("network is null");
        return 1;
    }

    if (0 != (pNetwork->handle = (intptr_t)aliot_platform_ssl_connect(
                                            pNetwork->pHostAddress,
                                            pNetwork->port,
                                            pNetwork->ca_crt,
                                            pNetwork->ca_crt_len + 1))) {
        return 0;
    } else {
        //TODO SHOLUD not remove this handle space
        // The space will be freed by calling disconnect_ssl()
        //aliyun_iot_memory_free((void *)pNetwork->handle);
        return -1;
    }
//#else
//    return -1;
//#endif
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


int aliyun_iot_net_write(pNetwork_t pNetwork, const char *buffer, uint32_t len, uint32_t timeout_ms)
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


int aliyun_iot_net_init(pNetwork_t pNetwork, const char *host, uint16_t port, const char *ca_crt)
{
    pNetwork->pHostAddress = host;
    pNetwork->port = port;
    pNetwork->ca_crt = ca_crt;
    if (NULL == ca_crt) {
        pNetwork->ca_crt_len = 0;
    } else {
        pNetwork->ca_crt_len = strlen(ca_crt);
    }

    pNetwork->handle = -1;
    pNetwork->read = aliyun_iot_net_read;
    pNetwork->write = aliyun_iot_net_write;
    pNetwork->disconnect = aliyun_iot_net_disconnect;
    pNetwork->connect = aliyun_iot_net_connect;

    return 0;
}


#include <string.h>

#include "aliot_platform.h"
#include "utils_net.h"
#include "lite/lite-log.h"
#include "utils_hexdump.h"


/*** TCP connection ***/
int read_tcp(utils_network_pt pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    return aliot_platform_tcp_read(pNetwork->handle, buffer, len, timeout_ms);
}


static int write_tcp(utils_network_pt pNetwork, const char *buffer, uint32_t len, uint32_t timeout_ms)
{
    return aliot_platform_tcp_write(pNetwork->handle, buffer, len, timeout_ms);
}

static int disconnect_tcp(utils_network_pt pNetwork)
{
    if ( 0 == pNetwork->handle) {
        return -1;
    }

    aliot_platform_tcp_destroy(pNetwork->handle);
    pNetwork->handle = 0;
    return 0;
}


static int connect_tcp(utils_network_pt pNetwork)
{
    if (NULL == pNetwork) {
        log_err("network is null");
        return 1;
    }

    pNetwork->handle = aliot_platform_tcp_establish(pNetwork->pHostAddress, pNetwork->port);
    if ( 0 == pNetwork->handle ) {
        return -1;
    }

    return 0;
}


/*** SSL connection ***/
#ifndef ALIOT_MQTT_TCP
static int read_ssl(utils_network_pt pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork) {
        log_err("network is null");
        return -1;
    }

    return aliot_platform_ssl_read((void *)pNetwork->handle, buffer, len, timeout_ms);
}

static int write_ssl(utils_network_pt pNetwork, const char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork) {
        log_err("network is null");
        return -1;
    }

    return aliot_platform_ssl_write((void *)pNetwork->handle, buffer, len, timeout_ms);
}

static int disconnect_ssl(utils_network_pt pNetwork)
{
    if (NULL == pNetwork) {
        log_err("network is null");
        return -1;
    }

    aliot_platform_ssl_destroy((void *)pNetwork->handle);
    pNetwork->handle = 0;

    return 0;
}

static int connect_ssl(utils_network_pt pNetwork)
{
    if (NULL == pNetwork) {
        log_err("network is null");
        return 1;
    }

    if (0 != (pNetwork->handle = (intptr_t)aliot_platform_ssl_establish(
                                            pNetwork->pHostAddress,
                                            pNetwork->port,
                                            pNetwork->ca_crt,
                                            pNetwork->ca_crt_len + 1))) {
        return 0;
    } else {
        //TODO SHOLUD not remove this handle space
        // The space will be freed by calling disconnect_ssl()
        //utils_memory_free((void *)pNetwork->handle);
        return -1;
    }
}
#endif  /* #ifndef ALIOT_MQTT_TCP */


/****** network interface ******/

int utils_net_read(utils_network_pt pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork->ca_crt) { //TCP connection
        return read_tcp(pNetwork, buffer, len, timeout_ms);
#ifndef ALIOT_MQTT_TCP
    } else { //SSL connection
        return read_ssl(pNetwork, buffer, len, timeout_ms);
#endif
    }
}


int utils_net_write(utils_network_pt pNetwork, const char *buffer, uint32_t len, uint32_t timeout_ms)
{
    if (NULL == pNetwork->ca_crt) { //TCP connection
        return write_tcp(pNetwork, buffer, len, timeout_ms);
#ifndef ALIOT_MQTT_TCP
    } else { //SSL connection
        return write_ssl(pNetwork, buffer, len, timeout_ms);
#endif
    }
}


int aliot_net_disconnect(utils_network_pt pNetwork)
{
    if (NULL == pNetwork->ca_crt) { //TCP connection
        return disconnect_tcp(pNetwork);
#ifndef ALIOT_MQTT_TCP
    } else { //SSL connection
        return disconnect_ssl(pNetwork);
#endif
    }
}


int aliot_net_connect(utils_network_pt pNetwork)
{
    if (NULL == pNetwork->ca_crt) { //TCP connection
        return connect_tcp(pNetwork);
#ifndef ALIOT_MQTT_TCP
    } else { //SSL connection
        return connect_ssl(pNetwork);
#endif
    }
}


int aliot_net_init(utils_network_pt pNetwork, const char *host, uint16_t port, const char *ca_crt)
{
    pNetwork->pHostAddress = host;
    pNetwork->port = port;
    pNetwork->ca_crt = ca_crt;
    if (NULL == ca_crt) {
        pNetwork->ca_crt_len = 0;
    } else {
        pNetwork->ca_crt_len = strlen(ca_crt);
    }

    pNetwork->handle = 0;
    pNetwork->read = utils_net_read;
    pNetwork->write = utils_net_write;
    pNetwork->disconnect = aliot_net_disconnect;
    pNetwork->connect = aliot_net_connect;

    return 0;
}

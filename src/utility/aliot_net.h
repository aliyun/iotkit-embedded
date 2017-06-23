
#ifndef _ALIOT_COMMON_NET_H_
#define _ALIOT_COMMON_NET_H_

#include "aliot_platform.h"


/**
 * @brief The structure of network connection(TCP or SSL).
 *   The user has to allocate memory for this structure.
 */

struct aliot_network;
typedef struct aliot_network aliot_network_t, *aliot_network_pt;

struct aliot_network {
    const char *pHostAddress;
    uint16_t port;
    uint16_t ca_crt_len;

    /**< NULL, TCP connection; NOT NULL, SSL connection */
    const char *ca_crt;

    /**< connection handle: 0, NOT connection; NOT 0, handle of the connection */
    uintptr_t handle;

    /**< Read data from server function pointer. */
    int (*read)(aliot_network_pt, char *, uint32_t, uint32_t);

    /**< Send data to server function pointer. */
    int (*write)(aliot_network_pt, const char *, uint32_t, uint32_t);

    /**< Disconnect the network */
    int (*disconnect)(aliot_network_pt);

    /**< Establish the network */
    int (*connect)(aliot_network_pt);
};


int aliot_net_read(aliot_network_pt pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms);
int aliot_net_write(aliot_network_pt pNetwork, const char *buffer, uint32_t len, uint32_t timeout_ms);
int aliot_net_disconnect(aliot_network_pt pNetwork);
int aliot_net_connect(aliot_network_pt pNetwork);
int aliot_net_init(aliot_network_pt pNetwork, const char *host, uint16_t port, const char *ca_crt);

#endif /* ALIOT_COMMON_NET_H */

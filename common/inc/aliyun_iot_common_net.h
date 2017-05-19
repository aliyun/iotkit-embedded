/*
 * aliyun_iot_common_net.h
 *
 *  Created on: May 5, 2017
 *      Author: qibiao.wqb
 */

#ifndef ALIYUN_IOT_COMMON_NET_H
#define ALIYUN_IOT_COMMON_NET_H

#include "aliyun_iot_network_ssl.h"

typedef struct {
    char *pHostAddress;     ///< Pointer to a host name string
    char *pHostPort;        ///< destination port
    char *pPubKey;          ///< Pointer to a ca string
}ConnectParams;


/**
 * @brief The structure of network connection(TCP or SSL).
 *   The user has to allocate memory for this structure.
 */

struct Network;

typedef struct Network Network_t;
typedef struct Network* pNetwork_t;

struct Network
{
    char *port; //todo to integer.
    char *pHostAddress;
    char *ca_crt;       //TCP connection when the value is NULL; SSL connection when the value is NOT NULL
    intptr_t handle;    //connection handle

    int (*read)(pNetwork_t, char *, uint32_t, uint32_t);      /**< Read data from server function pointer. */
    int (*write)(pNetwork_t, char *, uint32_t, uint32_t);     /**< Send data to server function pointer. */
    int (*disconnect)(pNetwork_t);                  /**< Disconnect the network function pointer.此函数close socket后需要初始化为-1，如果为-1则不再执行close操作*/
    intptr_t (*connect)(pNetwork_t);
};


int aliyun_iot_net_read(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms);
int aliyun_iot_net_write(pNetwork_t pNetwork, char *buffer, uint32_t len, uint32_t timeout_ms);
int aliyun_iot_net_disconnect(pNetwork_t pNetwork);
intptr_t aliyun_iot_net_connect(pNetwork_t pNetwork);
int aliyun_iot_net_init(pNetwork_t pNetwork, char *host, char *port, char *ca_crt);

#endif /* ALIYUN_IOT_COMMON_NET_H */



/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#if defined(COAP_CLIENT) || defined(COAP_SERVER)
#include "infra_config.h"
#include "infra_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <process.h>
#include <ipmib.h>

#define HAL_DEBUG_OUT 1

static wchar_t *gIfName = L"\\DEVICE\\TCPIP_{C9E93150-C6B1-4B15-8A2E-C08747261CD2}";
/*The return value will be bigger than zero if it gets the index successfully*/
static DWORD get_interface_index()
{
    MIB_IFTABLE *pIfTable = NULL;
    MIB_IFROW *pIfRow;
    DWORD dwSize = 0, ifIndex = 0;
    int i;

    if (GetIfTable((MIB_IFTABLE *)&dwSize, &dwSize, 0) != ERROR_INSUFFICIENT_BUFFER) {
        printf("Failed to get if table at step 1\n\r");
        return (0);
    }
    pIfTable = (MIB_IFTABLE *) malloc(dwSize);
    if (pIfTable == NULL) {
        return (0);
    }
    if (GetIfTable((MIB_IFTABLE *)pIfTable, &dwSize, 0) != NO_ERROR) {
        free(pIfTable);
        printf("Failed to get if table\n\r");
        return (0);
    }

    for (i = 0; i < pIfTable->dwNumEntries; i++) {
        pIfRow = (MIB_IFROW *) & pIfTable->table[i];
        if (wcscmp(pIfRow->wszName, gIfName) == 0) {
            ifIndex = pIfRow->dwIndex;
            break;
        }
    }
    if (i >= pIfTable->dwNumEntries) {
        free(pIfTable);
        printf("Failed to find the network interface \n\r");
        return (0);
    }

    free(pIfTable);
    return (ifIndex);
}

static uint32_t get_subnet_bcast()
{
    DWORD ifIndex;
    PMIB_IPADDRTABLE pIPAddrTable = NULL;
    DWORD dwSize = 0;
    int i;
    unsigned int net, snBcast;

    if ((ifIndex = get_interface_index()) == 0) {
        printf("Failed to get IP, please check if the WINDOWS_IF_NAME is correct\n\r");
        return (0);
    }

    if (GetIpAddrTable((MIB_IPADDRTABLE *)&dwSize, &dwSize, 0) != ERROR_INSUFFICIENT_BUFFER) {
        printf("System doesn't have an ip address yet\n\r");
        return (0);
    }
    pIPAddrTable = (MIB_IPADDRTABLE *) malloc(dwSize);
    if (pIPAddrTable == NULL) {
        return (0);
    }
    if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) != NO_ERROR) {
        printf("Failed to get ip addr table\n\r");
        return (0);
    }
    for (i = 0; i < (int) pIPAddrTable->dwNumEntries; i++) {
        if (pIPAddrTable->table[i].dwIndex == ifIndex) {
            net = (unsigned int) pIPAddrTable->table[i].dwAddr & pIPAddrTable->table[i].dwAddr & pIPAddrTable->table[i].dwAddr &
                  pIPAddrTable->table[i].dwMask;
            snBcast = net | (~pIPAddrTable->table[i].dwMask);
            //printf("bcast is %X\n\r", snBcast);

            free(pIPAddrTable);
            return (snBcast);
        }
    }

    free(pIPAddrTable);
    return 0;
}

int HAL_UDP_close_without_connect(intptr_t sockfd)
{
    if (sockfd > 0) {
        closesocket(sockfd);
    }
    return (0);
}

intptr_t HAL_UDP_create_without_connect(const char *host, unsigned short port)
{
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in service;
    BOOL flag;
    int ret, loop;

    if (HAL_DEBUG_OUT) {
        printf("Create UDP: %s %d\n\r", host, (int)port);
    }
    if (s == INVALID_SOCKET) {
        return (-1);
    }
    if (port == 0) { /*doesn't bind port*/
        return (s);
    }

    service.sin_family = AF_INET;
    if (host == NULL) {
        service.sin_addr.s_addr = inet_addr("0.0.0.0");
    } else {
        service.sin_addr.s_addr = inet_addr(host);
    }
    service.sin_port = htons(port);

    if (bind(s, (SOCKADDR *) &service, sizeof(service)) == SOCKET_ERROR) {
        closesocket(s);
        return (-1);
    }

    flag = TRUE;
    ret = setsockopt(s, SOL_SOCKET, SO_BROADCAST, (void *)&flag, sizeof(flag));
    if (ret == SOCKET_ERROR) {
        closesocket(s);
        printf("Failed to allow UDP socket to send broadcast pkts\n\r");
        return -1;
    }

    loop = 0;
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, (void *)&loop, sizeof(loop));
    if (ret == SOCKET_ERROR) {
        closesocket(s);
        printf("Failed to set IP_MULTICAST_LOOP to false\n\r");
        return -1;
    }

    return (s);
}


int HAL_UDP_joinmulticast(intptr_t sockfd,
                          char *p_group)
{
    struct ip_mreq mreq;
    int ret;

    memset(&mreq, 0, sizeof(struct ip_mreq));
    mreq.imr_multiaddr.S_un.S_addr = inet_addr(p_group);  //×é²¥Ô´µØÖ·
    mreq.imr_interface.S_un.S_addr = INADDR_ANY;     //±¾µØµØÖ·
    ret = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char FAR *)&mreq, sizeof(mreq));
    if (ret == SOCKET_ERROR) {
        printf("Failed to join multicast group\n\r");
        return -1;
    }
    return (0);
}


int HAL_UDP_recvfrom(intptr_t sockfd,
                     NetworkAddr *p_remote,
                     unsigned char *p_data,
                     unsigned int datalen,
                     unsigned int timeout_ms)
{
    int ret, errCode;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    fd_set read_fds;
    struct timeval timeout = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};

    FD_ZERO(&read_fds);
    FD_SET(sockfd, &read_fds);

    ret = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
    switch (ret) {
        default:
            break;

        case (0):
            return 0;    /* receive timeout */
            break;

        case (SOCKET_ERROR):
            errCode = WSAGetLastError();
            printf("select error %d\n\r", errCode);
            return (-1);
            break;
    }

    ret = recvfrom(sockfd, (char *)p_data, (int)datalen, 0, (struct sockaddr *)&addr, &addr_len);
    if (ret <= 0) {
        return (-1);
    }
    if (NULL != p_remote) {
        p_remote->port = ntohs(addr.sin_port);
        strcpy((char *)p_remote->addr, inet_ntoa(addr.sin_addr));
    }
    if (HAL_DEBUG_OUT) {
        printf("UDP socket %d rcv %dB\n\r", (int)sockfd, ret);
    }
    return (ret);
}

static uint32_t get_subnet_bcast();

int HAL_UDP_sendto(intptr_t sockfd,
                   const NetworkAddr *p_remote,
                   const unsigned char *p_data,
                   unsigned int datalen,
                   unsigned int timeout_ms)
{
    int ret;
    unsigned long ipAddr;
    struct hostent *hp;
    struct sockaddr_in addr;
    DWORD toVal;
    //struct timeval timeout = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};

    if (HAL_DEBUG_OUT) {
        printf("UDP_sendto %s:%d %d\n\r", p_remote->addr, p_remote->port, datalen);
    }
    if ((ipAddr = inet_addr((char *)p_remote->addr)) == INADDR_NONE) {
        hp = gethostbyname((char *)p_remote->addr);
        if (!hp) {
            printf("can't resolute the host address \n");
            return -1;
        }
        ipAddr = *(uint32_t *)(hp->h_addr);
    }
    if (ipAddr == 0xFFFFFFFF) {
        ipAddr = get_subnet_bcast();
    }

    toVal = timeout_ms;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (void *)&toVal, sizeof(toVal));
    if (ret == SOCKET_ERROR) {
        printf("Failed to set UDP_send timeout\n\r");
        return -1;
    }

    addr.sin_addr.s_addr = ipAddr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(p_remote->port);
    ret = sendto(sockfd, (char *)p_data, (int)datalen, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    if (HAL_DEBUG_OUT) {
        printf("UDP socket %d sent %dB\n\r", (int)sockfd, ret);
    }
    return (ret) > 0 ? ret : -1;
}

uint32_t HAL_Wifi_Get_IP(char ip_str[NETWORK_ADDR_LEN], const char *ifname)
{
    DWORD ifIndex;
    PMIB_IPADDRTABLE pIPAddrTable = NULL;
    DWORD dwSize = 0;
    IN_ADDR IPAddr;
    int i;

    ip_str[0] = 0;
    if ((ifIndex = get_interface_index()) == 0) {
        printf("Failed to get IP, please check if the WINDOWS_IF_NAME is correct\n\r");
        return (0);
    }

    if (GetIpAddrTable((MIB_IPADDRTABLE *)&dwSize, &dwSize, 0) != ERROR_INSUFFICIENT_BUFFER) {
        printf("System doesn't have an ip address yet\n\r");
        return (0);
    }
    pIPAddrTable = (MIB_IPADDRTABLE *) malloc(dwSize);
    if (pIPAddrTable == NULL) {
        return (0);
    }
    if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) != NO_ERROR) {
        printf("Failed to get ip addr table\n\r");
        return (0);
    }
    for (i = 0; i < (int) pIPAddrTable->dwNumEntries; i++) {
        if (pIPAddrTable->table[i].dwIndex == ifIndex) {
            IPAddr.S_un.S_addr = (u_long) pIPAddrTable->table[i].dwAddr;
            snprintf(ip_str, NETWORK_ADDR_LEN, "%s", inet_ntoa(IPAddr));
            if (HAL_DEBUG_OUT) {
                printf("WIFI IP address is %s\n\r", ip_str);
            }
            free(pIPAddrTable);
            return (IPAddr.S_un.S_addr);
        }
    }

    free(pIPAddrTable);
    return 0;
}
#endif  /* #if defined(HAL_UDP) */




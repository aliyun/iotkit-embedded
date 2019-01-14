/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef _AT_CONN_MGMT_H_
#define _AT_CONN_MGMT_H_

enum netconn_type {
    NETCONN_INVALID  = 0,
    /** TCP IPv4 */
    NETCONN_TCP,
    NETCONN_TYPE_NUM
};

int at_conn_init(void);

int at_conn_recv(int connid, void *mem, uint32_t len);


int at_conn_getaddrinfo(const char *nodename, char resultip[16]);


int at_conn_setup(int type);


int at_conn_start(int connid, char* remoteipaddr, uint16_t remoteport);


int at_conn_close(int connid);


int at_conn_recvbufempty(int connid);


int at_conn_send(int connid, const void *data, uint32_t size);


int at_conn_recv(int connid, void *mem, uint32_t len);

#endif
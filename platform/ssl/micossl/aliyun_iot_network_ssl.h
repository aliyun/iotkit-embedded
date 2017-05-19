#ifndef ALIYUN_IOT_NETWORK_SSL_H
#define ALIYUN_IOT_NETWORK_SSL_H

#include <string.h>

#include "../../../platform/os/mico/inc/aliyun_iot_platform_network.h"
#include "mico_socket.h"
#include "aliyun_iot_common_log.h"

typedef struct _TLSDataParams {
	mico_ssl_t ssl;
	int socketId;
}TLSDataParams;

int aliyun_iot_network_ssl_read(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms);

int aliyun_iot_network_ssl_write(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms);

void aliyun_iot_network_ssl_disconnect(TLSDataParams *pTlsData);

int aliyun_iot_network_ssl_connect(TLSDataParams *pTlsData, const char *addr, const char *port, const char *ca_crt, size_t ca_crt_len);

#endif

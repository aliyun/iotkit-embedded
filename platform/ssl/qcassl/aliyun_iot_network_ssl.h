#ifndef ALIYUN_IOT_NETWORK_SSL_H
#define ALIYUN_IOT_NETWORK_SSL_H

#include "stdio.h"

#include "aliyun_iot_platform_memory.h"
#include "aliyun_iot_platform_network.h"
#include "qcom/base.h"
#include "qcom/socket_api.h"
#include "qcom/qcom_ssl.h"
#include "qcom/socket.h"
#include "qcom/qcom_misc.h"
#include "aliyun_iot_common_log.h"

typedef struct _TLSDataParams {
    SSL *ssl;
	SSL_CTX	*ssl_ctx;
	SSL_CONFIG	*ssl_cfg;
	int socketId;
}TLSDataParams;

int aliyun_iot_network_ssl_read(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms);

int aliyun_iot_network_ssl_write(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms);

void aliyun_iot_network_ssl_disconnect(TLSDataParams *pTlsData);

int aliyun_iot_network_ssl_connect(TLSDataParams *pTlsData, const char *addr, const char *port, const char *ca_crt, size_t ca_crt_len);

#endif

#ifndef ALIYUN_IOT_NETWORK_SSL_H
#define ALIYUN_IOT_NETWORK_SSL_H

#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_error.h"

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "aliyun_iot_platform_network.h"


typedef struct _TLSDataParams {
    SSL *pssl;
    int socketId;
} TLSDataParams;

int aliyun_iot_network_ssl_read(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms);

int aliyun_iot_network_ssl_write(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms);

void aliyun_iot_network_ssl_disconnect(TLSDataParams *pTlsData);

int aliyun_iot_network_ssl_connect(TLSDataParams *pTlsData, const char *addr, const char *port, const char *ca_crt,
                                   size_t ca_crt_len);

#endif

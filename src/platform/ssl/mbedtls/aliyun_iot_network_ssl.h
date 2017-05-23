#ifndef ALIYUN_IOT_NETWORK_SSL_H
#define ALIYUN_IOT_NETWORK_SSL_H

#include <string.h>
#include "mbedtls/ssl.h"
#include "mbedtls/net.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/debug.h"

typedef struct _TLSDataParams {
    mbedtls_ssl_context ssl;          /**< mbed TLS control context. */
    mbedtls_net_context fd;           /**< mbed TLS network context. */
    mbedtls_ssl_config conf;          /**< mbed TLS configuration context. */
    mbedtls_x509_crt cacertl;         /**< mbed TLS CA certification. */
    mbedtls_x509_crt clicert;         /**< mbed TLS Client certification. */
    mbedtls_pk_context pkey;          /**< mbed TLS Client key. */
}TLSDataParams;

int aliyun_iot_network_ssl_read(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms);

int aliyun_iot_network_ssl_write(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms);

void aliyun_iot_network_ssl_disconnect(TLSDataParams *pTlsData);

int aliyun_iot_network_ssl_connect(TLSDataParams *pTlsData, const char *addr, const char *port, const char *ca_crt, size_t ca_crt_len);

#endif

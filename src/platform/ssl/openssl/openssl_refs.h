#ifndef ALIYUN_IOT_NETWORK_SSL_H
#define ALIYUN_IOT_NETWORK_SSL_H

#include "lite/lite-log.h"
#include "utils_error.h"

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "platform_network.h"


typedef struct _TLSDataParams {
    SSL *pssl;
    int socketId;
} TLSDataParams_t;

int utils_network_ssl_read(TLSDataParams_t *pTlsData, char *buffer, int len, int timeout_ms);

int utils_network_ssl_write(TLSDataParams_t *pTlsData, const char *buffer, int len, int timeout_ms);

void utils_network_ssl_disconnect(TLSDataParams_t *pTlsData);

int utils_network_ssl_connect(TLSDataParams_t *pTlsData, const char *addr, const char *port, const char *ca_crt,
                              size_t ca_crt_len);

#endif

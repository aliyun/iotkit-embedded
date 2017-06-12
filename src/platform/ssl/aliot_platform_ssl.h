

#ifndef _ALIOT_PLATFORM_SSL_H_
#define _ALIOT_PLATFORM_SSL_H_

#include "aliot_platform.h"

int aliot_platform_ssl_read(void *handle, char *buf, int len, int timeout_ms);

int aliot_platform_ssl_write(void *handle, const char *buf, int len, int timeout_ms);

void aliot_platform_ssl_disconnect(void *handle);

void *aliot_platform_ssl_connect(
                const char *host,
                uint16_t port,
                const char *ca_crt,
                size_t ca_crt_len);

#endif /* _ALIOT_PLATFORM_SSL_H_ */

#include "CoAPPlatform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/platform.h"
#include "mbedtls/sha256.h"
#include "mbedtls/debug.h"
#include "mbedtls/timing.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ssl_cookie.h"


#ifndef __COAP_DTLS_H__
#define __COAP_DTLS_H__

#define TRANSPORT_ADDR_LEN 16

#define DTLS_TRC   COAP_TRC
#define DTLS_DUMP  COAP_DUMP
#define DTLS_DEBUG COAP_DEBUG
#define DTLS_INFO  COAP_INFO
#define DTLS_ERR   COAP_ERR

typedef  int (*coap_dtls_send_t)(void *socket_id,
                                 unsigned char  *p_data,
                                 size_t          datalen);


typedef  int (*coap_dtls_recv_t)(void *socket_id,
                                 unsigned char   *p_data,
                                 size_t           datalen);

typedef  int (*coap_dtls_recv_timeout_t)(void *socket_id,
                              unsigned char   *p_data,
                              size_t           datalen);


typedef struct
{
    int               socket_id;
    unsigned char     remote_addr[TRANSPORT_ADDR_LEN];
    unsigned short    remote_port;
} dtls_network_t;


typedef struct
{
    dtls_network_t            network;
    coap_dtls_send_t          send_fn;
    coap_dtls_recv_t          recv_fn;
    coap_dtls_recv_timeout_t  recv_timeout_fn;
    unsigned char             *p_ca_cert_pem;
} coap_dtls_options_t;


typedef void DTLSContext;

DTLSContext *DTLSSession_init();

unsigned int DTLSSession_create(DTLSContext *conetxt, coap_dtls_options_t  *p_options);

unsigned int DTLSSession_write(DTLSContext *conetxt,
                                unsigned char   *p_data,
                                unsigned int    *p_datalen);


unsigned int DTLSSession_read(DTLSContext *conetxt,
                               unsigned char   *p_data,
                               unsigned int    *p_datalen);

unsigned int DTLSSession_free(DTLSContext *conetxt);


#endif

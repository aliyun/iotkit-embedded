/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "infra_config.h"
#ifdef COAP_DTLS_SUPPORT
#include "wrappers_defs.h"
#include "mbedtls/ssl.h"
#include "mbedtls/platform.h"
#include "mbedtls/sha256.h"
#include "mbedtls/debug.h"
#include "mbedtls/timing.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/net_sockets.h"
#include "wrappers.h"

#include "infra_sha256.h"
#include "infra_string.h"

#define DTLS_AUTH_MODE_CA        0
#define DTLS_AUTH_MODE_PSK       1

#ifndef DTLS_AUTH_MODE
/* choose dtls auth mode here */
#define DTLS_AUTH_MODE           DTLS_AUTH_MODE_PSK
#endif

#define DTLS_TRC(...)    HAL_Printf("[trc] "), HAL_Printf(__VA_ARGS__)
#define DTLS_DUMP(...)   HAL_Printf("[dump] "), HAL_Printf(__VA_ARGS__)
#define DTLS_DEBUG(...)  HAL_Printf("[dbg] "), HAL_Printf(__VA_ARGS__)
#define DTLS_INFO(...)   HAL_Printf("[inf] "), HAL_Printf(__VA_ARGS__)
#define DTLS_ERR(...)    HAL_Printf("[err] "), HAL_Printf(__VA_ARGS__)

#ifdef DTLS_SESSION_SAVE
    mbedtls_ssl_session *saved_session = NULL;
#endif

typedef struct {
    mbedtls_ssl_context          context;
    mbedtls_ssl_config           conf;
    mbedtls_ctr_drbg_context     ctr_drbg;
    mbedtls_entropy_context      entropy;
#if (DTLS_AUTH_MODE == DTLS_AUTH_MODE_CA)
    mbedtls_x509_crt             cacert;
#endif
    mbedtls_net_context          fd;
    mbedtls_timing_delay_context timer;
} dtls_session_t;

typedef struct {
    void *(*malloc)(uint32_t size);
    void (*free)(void *ptr);
} dtls_hooks_t;

/* define MBEDTLS_MEM_STATS to enable dtls memery stats */
#ifdef MBEDTLS_MEM_STATS
#define MBEDTLS_MEM_INFO_MAGIC   0x12345678

static unsigned int mbedtls_mem_used = 0;
static unsigned int mbedtls_max_mem_used = 0;
static dtls_hooks_t g_dtls_hooks = {HAL_Malloc, HAL_Free};

typedef struct {
    int magic;
    int size;
} mbedtls_mem_info_t;

static void *_DTLSMalloc_wrapper(uint32_t size)
{
    return HAL_Malloc(size);

}

static void *_DTLSCalloc_wrapper(size_t n, size_t size)
{
    void *buf = NULL;
    mbedtls_mem_info_t *mem_info = NULL;

    if (n == 0 || size == 0) {
        return NULL;
    }

    buf = g_dtls_hooks.malloc(n * size + sizeof(mbedtls_mem_info_t));
    if (NULL == buf) {
        return NULL;
    } else {
        memset(buf, 0, n * size + sizeof(mbedtls_mem_info_t));
    }

    mem_info = (mbedtls_mem_info_t *)buf;
    mem_info->magic = MBEDTLS_MEM_INFO_MAGIC;
    mem_info->size = n * size;
    buf += sizeof(mbedtls_mem_info_t);

    mbedtls_mem_used += mem_info->size;
    if (mbedtls_mem_used > mbedtls_max_mem_used) {
        mbedtls_max_mem_used = mbedtls_mem_used;
    }

    return buf;
}

void _DTLSFree_wrapper(void *ptr)
{
    mbedtls_mem_info_t *mem_info = NULL;
    if (NULL == ptr) {
        return;
    }

    mem_info = ptr - sizeof(mbedtls_mem_info_t);
    if (mem_info->magic != MBEDTLS_MEM_INFO_MAGIC) {
        DTLS_TRC("Warning - invalid mem info magic: 0x%x\r\n", mem_info->magic);
        return;
    }
    mbedtls_mem_used -= mem_info->size;

    g_dtls_hooks.free(mem_info);
}
#else /* #ifdef MBEDTLS_MEM_STATS */
static  void *_DTLSCalloc_wrapper(size_t n, size_t s)
{
    void *ptr = NULL;
    size_t len = n * s;
    ptr = HAL_Malloc(len);
    if (NULL != ptr) {
        memset(ptr, 0x00, len);
    }
    return ptr;
}

static  void _DTLSFree_wrapper(void *ptr)
{
    if (NULL != ptr) {
        HAL_Free(ptr);
        ptr = NULL;
    }
}
#endif /* #ifdef MBEDTLS_MEM_STATS */

#ifdef DTLS_SESSION_SAVE
static int _DTLSSession_save(const mbedtls_ssl_session *session,
                             unsigned char *buf, size_t buf_len,
                             size_t *olen)
{
    unsigned char *p = buf;
    size_t left = buf_len;
#if (DTLS_AUTH_MODE == DTLS_AUTH_MODE_CA)
    size_t cert_len;
#endif /* #if (DTLS_AUTH_MODE == DTLS_AUTH_MODE_CA) */

    if (left < sizeof(mbedtls_ssl_session)) {
        return (MBEDTLS_ERR_SSL_BUFFER_TOO_SMALL);
    }

    memcpy(p, session, sizeof(mbedtls_ssl_session));
    p += sizeof(mbedtls_ssl_session);
    left -= sizeof(mbedtls_ssl_session);

#if (DTLS_AUTH_MODE == DTLS_AUTH_MODE_CA)
    if (session->peer_cert == NULL) {
        cert_len = 0;
    } else {
        cert_len = session->peer_cert->raw.len;
    }

    if (left < 3 + cert_len) {
        return (MBEDTLS_ERR_SSL_BUFFER_TOO_SMALL);
    }

    *p++ = (unsigned char)(cert_len >> 16 & 0xFF);
    *p++ = (unsigned char)(cert_len >>  8 & 0xFF);
    *p++ = (unsigned char)(cert_len       & 0xFF);

    if (session->peer_cert != NULL) {
        memcpy(p, session->peer_cert->raw.p, cert_len);
    }

    p += cert_len;
#endif /* #if (DTLS_AUTH_MODE == DTLS_AUTH_MODE_CA) */

    *olen = p - buf;

    return (0);
}
#endif /* #ifdef DTLS_SESSION_SAVE */

static unsigned int _DTLSVerifyOptions_set(dtls_session_t *p_dtls_session, unsigned char *p_ca_cert_pem, char *host)
{
    int result;
    unsigned int err_code = DTLS_SUCCESS;

#if (DTLS_AUTH_MODE == DTLS_AUTH_MODE_CA)
    if (p_ca_cert_pem != NULL) {
        mbedtls_ssl_conf_authmode(&p_dtls_session->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        if (strstr(host, "pre.iot-as-coap")) {
            DTLS_TRC("host = '%s' so verify server OPTIONAL\r\n", host);
            mbedtls_ssl_conf_authmode(&p_dtls_session->conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
        }
        DTLS_TRC("Call mbedtls_ssl_conf_authmode\r\n");

        DTLS_TRC("x509 ca cert pem len %d\r\n%s\r\n", (int)strlen((char *)p_ca_cert_pem) + 1, p_ca_cert_pem);
        result = mbedtls_x509_crt_parse(&p_dtls_session->cacert,
                                        p_ca_cert_pem,
                                        strlen((const char *)p_ca_cert_pem) + 1);

        DTLS_TRC("mbedtls_x509_crt_parse result 0x%04x\r\n", result);
        if (0 != result) {
            err_code = DTLS_INVALID_CA_CERTIFICATE;
        } else {
            mbedtls_ssl_conf_ca_chain(&p_dtls_session->conf, &p_dtls_session->cacert, NULL);
        }
    }
#else
    {
        static const int ciphersuites[1] = {MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA};
        char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
        char device_name[IOTX_DEVICE_NAME_LEN + 1] = {0};
        char device_secret[IOTX_DEVICE_SECRET_LEN + 1] = {0};
        char *auth_type = "devicename";
        char *sign_method = "hmacsha256";
        char *timestamp = "2524608000000";
        char *psk_identity = NULL, string_to_sign[IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 33] = {0};
        uint32_t psk_identity_len = 0;
        uint8_t sign_hex[32] = {0};
        char sign_string[65] = {0};

        IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, product_key);
        IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_NAME, device_name);
        IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_SECRET, device_secret);

        /* psk identity length */
        psk_identity_len = strlen(auth_type) + strlen(sign_method) + strlen(product_key) + strlen(device_name) + strlen(
                                       timestamp) + 5;
        psk_identity = HAL_Malloc(psk_identity_len);
        if (psk_identity == NULL) {
            printf("psk_identity malloc failed\n");
            return -1;
        }
        memset(psk_identity, 0, psk_identity_len);
        memcpy(psk_identity, auth_type, strlen(auth_type));
        memcpy(psk_identity + strlen(psk_identity), "|", strlen("|"));
        memcpy(psk_identity + strlen(psk_identity), sign_method, strlen(sign_method));
        memcpy(psk_identity + strlen(psk_identity), "|", strlen("|"));
        memcpy(psk_identity + strlen(psk_identity), product_key, strlen(product_key));
        memcpy(psk_identity + strlen(psk_identity), "&", strlen("&"));
        memcpy(psk_identity + strlen(psk_identity), device_name, strlen(device_name));
        memcpy(psk_identity + strlen(psk_identity), "|", strlen("|"));
        memcpy(psk_identity + strlen(psk_identity), timestamp, strlen(timestamp));

        /* string to sign */
        memcpy(string_to_sign, "id", strlen("id"));
        memcpy(string_to_sign + strlen(string_to_sign), product_key, strlen(product_key));
        memcpy(string_to_sign + strlen(string_to_sign), "&", strlen("&"));
        memcpy(string_to_sign + strlen(string_to_sign), device_name, strlen(device_name));
        memcpy(string_to_sign + strlen(string_to_sign), "timestamp", strlen("timestamp"));
        memcpy(string_to_sign + strlen(string_to_sign), timestamp, strlen(timestamp));

        utils_hmac_sha256((uint8_t *)string_to_sign, strlen(string_to_sign), (uint8_t *)device_secret,
                          strlen(device_secret), sign_hex);
        infra_hex2str(sign_hex, 32, sign_string);

        /*
        printf("psk_identity: %s\n",psk_identity);
        printf("psk         : %s\n",sign_string);
        */

        mbedtls_ssl_conf_psk(&(p_dtls_session->conf), (const unsigned char *)sign_string, strlen(sign_string),
                             (const unsigned char *)psk_identity, strlen(psk_identity));
        mbedtls_ssl_conf_ciphersuites(&(p_dtls_session->conf), ciphersuites);

        HAL_Free(psk_identity);

        printf("mbedtls psk config finished\n");
    }
#endif 

    return err_code;
}

static void _DTLSLog_wrapper(void        *p_ctx, int level,
                             const char *p_file, int line,   const char *p_str)
{
    DTLS_INFO("[mbedTLS]:[%s]:[%d]: %s\r\n", p_file, line, p_str);
}

static unsigned int _DTLSContext_setup(dtls_session_t *p_dtls_session, coap_dtls_options_t *p_options)
{
    int   result = 0;

    mbedtls_ssl_init(&p_dtls_session->context);

    result = mbedtls_ssl_setup(&p_dtls_session->context, &p_dtls_session->conf);
    DTLS_TRC("mbedtls_ssl_setup result 0x%04x\r\n", result);

    if (result == 0) {
        if (p_dtls_session->conf.transport == MBEDTLS_SSL_TRANSPORT_DATAGRAM) {
            mbedtls_ssl_set_timer_cb(&p_dtls_session->context,
                                     (void *)&p_dtls_session->timer,
                                     mbedtls_timing_set_delay,
                                     mbedtls_timing_get_delay);
        }

#if (DTLS_AUTH_MODE == DTLS_AUTH_MODE_CA)
        DTLS_TRC("mbedtls_ssl_set_hostname %s\r\n", p_options->p_host);
        mbedtls_ssl_set_hostname(&p_dtls_session->context, p_options->p_host);
#endif /* #if (DTLS_AUTH_MODE == DTLS_AUTH_MODE_CA) */
        mbedtls_ssl_set_bio(&p_dtls_session->context,
                            (void *)&p_dtls_session->fd,
                            mbedtls_net_send,
                            mbedtls_net_recv,
                            mbedtls_net_recv_timeout);
        DTLS_TRC("mbedtls_ssl_set_bio result 0x%04x\r\n", result);

#ifdef DTLS_SESSION_SAVE
        if (NULL != saved_session) {
            result = mbedtls_ssl_set_session(&p_dtls_session->context, saved_session);
            DTLS_TRC("mbedtls_ssl_set_session return 0x%04x\r\n", result);
        }
#endif

        do {
            result = mbedtls_ssl_handshake(&p_dtls_session->context);
        } while (result == MBEDTLS_ERR_SSL_WANT_READ ||
                 result == MBEDTLS_ERR_SSL_WANT_WRITE);
        DTLS_TRC("mbedtls_ssl_handshake result 0x%04x\r\n", result);
#ifdef MBEDTLS_MEM_STATS
        DTLS_TRC("mbedtls handshake memory total used: %d  max used: %d\r\n",
                 mbedtls_mem_used, mbedtls_max_mem_used);
#endif

#ifdef DTLS_SESSION_SAVE
        if (0 == result) {
            if (NULL == saved_session) {
                saved_session = HAL_Malloc(sizeof(mbedtls_ssl_session));
            }
            if (NULL != saved_session) {
                memset(saved_session, 0x00, sizeof(mbedtls_ssl_session));
                result = mbedtls_ssl_get_session(&p_dtls_session->context, saved_session);
                DTLS_TRC("mbedtls_ssl_get_session return 0x%04x\r\n", result);
            }
        }
#endif
    }

    return (result ? DTLS_HANDSHAKE_FAILED : DTLS_SUCCESS);
}

dtls_session_t *_DTLSSession_init()
{
    dtls_session_t *p_dtls_session = NULL;
    p_dtls_session = HAL_Malloc(sizeof(dtls_session_t));

    mbedtls_debug_set_threshold(0);
#ifdef MBEDTLS_MEM_STATS
    mbedtls_mem_used = 0;
    mbedtls_max_mem_used = 0;
#endif
    mbedtls_platform_set_calloc_free(_DTLSCalloc_wrapper, _DTLSFree_wrapper);
    if (NULL != p_dtls_session) {
        mbedtls_net_init(&p_dtls_session->fd);
        mbedtls_ssl_init(&p_dtls_session->context);
        mbedtls_ssl_config_init(&p_dtls_session->conf);

#if (DTLS_AUTH_MODE == DTLS_AUTH_MODE_CA)
        mbedtls_x509_crt_init(&p_dtls_session->cacert);
#endif /* #if (DTLS_AUTH_MODE == DTLS_AUTH_MODE_CA) */
        mbedtls_ctr_drbg_init(&p_dtls_session->ctr_drbg);
        mbedtls_entropy_init(&p_dtls_session->entropy);
        DTLS_INFO("HAL_DTLSSession_init success\r\n");

    }

    return p_dtls_session;
}

unsigned int _DTLSSession_deinit(dtls_session_t *p_dtls_session)
{
    int ret;
    if (p_dtls_session != NULL) {
        do {
            ret = mbedtls_ssl_close_notify(&p_dtls_session->context);
        } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);

        mbedtls_net_free(&p_dtls_session->fd);
#if (DTLS_AUTH_MODE == DTLS_AUTH_MODE_CA)
        mbedtls_x509_crt_free(&p_dtls_session->cacert);
#endif
        mbedtls_ssl_config_free(&p_dtls_session->conf);
        mbedtls_ssl_free(&p_dtls_session->context);

        mbedtls_ctr_drbg_free(&p_dtls_session->ctr_drbg);
        mbedtls_entropy_free(&p_dtls_session->entropy);
        HAL_Free(p_dtls_session);
    }

    return DTLS_SUCCESS;
}

DTLSContext *HAL_DTLSSession_create(coap_dtls_options_t *p_options)
{
    char port[6] = {0};
    int result = 0;
    dtls_session_t *p_dtls_session = NULL;

    p_dtls_session = _DTLSSession_init();
    if (NULL != p_dtls_session) {
        mbedtls_ssl_config_init(&p_dtls_session->conf);
        result = mbedtls_ctr_drbg_seed(&p_dtls_session->ctr_drbg, mbedtls_entropy_func, &p_dtls_session->entropy,
                                       (const unsigned char *)"IoTx",
                                       strlen("IoTx"));
        if (0 !=  result) {
            DTLS_ERR("mbedtls_ctr_drbg_seed result 0x%04x\r\n", result);
            goto error;
        }
        result = mbedtls_ssl_config_defaults(&p_dtls_session->conf,
                                             MBEDTLS_SSL_IS_CLIENT,
                                             MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                             MBEDTLS_SSL_PRESET_DEFAULT);
        if (0 != result) {
            DTLS_ERR("mbedtls_ssl_config_defaults result 0x%04x\r\n", result);
            goto error;
        }
        mbedtls_ssl_conf_rng(&p_dtls_session->conf, mbedtls_ctr_drbg_random, &p_dtls_session->ctr_drbg);
        mbedtls_ssl_conf_dbg(&p_dtls_session->conf, _DTLSLog_wrapper, NULL);

        result = _DTLSVerifyOptions_set(p_dtls_session, p_options->p_ca_cert_pem, p_options->p_host);
        if (DTLS_SUCCESS != result) {
            DTLS_ERR("DTLSVerifyOptions_set result 0x%04x\r\n", result);
            goto error;
        }
        sprintf(port, "%u", p_options->port);
        result = mbedtls_net_connect(&p_dtls_session->fd, p_options->p_host,
                                     port, MBEDTLS_NET_PROTO_UDP);
        if (0 != result) {
            DTLS_ERR("mbedtls_net_connect result 0x%04x\r\n", result);
            goto error;
        }

#ifdef MBEDTLS_SSL_PROTO_DTLS
        if (p_dtls_session->conf.transport == MBEDTLS_SSL_TRANSPORT_DATAGRAM) {
            mbedtls_ssl_conf_min_version(&p_dtls_session->conf,
                                         MBEDTLS_SSL_MAJOR_VERSION_3,
                                         MBEDTLS_SSL_MINOR_VERSION_3);

            mbedtls_ssl_conf_max_version(&p_dtls_session->conf,
                                         MBEDTLS_SSL_MAJOR_VERSION_3,
                                         MBEDTLS_SSL_MINOR_VERSION_3);

            mbedtls_ssl_conf_handshake_timeout(&p_dtls_session->conf,
                                               (MBEDTLS_SSL_DTLS_TIMEOUT_DFL_MIN * 2),
                                               (MBEDTLS_SSL_DTLS_TIMEOUT_DFL_MIN * 2 * 4));
        }
#endif
        result = _DTLSContext_setup(p_dtls_session, p_options);
        if (DTLS_SUCCESS != result) {
            DTLS_ERR("DTLSVerifyOptions_set result 0x%04x\r\n", result);
            goto error;
        }

        return (DTLSContext *)p_dtls_session;
    }

error:
    if (NULL != p_dtls_session) {
        _DTLSSession_deinit(p_dtls_session);
    }
    return NULL;
}

unsigned int HAL_DTLSSession_write(DTLSContext *context,
                                   const unsigned char *p_data,
                                   unsigned int *p_datalen)
{
    int len  = 0;
    unsigned int err_code = DTLS_SUCCESS;
    dtls_session_t *p_dtls_session = (dtls_session_t *)context;

    if (NULL != p_dtls_session && NULL != p_data && p_datalen != NULL) {
        len = (*p_datalen);
        len = mbedtls_ssl_write(&p_dtls_session->context, p_data, len);

        if (len < 0) {
            if (len == MBEDTLS_ERR_SSL_CONN_EOF) {
                if (p_dtls_session->context.state < MBEDTLS_SSL_HANDSHAKE_OVER) {
                    err_code = DTLS_HANDSHAKE_IN_PROGRESS;
                }
            }
        } else {
            (*p_datalen) = len;
            err_code      = DTLS_SUCCESS;
        }
    }

    return err_code;
}

unsigned int HAL_DTLSSession_read(DTLSContext *context,
                                  unsigned char   *p_data,
                                  unsigned int    *p_datalen,
                                  unsigned int     timeout)
{
    int len = 0;
    unsigned int err_code = DTLS_READ_DATA_FAILED;
    dtls_session_t *p_dtls_session = (dtls_session_t *)context;

    if (NULL != p_dtls_session && NULL != p_data && p_datalen != NULL) {
        mbedtls_ssl_conf_read_timeout(&(p_dtls_session->conf), timeout);
        len = mbedtls_ssl_read(&p_dtls_session->context, p_data, *p_datalen);

        if (0  <  len) {
            *p_datalen = len;
            err_code = DTLS_SUCCESS;
            DTLS_TRC("mbedtls_ssl_read len %d bytes\r\n", len);
        } else {
            *p_datalen = 0;
            if (MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE == len) {
                err_code = DTLS_FATAL_ALERT_MESSAGE;
                DTLS_INFO("Recv peer fatal alert message\r\n");
            } else if (MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY == len) {
                err_code = DTLS_PEER_CLOSE_NOTIFY;
                DTLS_INFO("The DTLS session was closed by peer\r\n");
            } else if (MBEDTLS_ERR_SSL_TIMEOUT == len) {
                err_code = DTLS_SUCCESS;
                DTLS_TRC("DTLS recv timeout\r\n");
            } else {
                DTLS_TRC("mbedtls_ssl_read error result (-0x%04x)\r\n", len);
            }
        }
    }
    return err_code;
}

unsigned int HAL_DTLSSession_free(DTLSContext *context)
{
    dtls_session_t *p_dtls_session = NULL;
    if (NULL != context) {
        p_dtls_session = (dtls_session_t *)context;
        return _DTLSSession_deinit(p_dtls_session);
    }

    return DTLS_INVALID_PARAM;
}

#endif

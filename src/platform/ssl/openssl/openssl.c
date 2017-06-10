#include "aliyun_iot_network_ssl.h"

static SSL_CTX *ssl_ctx = NULL;
static X509_STORE *ca_store = NULL;
static X509 *ca = NULL;

static X509 *ssl_load_cert(const char *cert_str)
{
    X509 *cert = NULL;
    BIO *in = NULL;

    if (!cert_str) {
        return NULL;
    }

    in = BIO_new_mem_buf((void *)cert_str, -1);
    if (!in) {
        WRITE_IOT_ERROR_LOG("openssl malloc cert buffer failed");
        return NULL;
    }

    cert = PEM_read_bio_X509(in, NULL, NULL, NULL);
    if (in) {
        //WRITE_IOT_ERROR_LOG("openssl read pem failed");
        BIO_free(in);
        //in = NULL;
    }

    return cert;
}


static int ssl_ca_store_init(const char *my_ca)
{
    if (!ca_store) {
        if (!my_ca) {
            WRITE_IOT_ERROR_LOG("no global ca string provided \n");
            return -1;
        }

        ca_store = X509_STORE_new();
        ca = ssl_load_cert(my_ca);
        int ret = X509_STORE_add_cert(ca_store, ca);
        if (ret != 1) {
            WRITE_IOT_ERROR_LOG("failed to X509_STORE_add_cert ret = %d \n", ret);
            return -1;
        }
    }

    return 0;
}

//extern int OPENSSL_init_ssl();
static int ssl_init(const char *my_ca)
{
    if (ssl_ca_store_init(my_ca) != 0) {
        return -1;
    }

    if (!ssl_ctx) {
        const SSL_METHOD *meth;

        SSLeay_add_ssl_algorithms();

        meth = TLSv1_2_client_method();

        SSL_load_error_strings();
        ssl_ctx = SSL_CTX_new(meth);
        if (!ssl_ctx) {
            WRITE_IOT_ERROR_LOG("fail to initialize ssl context \n");
            return -1;
        }
    } else {
        WRITE_IOT_ERROR_LOG("ssl context already initialized \n");
    }

    return 0;
}


static int ssl_establish(int sock, SSL **ppssl)
{
    int err;
    SSL *ssl_temp = NULL;
    X509 *server_cert = NULL;

    if (!ssl_ctx) {
        WRITE_IOT_ERROR_LOG("no ssl context to create ssl connection \n");
        return -1;
    }

    ssl_temp = SSL_new(ssl_ctx);

    SSL_set_fd(ssl_temp, sock);
    err = SSL_connect(ssl_temp);
    if (err == -1) {
        WRITE_IOT_ERROR_LOG("failed create ssl connection \n");
        goto err;
    }

    WRITE_IOT_ERROR_LOG("success to verify cert \n");

    *ppssl = ssl_temp;

    return 0;

err:
    if (ssl_temp) {
        SSL_free(ssl_temp);
    }

    if (server_cert) {
        X509_free(server_cert);
    }

    *ppssl = NULL;
    return -1;
}



SSL *platform_ssl_connect(int tcp_fd, const char *server_cert, int server_cert_len)
{
    SSL *pssl;

    if (0 != ssl_init(server_cert)) {
        return NULL;
    }

    if (0 != ssl_establish(tcp_fd, &pssl)) {
        return NULL;
    }

    return pssl;
}


int platform_ssl_close(SSL *ssl)
{
    if (ssl) {
        SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
        SSL_free(ssl);
        ssl = NULL;
    }

    if (ssl_ctx) {
        SSL_CTX_free(ssl_ctx);
        ssl_ctx = NULL;
    }

    if (ca) {
        X509_free(ca);
        ca = NULL;
    }

    if (ca_store) {
        X509_STORE_free(ca_store);
        ca_store = NULL;
    }

    return 0;
}

int aliyun_iot_network_ssl_read(TLSDataParams_t *pTlsData, unsigned char *buffer, int len, int timeout_ms)
{
    int readLen = 0;
    int ret = -1;

    ALIOT_LOG_DEBUG("SSL_read len=%d timer=%d ms", len, timeout_ms);

    if (!SSL_pending(pTlsData->pssl)) {
        IOT_NET_FD_ISSET_E result;
        ret = aliyun_iot_network_select(pTlsData->socketId, IOT_NET_TRANS_RECV, timeout_ms, &result);
        if (ret < 0) {
            INT32 err = aliyun_iot_get_errno();
            if (err == EINTR_IOT) {
                ALIOT_LOG_DEBUG("continue");
            } else {
                WRITE_IOT_ERROR_LOG("read(select) fail ret=%d", ret);
                return -1;
            }

        } else if (ret == 0) {
            WRITE_IOT_ERROR_LOG("read(select) timeout");
            return -2;
        } else if (ret == 1) {
            ALIOT_LOG_DEBUG("start to read packet");
        }
    }

    while (readLen < len) {
        ret = SSL_read(pTlsData->pssl, (unsigned char *)(buffer + readLen), (len - readLen));
        if (ret > 0) {
            readLen += ret;
        } else if (ret == 0) {
            WRITE_IOT_ERROR_LOG("SSL_read read timeout");
            return -2; //eof
        } else {
            WRITE_IOT_ERROR_LOG("SSL_read error ret = -0x%x", ret);
            return -1; //Connnection error
        }
    }

    //ALIOT_LOG_DEBUG("SSL_read readlen=%d", readLen);
    return readLen;
}


int aliyun_iot_network_ssl_write(TLSDataParams_t *pTlsData, unsigned char *buffer, int len, int timeout_ms)
{
    int writtenLen = 0;
    int ret = 0;

    ALIOT_LOG_DEBUG("SSL_write len=%d timer=%d", len, timeout_ms);
    while (writtenLen < len) {
        ret = SSL_write(pTlsData->pssl, (unsigned char *)(buffer + writtenLen), (len - writtenLen));
        if (ret > 0) {
            writtenLen += ret;
            continue;
        } else if (ret == 0) {
            WRITE_IOT_ERROR_LOG("SSL_write write timeout");
            return writtenLen;
        } else {
            WRITE_IOT_ERROR_LOG("SSL_write write fail ret %d", ret);
            return -1; //Connnection error
        }
    }
    ALIOT_LOG_DEBUG("SSL_write write len=%d", writtenLen);
    return writtenLen;
}

void aliyun_iot_network_ssl_disconnect(TLSDataParams_t *pTlsData)
{
    if (pTlsData->pssl != NULL) {
        platform_ssl_close(pTlsData->pssl);
        pTlsData->pssl = NULL;
    }

    if (pTlsData->socketId >= 0) {
        close(pTlsData->socketId);
        pTlsData->socketId = -1;
    }
}

int aliyun_iot_network_ssl_connect(TLSDataParams_t *pTlsData, const char *addr, const char *port, const char *ca_crt,
                                   size_t ca_crt_len)
{
    int rc = 0;

    if (NULL == pTlsData) {
        WRITE_IOT_ERROR_LOG("network is null");
        return 1;
    }

    pTlsData->socketId = aliyun_iot_network_create(addr, port, IOT_NET_PROTOCOL_TCP);
    if (pTlsData->socketId < 0) {
        pTlsData->socketId = -1;
        pTlsData->pssl = NULL;
        WRITE_IOT_ERROR_LOG("TLS network create failed");
        return -1;
    }

    pTlsData->pssl = platform_ssl_connect(pTlsData->socketId, ca_crt, ca_crt_len);
    if (NULL == pTlsData->pssl) {
        WRITE_IOT_ERROR_LOG("openssl Connect failed");
        return -1;
    }

    ALIOT_LOG_DEBUG("openssl Connect Success");
    return 0;

}



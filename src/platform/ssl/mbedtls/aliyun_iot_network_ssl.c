#include "aliyun_iot_network_ssl.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_error.h"

#include "error.h"

#define DEBUG_LEVEL 10

unsigned int mqtt_avRandom()
{
    return (((unsigned int)rand() << 16) + rand());
}

static int mqtt_ssl_random(void *p_rng, unsigned char *output, size_t output_len)
{
    uint32_t rnglen = output_len;
    uint8_t   rngoffset = 0;

    while (rnglen > 0)
    {
        *(output + rngoffset) = (unsigned char)mqtt_avRandom() ;
        rngoffset++;
        rnglen--;
    }
    return 0;
}

static void mqtt_ssl_debug( void *ctx, int level, const char *file, int line, const char *str )
{
    ((void) level);

    fprintf( (FILE *) ctx, "%s:%04d: %s", file, line, str );
    fflush(  (FILE *) ctx  );
}

int mqtt_real_confirm(int verify_result)
{
    WRITE_IOT_DEBUG_LOG("certificate verification result: 0x%02x", verify_result);

    if((verify_result & MBEDTLS_X509_BADCERT_EXPIRED) != 0)
    {
         return ERROR_CERTIFICATE_EXPIRED;
    }

    if((verify_result & MBEDTLS_X509_BADCERT_REVOKED) != 0)
    {
        WRITE_IOT_NOTICE_LOG("! fail ! server certificate has been revoked");
    }

    if((verify_result & MBEDTLS_X509_BADCERT_CN_MISMATCH) != 0)
    {
        WRITE_IOT_NOTICE_LOG("! fail ! CN mismatch");
    }

    if((verify_result & MBEDTLS_X509_BADCERT_NOT_TRUSTED) != 0)
    {
        WRITE_IOT_NOTICE_LOG("! fail ! self-signed or not signed by a trusted CA");
    }

    return 0;
}

static int ssl_parse_crt(mbedtls_x509_crt *crt)
{
    char buf[1024];
    mbedtls_x509_crt *local_crt = crt;
    int i = 0;
    while (local_crt)
    {
        mbedtls_x509_crt_info(buf, sizeof(buf) - 1, "", local_crt);
        {
            char str[512];
            const char *start, *cur;
            start = buf;
            for (cur = buf; *cur != '\0'; cur++)
            {
                if (*cur == '\n')
                {
                    size_t len = cur - start + 1;
                    if (len > 511)
                    {
                        len = 511;
                    }
                    memcpy(str, start, len);
                    str[len] = '\0';
                    start = cur + 1;
                    printf("%s", str);
                }
            }
        }
        WRITE_IOT_DEBUG_LOG("crt content:%zu\n", strlen(buf));
        local_crt = local_crt->next;
        i++;
    }
    return i;
}

int mqtt_ssl_client_init(mbedtls_ssl_context *ssl,
                         mbedtls_net_context *tcp_fd,
                         mbedtls_ssl_config *conf,
                         mbedtls_x509_crt *crt509_ca, const char *ca_crt, size_t ca_len,
                         mbedtls_x509_crt *crt509_cli, const char *cli_crt, size_t cli_len,
                         mbedtls_pk_context *pk_cli, const char *cli_key, size_t key_len,  const char *cli_pwd, size_t pwd_len
                        )
{
    int ret = -1;

    /*
     * 0. Initialize the RNG and the session data
     */
#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold((int)DEBUG_LEVEL);
#endif
    mbedtls_net_init( tcp_fd );
    mbedtls_ssl_init( ssl );
    mbedtls_ssl_config_init( conf );
    mbedtls_x509_crt_init(crt509_ca);

    /*verify_source->trusted_ca_crt==NULL
     * 0. Initialize certificates
     */

    WRITE_IOT_DEBUG_LOG( "  . Loading the CA root certificate ..." );
    if (NULL != ca_crt)
    {
        if (0 != (ret = mbedtls_x509_crt_parse(crt509_ca, (const unsigned char *)ca_crt, ca_len)))
        {
            WRITE_IOT_ERROR_LOG(" failed ! x509parse_crt returned -0x%04x", -ret);
            return ret;
        }
    }
    ssl_parse_crt(crt509_ca);
    WRITE_IOT_DEBUG_LOG( " ok (%d skipped)", ret );


    /* Setup Client Cert/Key */
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#if defined(MBEDTLS_CERTS_C)
    mbedtls_x509_crt_init(crt509_cli);
    mbedtls_pk_init( pk_cli );
#endif
    if ( cli_crt != NULL && cli_key != NULL)
    {
#if defined(MBEDTLS_CERTS_C)
        WRITE_IOT_DEBUG_LOG("start prepare client cert .");
        ret = mbedtls_x509_crt_parse( crt509_cli, (const unsigned char *) cli_crt, cli_len );
#else
        {
            ret = 1;
            WRITE_IOT_DEBUG_LOG("MBEDTLS_CERTS_C not defined.");
        }
#endif
        if ( ret != 0 )
        {
            WRITE_IOT_ERROR_LOG( " failed!  mbedtls_x509_crt_parse returned -0x%x\n", -ret );
            return ret;
        }

#if defined(MBEDTLS_CERTS_C)
        WRITE_IOT_DEBUG_LOG("start mbedtls_pk_parse_key[%s]", cli_pwd);
        ret = mbedtls_pk_parse_key( pk_cli,(const unsigned char *) cli_key, key_len, (const unsigned char *) cli_pwd, pwd_len);
#else
        {
            ret = 1;
            WRITE_IOT_ERROR_LOG("MBEDTLS_CERTS_C not defined.");
        }
#endif

        if ( ret != 0 )
        {
            WRITE_IOT_ERROR_LOG( " failed\n  !  mbedtls_pk_parse_key returned -0x%x\n", -ret);
            return ret;
        }
    }
#endif /* MBEDTLS_X509_CRT_PARSE_C */

    return 0;
}


int aliyun_iot_network_ssl_read(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms)
{
    size_t readLen = 0;
    int ret = -1;
	
    WRITE_IOT_DEBUG_LOG("aliyun_iot_network_ssl_read len=%d timer=%d ms", len, timeout_ms);
	
    mbedtls_ssl_conf_read_timeout(&(pTlsData->conf), timeout_ms);
    while (readLen < len)
    {
        ret = mbedtls_ssl_read(&(pTlsData->ssl), (unsigned char *)(buffer + readLen), (len - readLen));
        //WRITE_IOT_DEBUG_LOG("%s, mbedtls_ssl_read return:%d", __func__, ret);
        if (ret > 0)
        {
            readLen += ret;
        }
        else if (ret == 0)
        {
            //WRITE_IOT_ERROR_LOG("mqtt ssl read timeout");
            return 0; //eof
        }
        else
        {
            if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
            {
                //read already complete(if call mbedtls_ssl_read again, it will return 0(eof))
                WRITE_IOT_ERROR_LOG("MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY");
                //return -2;
                return readLen;
            }

            WRITE_IOT_ERROR_LOG("mqtt recv ssl error,ret = %d",ret);
            return -1; //Connnection error
        }
    }
    WRITE_IOT_DEBUG_LOG("aliyun_iot_network_ssl_read readlen=%zu", readLen);
    return readLen;
}

int aliyun_iot_network_ssl_write(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms)
{
    size_t writtenLen = 0;
    int ret = -1;

    WRITE_IOT_DEBUG_LOG("mqtt_ssl_write_all len=%d timer=%d ms", len, timeout_ms);
    while (writtenLen < len)
    {
        ret = mbedtls_ssl_write(&(pTlsData->ssl), (unsigned char *)(buffer + writtenLen), (len - writtenLen));
        if (ret > 0)
        {
            writtenLen += ret;
            continue;
        }
        else if (ret == 0)
        {
            WRITE_IOT_ERROR_LOG("mqtt ssl write timeout");
            return -2;
        }
        else
        {
            WRITE_IOT_ERROR_LOG("mqtt ssl write fail,ret = %d",ret);
            return -1; //Connnection error
        }
    }
    WRITE_IOT_DEBUG_LOG("mqtt ssl write len=%zu", writtenLen);
    return writtenLen;
}

void aliyun_iot_network_ssl_disconnect(TLSDataParams *pTlsData)
{
    mbedtls_ssl_close_notify(&(pTlsData->ssl));
    mbedtls_net_free(&(pTlsData->fd));
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_x509_crt_free( &(pTlsData->cacertl));
    if ((pTlsData->pkey).pk_info != NULL)
    {
        WRITE_IOT_DEBUG_LOG("mqtt need free client crt&key");
        mbedtls_x509_crt_free( &(pTlsData->clicert));
        mbedtls_pk_free( &(pTlsData->pkey) );
    }
#endif
    mbedtls_ssl_free( &(pTlsData->ssl));
    mbedtls_ssl_config_free(&(pTlsData->conf));
    WRITE_IOT_DEBUG_LOG( " mqtt_ssl_disconnect\n" );
}

/**
 * @brief This function connects to a MQTT server with TLS, and returns a value that indicates whether the connection is create successfully or not. Call #NewNetwork() to initialize network structure before calling this function.
 * @param[in] n is the the network structure pointer.
 * @param[in] addr is the Server Host name or IP address.
 * @param[in] port is the Server Port.
 * @param[in] ca_crt is the Server's CA certification.
 * @param[in] ca_crt_len is the length of Server's CA certification.
 * @param[in] client_crt is the client certification.
 * @param[in] client_crt_len is the length of client certification.
 * @param[in] client_key is the client key.
 * @param[in] client_key_len is the length of client key.
 * @param[in] client_pwd is the password of client key.
 * @param[in] client_pwd_len is the length of client key's password.
 * @sa #NewNetwork();
 * @return If the return value is 0, the connection is created successfully. If the return value is -1, then calling lwIP #socket() has failed. If the return value is -2, then calling lwIP #connect() has failed. Any other value indicates that calling lwIP #getaddrinfo() has failed.
 */
int TLSConnectNetwork(TLSDataParams *pTlsData, const char *addr, const char *port,
                      const char *ca_crt, size_t ca_crt_len,
                      const char *client_crt,	size_t client_crt_len,
                      const char *client_key,	size_t client_key_len,
                      const char *client_pwd, size_t client_pwd_len)
{
    int ret = -1;
    /*
     * 0. Init
     */
    if (0 != (ret = mqtt_ssl_client_init(&(pTlsData->ssl), &(pTlsData->fd), &(pTlsData->conf),
                                         &(pTlsData->cacertl), ca_crt, ca_crt_len,
                                         &(pTlsData->clicert), client_crt, client_crt_len,
                                         &(pTlsData->pkey), client_key, client_key_len, client_pwd, client_pwd_len)))
    {
        WRITE_IOT_ERROR_LOG( " failed ! ssl_client_init returned -0x%04x", -ret );
        return ret;
    }

    /*
     * 1. Start the connection
     */
    WRITE_IOT_DEBUG_LOG("  . Connecting to tcp/%s/%s...", addr, port);
    if (0 != (ret = mbedtls_net_connect(&(pTlsData->fd), addr, port, MBEDTLS_NET_PROTO_TCP)))
    {
        WRITE_IOT_ERROR_LOG(" failed ! net_connect returned -0x%04x", -ret);
        return ret;
    }
    WRITE_IOT_DEBUG_LOG( " ok" );

    /*
     * 2. Setup stuff
     */
    WRITE_IOT_DEBUG_LOG( "  . Setting up the SSL/TLS structure..." );
    if ( ( ret = mbedtls_ssl_config_defaults( &(pTlsData->conf),MBEDTLS_SSL_IS_CLIENT,MBEDTLS_SSL_TRANSPORT_STREAM,MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        WRITE_IOT_ERROR_LOG( " failed! mbedtls_ssl_config_defaults returned %d", ret );
        return ret;
    }
    WRITE_IOT_DEBUG_LOG( " ok" );

    /* OPTIONAL is not optimal for security, but makes interop easier in this simplified example */
    if (ca_crt != NULL)
    {
        mbedtls_ssl_conf_authmode( &(pTlsData->conf), MBEDTLS_SSL_VERIFY_OPTIONAL );
    }
    else
    {
        mbedtls_ssl_conf_authmode( &(pTlsData->conf), MBEDTLS_SSL_VERIFY_NONE);
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_ssl_conf_ca_chain( &(pTlsData->conf), &(pTlsData->cacertl), NULL);

    if ( ( ret = mbedtls_ssl_conf_own_cert( &(pTlsData->conf), &(pTlsData->clicert), &(pTlsData->pkey) ) ) != 0 )
    {
        WRITE_IOT_ERROR_LOG( " failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n", ret );
        return ret;
    }
#endif
    mbedtls_ssl_conf_rng( &(pTlsData->conf), mqtt_ssl_random, NULL );
    mbedtls_ssl_conf_dbg( &(pTlsData->conf), mqtt_ssl_debug, NULL );
    //mbedtls_ssl_conf_dbg( &(pTlsData->conf), mqtt_ssl_debug, stdout );

    if ( ( ret = mbedtls_ssl_setup(&(pTlsData->ssl), &(pTlsData->conf)) ) != 0 )
    {
        WRITE_IOT_ERROR_LOG( " failed! mbedtls_ssl_setup returned %d", ret );
        return ret;
    }
    mbedtls_ssl_set_hostname(&(pTlsData->ssl), addr);
    mbedtls_ssl_set_bio( &(pTlsData->ssl), &(pTlsData->fd), mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    /*
      * 4. Handshake
      */
    WRITE_IOT_DEBUG_LOG("  . Performing the SSL/TLS handshake...");

    while ((ret = mbedtls_ssl_handshake(&(pTlsData->ssl))) != 0)
    {
        if ((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE))
        {
        	WRITE_IOT_ERROR_LOG( " failed  ! mbedtls_ssl_handshake returned -0x%04x", -ret);
            return ret;
        }
    }
    WRITE_IOT_DEBUG_LOG( " ok" );
    /*
     * 5. Verify the server certificate
     */
    WRITE_IOT_DEBUG_LOG("  . Verifying peer X.509 certificate..");
    if (0 != (ret = mqtt_real_confirm(mbedtls_ssl_get_verify_result(&(pTlsData->ssl)))))
    {
        WRITE_IOT_ERROR_LOG(" failed  ! verify result not confirmed.");
        return ret;
    }
    //n->my_socket = (int)((n->tlsdataparams.fd).fd);
    //WRITE_IOT_DEBUG_LOG("my_socket=%d", n->my_socket);

    return 0;
}

int aliyun_iot_network_ssl_connect(TLSDataParams *pTlsData, const char *addr, const char *port, const char *ca_crt, size_t ca_crt_len)
{
    return TLSConnectNetwork(pTlsData, addr, port, ca_crt, ca_crt_len, NULL, 0, NULL, 0, NULL, 0);
    //return TLSConnectNetwork(pTlsData, addr, port, NULL, 0, NULL, 0, NULL, 0, NULL, 0);
}


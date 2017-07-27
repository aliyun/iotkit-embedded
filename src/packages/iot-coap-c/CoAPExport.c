#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CoAPNetwork.h"
#include "CoAPExport.h"

#define COAP_PRE_SERVER_URL  "coap://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5683"
#define COAPS_PRE_SERVER_URL "coaps://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5684"

#define COAP_DEFAULT_PORT        5683 /* CoAP default UDP port */
#define COAPS_DEFAULT_PORT       5684 /* CoAP default UDP port for secure transmission */

#define COAP_DEFAULT_SCHEME      "coap" /* the default scheme for CoAP URIs */
#define COAP_DEFAULT_HOST_LEN    128

unsigned int CoAPUri_Parse(unsigned char *p_uri, coap_address_t *p_addr, coap_endpoint_type *p_endpoint_type)
{
    int len = 0;
    char host[COAP_DEFAULT_HOST_LEN] = {0};
    unsigned char *p = NULL, *q = NULL;
    if(NULL == p_uri || NULL == p_addr || NULL == p_endpoint_type){
        return COAP_ERROR_INVALID_PARAM;
    }

    len = strlen(p_uri);
    p = p_uri;
    q = (unsigned char *)COAP_DEFAULT_SCHEME;
    while(len && *q && tolower(*p)==*q){
        ++p;
        ++q;
        --len;
    }

    if(*q){
        return COAP_ERROR_INVALID_URI;
    }

    if(tolower(*p) == 's'){
        ++p;
        --len;
        *p_endpoint_type = COAP_ENDPOINT_DTLS;
        p_addr->port     = COAPS_DEFAULT_PORT;
    }
    else{
        *p_endpoint_type = COAP_ENDPOINT_NOSEC;
        p_addr->port     = COAP_DEFAULT_PORT;
    }
    COAP_DEBUG("The endpoint type is: %d\r\n", *p_endpoint_type);

    q = (unsigned char *)"://";
    while(len && *q && tolower(*p)==*q){
        ++p;
        ++q;
        --len;
    }

    if(*q){
        return COAP_ERROR_INVALID_URI;
    }

    q = p;
    while(len && *q != ':'){
        ++q;
        --len;
    }
    if(p == q){
        return COAP_ERROR_INVALID_URI;
    }

    if(COAP_DEFAULT_HOST_LEN-1 < (q-p)){
        return COAP_ERROR_INVALID_URI;
    }
    else{
        strncpy(host , p, q - p);
    }
    COAP_DEBUG("The host name is: %s\r\n", host);
    HAL_UDP_resolveAddress(host, p_addr->addr);
    COAP_DEBUG("The address is: %s\r\n", p_addr->addr);

    if(len && *q == ':'){
        p = ++q;
        --len;

        while (len && isdigit (*q)){
            ++q;
            --len;
        }

        if (p < q){
            int uri_port = 0;

            while (p < q){
                uri_port = uri_port * 10 + (*p++ - '0');
            }

            if(uri_port > 65535){
                return COAP_ERROR_INVALID_URI;
            }
            p_addr->port = uri_port;
        }
    }
    COAP_DEBUG("The port is: %d\r\n", p_addr->port);

    return COAP_SUCCESS;
}


CoAPContext *CoAPContext_Create(char        *p_uri)
{
    unsigned int    ret   = COAP_SUCCESS;
    CoAPContext    *p_ctx = NULL;
    coap_network_init_t network_param;

    memset(&network_param, 0x00, sizeof(coap_network_init_t));
    p_ctx = coap_malloc(sizeof(CoAPContext));
    if(NULL == p_ctx){
        COAP_ERR("Create coap new context failed\r\n");
        return NULL;
    }

    p_ctx->message_id = 1;
    p_ctx->notifier = NULL;
    p_ctx->sendbuf = coap_malloc(COAP_MAX_PDU_LEN);
    p_ctx->recvbuf = coap_malloc(COAP_MAX_PDU_LEN);

    /*CoAP message send list*/
    CoAPList_Init(&p_ctx->list, 10);

    /*set the endpoint type by uri schema*/
    if(NULL != p_uri){
        ret = CoAPUri_Parse(p_uri, &network_param.remote.addr, &network_param.ep_type);
    }
    else{
#ifdef COAP_DTLS_SUPPORT
        ret = CoAPUri_Parse(COAPS_PRE_SERVER_URL, &network_param.remote.addr, &network_param.ep_type);
#else
        ret = CoAPUri_Parse(COAP_PRE_SERVER_URL, &network_param.remote.addr, &network_param.ep_type);
#endif
    }

    if(COAP_SUCCESS != ret){
        if(NULL != p_ctx){
            coap_free(p_ctx);
            p_ctx    =  NULL;
        }
    }

#ifdef COAP_DTLS_SUPPORT
    if(COAP_ENDPOINT_DTLS == network_param.ep_type){
        extern const char *iotx_coap_get_ca(void);
        network_param.p_ca_cert_pem     =  iotx_coap_get_ca();
        network_param.ep_type           = COAP_ENDPOINT_DTLS;
    }
#endif

    if(COAP_ENDPOINT_NOSEC == network_param.ep_type){
        network_param.ep_type = COAP_ENDPOINT_NOSEC;
        network_param.p_ca_cert_pem = NULL;
    }

    /*CoAP network init*/
    ret = CoAPNetwork_init(&network_param,  &p_ctx->network);

    if(COAP_SUCCESS != ret){
        if(NULL != p_ctx){
            coap_free(p_ctx);
            p_ctx    =  NULL;
        }
    }

    return p_ctx;
}


void CoAPContext_Free(CoAPContext *p_ctx)
{
    CoAPNetwork_deinit(&p_ctx->network);

    if(NULL != p_ctx){
        coap_free(p_ctx);
        p_ctx    =  NULL;
    }
}

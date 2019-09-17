/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */




#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "iotx_coap_internal.h"
#include "Cloud_CoAPExport.h"
#include "Cloud_CoAPNetwork.h"

unsigned int Cloud_CoAPNetwork_write(coap_network_t *p_network,
                                     const unsigned char   *p_data,
                                     unsigned int           datalen)
{
    int rc = COAP_ERROR_WRITE_FAILED;

    rc = HAL_UDP_write((intptr_t)p_network->context, p_data, datalen);
    COAP_DEBUG("[CoAP-NWK]: Network write return %d", rc);

    if (-1 == rc) {
        rc = COAP_ERROR_WRITE_FAILED;
    } else {
        rc = COAP_SUCCESS;
    }
    return (unsigned int)rc;
}

int Cloud_CoAPNetwork_read(coap_network_t *network, unsigned char  *data,
                           unsigned int datalen, unsigned int timeout)
{
    int len = 0;

    memset(data, 0x00, datalen);
    len = HAL_UDP_readTimeout((intptr_t)network->context,
                              data, COAP_MSG_MAX_PDU_LEN, timeout);
    if (len > 0) {
        COAP_TRC("<< CoAP recv %d bytes data", len);
    }
    return len;
}

unsigned int Cloud_CoAPNetwork_init(const coap_network_init_t *p_param, coap_network_t *p_network)
{
    unsigned int    err_code = COAP_SUCCESS;

    if (NULL == p_param || NULL == p_network) {
        return COAP_ERROR_INVALID_PARAM;
    }

    /* TODO : Parse the url here */
    p_network->ep_type = p_param->ep_type;

    if (COAP_ENDPOINT_NOSEC == p_param->ep_type
        || COAP_ENDPOINT_PSK == p_param->ep_type) {
        /*Create udp socket*/
        p_network->context = (void *)HAL_UDP_create(p_param->p_host, p_param->port);
        if ((void *) - 1 == p_network->context) {
            return COAP_ERROR_NET_INIT_FAILED;
        }
    }
    return err_code;
}


unsigned int Cloud_CoAPNetwork_deinit(coap_network_t *p_network)
{
    unsigned int    err_code = COAP_SUCCESS;

    if (COAP_ENDPOINT_NOSEC == p_network->ep_type
        || COAP_ENDPOINT_PSK == p_network->ep_type) {
        HAL_UDP_close_without_connect((intptr_t)p_network->context);
    }

    return err_code;
}


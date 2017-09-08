/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "iot_import_coap.h"

#ifndef __COAP_DTLS_H__
#define __COAP_DTLS_H__

#define dtls_log_print(level, ...) \
    {\
        fprintf(stderr, "%s [%s #%d]   ",level, __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);\
    }

#define DTLS_TRC(fmt,  args...)  dtls_log_print("<TRACE>", fmt, ##args)
#define DTLS_DUMP(fmt, args...)  dtls_log_print("<DUMP> ", fmt, ##args)
#define DTLS_DEBUG(fmt,args...)  dtls_log_print("<DEBUG>", fmt, ##args)
#define DTLS_INFO(fmt, args...)  dtls_log_print("<INFO> ", fmt, ##args)
#define DTLS_ERR(fmt,  args...)  dtls_log_print("<ERROR>", fmt, ##args)

#define DTLS_ERROR_BASE       (1<<24)


#define DTLS_SUCCESS                        0
#define DTLS_INVALID_PARAM             (DTLS_ERROR_BASE | 1)
#define DTLS_INVALID_CA_CERTIFICATE    (DTLS_ERROR_BASE | 2)
#define DTLS_HANDSHAKE_IN_PROGRESS     (DTLS_ERROR_BASE | 3)
#define DTLS_HANDSHAKE_FAILED          (DTLS_ERROR_BASE | 4)
#define DTLS_FATAL_ALERT_MESSAGE       (DTLS_ERROR_BASE | 5)
#define DTLS_PEER_CLOSE_NOTIFY         (DTLS_ERROR_BASE | 6)
#define DTLS_SESSION_CREATE_FAILED     (DTLS_ERROR_BASE | 7)
#define DTLS_READ_DATA_FAILED          (DTLS_ERROR_BASE | 8)


typedef struct {
    unsigned char             *p_ca_cert_pem;
    char                      *p_host;
    unsigned short             port;
} coap_dtls_options_t;


typedef void DTLSContext;

DTLSContext *HAL_DTLSSession_create(coap_dtls_options_t  *p_options);

unsigned int HAL_DTLSSession_write(DTLSContext *context,
                                   const unsigned char   *p_data,
                                   unsigned int    *p_datalen);


unsigned int HAL_DTLSSession_read(DTLSContext *context,
                                  unsigned char   *p_data,
                                  unsigned int    *p_datalen,
                                  unsigned int     timeout);

unsigned int HAL_DTLSSession_free(DTLSContext *context);


#endif

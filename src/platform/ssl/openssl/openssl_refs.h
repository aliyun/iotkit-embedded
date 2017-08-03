 /*
  * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
  * License-Identifier: Apache-2.0
  *
  * Licensed under the Apache License, Version 2.0 (the "License"); you may
  * not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */


#ifndef ALIYUN_IOT_NETWORK_SSL_H
#define ALIYUN_IOT_NETWORK_SSL_H

#include "lite-log.h"
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

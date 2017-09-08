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


#include "iot_import.h"
#ifndef __COAP_PLATFORM_H__
#define __COAP_PLATFORM_H__

#define NETWORK_ADDR_LEN      (16)


#define coap_malloc(size) HAL_Malloc(size)
#define coap_free(ptr)    HAL_Free(ptr)


void *HAL_UDP_create(char *host, unsigned short port);
void  HAL_UDP_close(void *p_socket);
int   HAL_UDP_write(void *p_socket, const unsigned char *p_data, unsigned int datalen);
int   HAL_UDP_read(void         *p_socket, unsigned char   *p_data, unsigned int     datalen);
int   HAL_UDP_readTimeout( void *p_socket,unsigned char  *p_data,
                unsigned int datalen,     unsigned int timeout );

int HAL_UDP_resolveAddress(const char *p_host,  char addr[NETWORK_ADDR_LEN]);


#endif

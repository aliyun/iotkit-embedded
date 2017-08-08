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


#include <stdlib.h>
#include <memory.h>

#include "utils_httpc.h"
#include "utils_epoch_time.h"
#include "lite-log.h"

#define HTTP_RESP_CONTENT_LEN   (64)

extern int httpclient_common(httpclient_t *client,
                             const char *url,
                             int port,
                             const char *ca_crt,
                             int method,
                             uint32_t timeout_ms,
                             httpclient_data_t *client_data);

uint64_t utils_get_epoch_time(char copy[], int len)
{
    int ret;
    httpclient_t httpclient;
    httpclient_data_t httpclient_data;
    char http_content[HTTP_RESP_CONTENT_LEN];

    memset(&httpclient, 0, sizeof(httpclient_t));
    memset(&httpclient_data, 0, sizeof(httpclient_data));
    memset(&http_content, 0, sizeof(HTTP_RESP_CONTENT_LEN));

    httpclient.header = "Accept: text/xml,text/html,\r\n";

    httpclient_data.response_buf = http_content;
    httpclient_data.response_content_len = HTTP_RESP_CONTENT_LEN;

    ret = httpclient_common(
                      &httpclient,
                      "http://iot-nsauth.alibaba.net/system/time",
                      80,
                      NULL,
                      HTTPCLIENT_GET,
                      5000,
                      &httpclient_data);
    if (0 != ret) {
        log_err("request epoch time from remote server failed.");
        return 0;
    } else {

        char            backup;
        uint64_t        cnv = 0;
        uint64_t        res = 0;

        backup = http_content[10];
        http_content[10] = '\0';
        cnv = atoi(http_content);
        res = cnv * 1000;
        http_content[10] = backup;
        cnv = atoi(http_content + 10);
        res += cnv;
        log_debug("res = %ld", res);

        snprintf(copy, len, "%s", http_content);
        return res;
    }
}

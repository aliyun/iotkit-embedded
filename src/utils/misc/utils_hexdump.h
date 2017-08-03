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


#ifndef _IOTX_DUMP_H_
#define _IOTX_DUMP_H_

typedef enum {
    IOTX_HEXDUMP_PREFIX_NONE,
    IOTX_HEXDUMP_PREFIX_ADDRESS,
    IOTX_HEXDUMP_PREFIX_OFFSET
} utils_hexdump_prefix_type_t;


void utils_hexdump(utils_hexdump_prefix_type_t prefix_type,
                   int rowsize,
                   int groupsize,
                   const void *buf,
                   size_t len,
                   int ascii);

#endif /* IOTX_DUMP_H_ */

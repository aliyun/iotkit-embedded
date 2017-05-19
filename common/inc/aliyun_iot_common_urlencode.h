#ifndef ALIYUN_IOT_COMMON_URLENCODE_H
#define ALIYUN_IOT_COMMON_URLENCODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

char *aliyun_iot_common_url_encode(const char *input);

char *aliyun_iot_common_url_decode(const char *input);

#endif


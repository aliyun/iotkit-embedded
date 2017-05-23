#ifndef ALIYUN_IOT_COMMON_HMAC_H
#define ALIYUN_IOT_COMMON_HMAC_H

#include <string.h>

//#include "md5.h"
//#include "sha1.h"


void aliyun_iot_common_hmac_md5(const char *msg, int msg_len, char *digest, const char *key, int key_len);

void aliyun_iot_common_hmac_sha1(const char *msg, int msg_len, char *digest, const char *key, int key_len);

#endif


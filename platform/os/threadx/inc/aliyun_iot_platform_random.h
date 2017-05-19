#ifndef ALIYUN_IOT_PLATFORM_RANDOM_H
#define ALIYUN_IOT_PLATFORM_RANDOM_H

#include <stdlib.h>

int aliyun_iot_gen_random(unsigned char *random, unsigned int len);
int aliyun_iot_rand(void) ;
double aliyun_iot_pow(double x, int n);

#endif


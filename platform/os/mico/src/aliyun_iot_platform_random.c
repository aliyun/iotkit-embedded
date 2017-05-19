#include "aliyun_iot_platform_random.h"

int aliyun_iot_gen_random(unsigned char *random, unsigned int len)
{
    unsigned int i;
    unsigned char c;

    for (i = 0; i < len; i++)
    {
        c = rand() % 256;
        random[i] = c;
    }

    return 0;
}


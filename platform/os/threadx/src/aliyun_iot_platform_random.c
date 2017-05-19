#include "aliyun_iot_platform_random.h"

static unsigned long next = 1;

int JsonAbs(int a)
{
    int temp = (a >> 31);
    return (a + temp) ^ temp;
}

double aliyun_iot_pow(double x, int n)
{
    int m = JsonAbs(n);
    double result = 1;
    while(m > 0)
	{
        if((m&1) != 0)
		{
            result = result * x;
        }
        x *= x;
        m >>= 1;
    }
    return n > 0 ? result : 1 / result;
}

/* RAND_MAX assumed to be 32767 */
int aliyun_iot_rand(void) 
{
    next = next * 1103515245 + 12345;
    return((unsigned)(next/65536) % 32768);
}

int aliyun_iot_srand(int seed)
{
	next = seed;
}
int aliyun_iot_gen_random(unsigned char *random, unsigned int len)
{
    unsigned int i;
    unsigned char c;

    for (i = 0; i < len; i++)
    {
        c = aliyun_iot_rand() % 256;
        random[i] = c;
    }

    return 0;
}


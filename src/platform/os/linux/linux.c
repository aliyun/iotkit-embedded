#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include "aliot_platform_os.h"

#define PLATFORM_LINUX_LOG(format, ...) \
    do { \
        printf("Linux:%s:%d %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
        fflush(stdout);\
    }while(0);

void *aliot_platform_mutex_create(void)
{
    int err_num;
    pthread_mutex_t *mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (NULL == mutex)
    {
        return NULL;
    }

    if (0 != (err_num = pthread_mutex_init(mutex, NULL))) {
        perror("create mutex failed");
        free(mutex);
        return NULL;
    }

    return mutex;
}

void aliot_platform_mutex_destroy(_IN_ void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_destroy((pthread_mutex_t *)mutex))) {
        perror("destroy mutex failed");
    }

    free(mutex);
}

void aliot_platform_mutex_lock(_IN_ void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_lock((pthread_mutex_t *)mutex))) {
        perror("lock mutex failed");
    }
}

void aliot_platform_mutex_unlock(_IN_ void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_unlock((pthread_mutex_t *)mutex))) {
        perror("unlock mutex failed");
    }
}

void *aliot_platform_malloc(_IN_ uint32_t size)
{
    return malloc(size);
}

void aliot_platform_free(_IN_ void *ptr)
{
    return free(ptr);
}

int aliot_platform_ota_start(const char *md5, uint32_t file_size)
{
    printf("this interface is NOT support yet.");
    return -1;
}

int aliot_platform_ota_write(_IN_ char *buffer, _IN_ uint32_t length)
{
    printf("this interface is NOT support yet.");
    return -1;
}

int aliot_platform_ota_finalize(_IN_ int stat)
{
    printf("this interface is NOT support yet.");
    return -1;
}

uint32_t aliot_platform_time_get_ms(void)
{
    struct timeval tv = { 0 };
    uint32_t time_ms;

    gettimeofday(&tv, NULL);

    time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return time_ms;
}

void aliot_platform_msleep(_IN_ uint32_t ms)
{
    usleep( 1000 * ms );
}

void aliot_platform_printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}

char *aliot_platform_module_get_pid(char pid_str[])
{
    strcpy(pid_str, "123456789");
    return pid_str;
}

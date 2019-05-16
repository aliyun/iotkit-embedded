/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <stdint.h>
#include <errno.h>

#if defined(AT_PARSER_ENABLED)
static int read_and_discard_all_data(const int fd)
{
    return 0;
}

int32_t HAL_AT_Uart_Init(uart_dev_t *uart)
{
    return 0;
}

int32_t HAL_AT_Uart_Deinit(uart_dev_t *uart)
{

    return 0;
}

int32_t HAL_AT_Uart_Send(uart_dev_t *uart, const void *data,
                         uint32_t size, uint32_t timeout)
{
    return 0;
}

int32_t HAL_AT_Uart_Recv(uart_dev_t *uart, void *data, uint32_t expect_size,
                         uint32_t *recv_size, uint32_t timeout)
{
    return 0;
}

#endif
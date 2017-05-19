#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_platform_uart.h"
#include "mico.h"
#include "platform.h"
#include "RingBufferUtils.h"
#include "debug.h"

#define UART_RX_BUFFER_LEN 2048
static ring_buffer_t  uart_ring_buffer;
static UINT8  uart_ring_data[UART_RX_BUFFER_LEN] = {0};

INT32 aliyun_iot_uart_send(char *pSendBuffer, int bufferLen)
{
	return MicoUartSend(UART_FOR_APP, pSendBuffer, bufferLen);
}

INT32 aliyun_iot_uart_recv(char *pRecvBuffer, int bufferLen, unsigned int timeOutMs)
{
	return MicoUartRecv(UART_FOR_APP, pRecvBuffer, bufferLen, timeOutMs);
}

INT32 aliyun_iot_uart_init()
{
	mico_uart_config_t uart_config = {0};

	uart_config.baud_rate	 = 115200;
	uart_config.data_width	 = DATA_WIDTH_8BIT;
	uart_config.parity		 = NO_PARITY;
	uart_config.stop_bits	 = STOP_BITS_1;
	uart_config.flow_control = FLOW_CONTROL_DISABLED;

	ring_buffer_init(&uart_ring_buffer, (UINT8 *)uart_ring_data, UART_RX_BUFFER_LEN);
	return MicoUartInitialize( UART_FOR_APP, &uart_config, &uart_ring_buffer );
}

/*
	Read the length of the data that is already recived by uart driver and stored in buffer
*/
INT32 aliyun_iot_uart_buffer_len()
{
	return MicoUartGetLengthInBuffer(UART_FOR_APP);
}


#ifndef ALIYUN_IOT_PLATFORM_UART_H
#define ALIYUN_IOT_PLATFORM_UART_H

/*
	send uart data
*/
INT32 aliyun_iot_uart_send(INT8 *pSendBuffer, INT32 bufferLen);

/*
	receive uart data
*/
INT32 aliyun_iot_uart_recv(INT8 *pRecvBuffer, INT32 bufferLen, UINT32 timeOutMs);

/*
	initialize the baud rate of the uart
*/
INT32 aliyun_iot_uart_init();

/*
	 the length of the data readable inside uart buffer
*/
INT32 aliyun_iot_uart_buffer_len();


#endif

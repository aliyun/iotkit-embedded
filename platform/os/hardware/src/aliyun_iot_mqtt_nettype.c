#include <string.h>
#include <stdio.h>
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_mqtt_nettype.h"
#include "aliyun_iot_platform_timer.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_platform_uart.h"

//AT+CPIN?
#define AT_GPRS_CHECK_SIM_CARD_FMT "AT+CPIN?\r\n"

//AT+CREG?
#define AT_GPRS_CHECK_REG_NET_FMT "AT+CREG?\r\n"

//AT+CGATT?
#define AT_GPRS_CHECK_GPRS_NET_NORMAL_FMT "AT+CGATT?\r\n"

//AT+CGATT=1
#define AT_GPRS_SET_GPRS_NET_NORMAL_FMT "AT+CGATT=1\r\n"

//AT+CFUN=1,1
#define AT_GPRS_RESTART_RF_MODULE_FMT "AT+CFUN=1,1\r\n"

//ATE0 close echo function
#define AT_GPRS_CLOSE_ECHO_FUNC_FMT "ATE0\r\n"

//AT+CSTT set apn
#define AT_GPRS_SET_APN_FUNC_FMT "AT+CSTT=\"%s\"\r\n"

//AT+CIICR
#define AT_GPRS_ACTIVATE_FMT "AT+CIICR\r\n"

//AT+CIFSR get local ip address
#define AT_GPRS_LOOPUP_IP_ADDR_FMT "AT+CIFSR\r\n"

// tcp connect
// AT+ CIPMUX=<mode>
// AT+CIPSTART= <type>,<addr>,<port>
#define AT_TCP_SOCKET_INIT_FMT "AT+CIPMUX=1\r\n"
#define AT_TCP_SOCKET_CONNECT_FMT "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n"
#define AT_SSL_SOCKET_CONNECT_FMT "AT+CIPSTART=%d,\"SSL\",\"%s\",%d\r\n"

// tcp fast close
// AT+CIPCLOSE=<Socket id>
#define AT_TCP_SOCKET_CLOSE_FMT "AT+CIPCLOSE=1\r\n"

//AT+CIPSHUT
#define AT_TCP_SOCKET_SHUTDOWN_FMT "AT+CIPSHUT\r\n"

// tcp send data
// AT+CIPSEND=<link id>,<length>
#define AT_TCP_SOCKET_SEND_DATA_FMT "AT+CIPSEND=%d\r\n"

// tcp recv data
// +IPD,<id>,<len>:<data>
#define AT_TCP_SOCKET_RECV_DATA_FMT "+IPD,%d,%d:%s"

//set ip header AT+CIPHEAD=1
#define AT_TCP_SOCKET_IP_HEADER_SET_FMT "AT+CIPHEAD=1\r\n"


typedef struct
{
	UINT32 size;
	UINT32 head;
	UINT32 tail;
	UINT8 *buffer;
} iot_ring_buffer_t;

#define MQTT_RING_BUFFER_LEN 1024
static iot_ring_buffer_t  mqtt_ring_buffer;
static UINT8 mqtt_ring_data[MQTT_RING_BUFFER_LEN] = {0};

#define IOT_MIN(x,y)  ((x) < (y) ? (x) : (y))

/*
	Initializes the size of the ring buffer
*/
INT32 aliyun_iot_ring_buffer_init( iot_ring_buffer_t *ring_buffer, UINT8 *buffer, UINT32 size )
{
    ring_buffer->buffer     = buffer;
    ring_buffer->size       = size;
    ring_buffer->head       = 0;
    ring_buffer->tail       = 0;
    return 0;
}

/*
	the size of ring buffer is already in use
*/
UINT32 aliyun_iot_ring_buffer_used_space( iot_ring_buffer_t *ring_buffer )
{
	UINT32 head_to_end = ring_buffer->size - ring_buffer->head;
	return ((head_to_end + ring_buffer->tail) % ring_buffer->size);
}


/*
	read mqtt data from ring buffer
*/
UINT8 aliyun_iot_ring_buffer_read(iot_ring_buffer_t *ring_buffer, UINT8 *data, UINT32 bytes_consume)
{
	UINT32 head_to_end = ring_buffer->size - ring_buffer->head;
	if(bytes_consume < head_to_end)
	{
		memcpy(data, &(ring_buffer->buffer[ring_buffer->head]), bytes_consume);
	}
	else
	{
		memcpy(data, &(ring_buffer->buffer[ring_buffer->head]), head_to_end);
		memcpy((data + head_to_end), ring_buffer->buffer, (bytes_consume - head_to_end));
	}
	ring_buffer->head = (ring_buffer->head + bytes_consume) % ring_buffer->size;
	return 0;
}

/*
	store mqtt data into global ring buffer
*/
UINT32 aliyun_iot_ring_buffer_write( iot_ring_buffer_t *ring_buffer, const UINT8 *data, UINT32 data_length )
{
	UINT32 tail_to_end = ring_buffer->size - ring_buffer->tail;

	/* Calculate the maximum amount we can copy */
	UINT32 amount_to_copy = IOT_MIN(data_length, (ring_buffer->tail == ring_buffer->head) ? ring_buffer->size : (tail_to_end + ring_buffer->head) % ring_buffer->size);

	/* Copy as much as we can until we fall off the end of the buffer */
	memcpy(&ring_buffer->buffer[ring_buffer->tail], data, IOT_MIN(amount_to_copy, tail_to_end));

	/* Check if we have more to copy to the front of the buffer */
	if (tail_to_end < amount_to_copy)
	{
		memcpy(ring_buffer->buffer, data + tail_to_end, amount_to_copy - tail_to_end);
	}

	/* Update the tail */
	ring_buffer->tail = (ring_buffer->tail + amount_to_copy) % ring_buffer->size;

	return amount_to_copy;
}


void aliyun_iot_timer_delay(UINT32 delayMs)
{
	UINT32 timingdelay;
	UINT32 loop = 0;
	timingdelay = aliyun_iot_timer_now() + delayMs;
	while(aliyun_iot_timer_now() < timingdelay)
	{
		loop++;
	}
}

//clean uart buffer ,remove useless uart data
INT32 aliyun_iot_uart_buffer_clean()
{
	INT8 uartBuf[MQTT_RING_BUFFER_LEN] = {0};
	INT32 dataLen = 0;

	dataLen = aliyun_iot_uart_buffer_len();
	if(dataLen < MQTT_RING_BUFFER_LEN)
	{
		aliyun_iot_uart_recv(uartBuf, dataLen, 1000);
		memset(uartBuf, 0, sizeof(uartBuf));
	}
	else
	{
		WRITE_IOT_DEBUG_LOG("uart data len exceed 1024");
		aliyun_iot_uart_recv(uartBuf, MQTT_RING_BUFFER_LEN, 1000);
		memset(uartBuf, 0, sizeof(uartBuf));
	}
	return 0;
}

//Check AT command return value
INT32 aliyun_iot_wait_at_ack(INT8 *pAck, INT32 timeOutMs)
{
	INT8 recvBuf[64] = {0};
	INT32 dataLen = 0;
	
	ALIYUN_IOT_TIME_TYPE_S timer;
	aliyun_iot_timer_init(&timer);
	aliyun_iot_timer_cutdown(&timer, timeOutMs);

	if(!pAck)
	{
		WRITE_IOT_ERROR_LOG("ack buffer is null");
		return -1;
	}
	
	do
	{	
		//read whole uart buffer data
		dataLen = aliyun_iot_uart_buffer_len();
		if(dataLen)
		{
			aliyun_iot_uart_recv(recvBuf, dataLen, timeOutMs);
			WRITE_IOT_DEBUG_LOG("ack %s \n", recvBuf);
			
			if(strlen(recvBuf) && strstr(recvBuf, pAck))
			{
				//WRITE_IOT_DEBUG_LOG("match success \r\n");
				return 0;
			}
			else
			{
				aliyun_iot_timer_delay(100);
				memset(recvBuf, 0, sizeof(recvBuf));
			}
		}
		else
		{
			aliyun_iot_timer_delay(100);
		}
	}while(!aliyun_iot_timer_expired(&timer));

	return -1;
}


/*
	send at cmd by uart and check ack value
*/
INT32 aliyun_iot_send_at_cmd(INT8 *pCmd, INT8 *pAck, INT32 timeOutMs)
{
	if(!pCmd || !pAck)
	{
		WRITE_IOT_ERROR_LOG("param error, cmd or ack value is null");
		return -1;
	}

	if(aliyun_iot_uart_send(pCmd, strlen(pCmd)))
	{
		WRITE_IOT_ERROR_LOG("send at cmd failed %s", pCmd);
		return -1;
	}

	if(aliyun_iot_wait_at_ack(pAck, timeOutMs))
	{
		WRITE_IOT_ERROR_LOG("wait %s ack failed", pAck);
		return -1;
	}
	return 0;
}

/*

+CGATT: 1 // STATE: IP INITIAL(初始化)
AT+CSTT // STATE: IP START(启动任务)
AT+CIICR // STATE: IP GPRSACT(接受场景配置)
AT+CIFSR // STATE: IP STATUS?(获得本地IP)
AT+CIPSTART="TCP","116.236.221.75",7015
OK
CONNECT OK // STATE: CONNECT OK(连接建立成功)
AT+CIPCLOSE // STATE: TCP CLOSED(TCP连接断开，移动场景还处于激活状态)
AT+CIPSHUT // STATE: IP INITIAL(移动场景关闭，重新IP初始化)

sign up for mobile network and initialize uart serial port
the network is normal when device get ip address 
*/
INT32 aliyun_iot_phy_net_connect()
{
	INT8 cmd[64] = {0};
	INT32 loop = 0;

	//wait for chip register network
	aliyun_iot_timer_delay(2000);

	//mqtt ring buffer init
	aliyun_iot_ring_buffer_init(&mqtt_ring_buffer, (UINT8 *)mqtt_ring_data, MQTT_RING_BUFFER_LEN);

	//uart com port init
	if(aliyun_iot_uart_init())
	{
		WRITE_IOT_ERROR_LOG("uart init failed");
		return -1;
	}

	aliyun_iot_timer_delay(1000);
	
	//clean useless uart buffer data
	aliyun_iot_uart_buffer_clean();
	
	//close at echo function
	WRITE_IOT_DEBUG_LOG("cmd:%s", AT_GPRS_CLOSE_ECHO_FUNC_FMT);
	aliyun_iot_send_at_cmd(AT_GPRS_CLOSE_ECHO_FUNC_FMT, "OK", 1000);
	
	//check sim card is insert or not
	WRITE_IOT_DEBUG_LOG("cmd:%s", AT_GPRS_CHECK_SIM_CARD_FMT);
	loop = 0;
	do
	{
		if(aliyun_iot_send_at_cmd(AT_GPRS_CHECK_SIM_CARD_FMT, "READY", 5000))
		{
			loop++;
			if(loop == 3)
			{
				WRITE_IOT_ERROR_LOG("please check sim card");
				return -1;
			}
		}
		else
		{
			WRITE_IOT_DEBUG_LOG("sim card ready");
			break;
		}
	}while(loop < 3);
	
	//check device whether register network
	WRITE_IOT_DEBUG_LOG("cmd:%s", AT_GPRS_CHECK_REG_NET_FMT);
	if(aliyun_iot_send_at_cmd(AT_GPRS_CHECK_REG_NET_FMT, "0,", 1000))
	{
		WRITE_IOT_ERROR_LOG("please register mobile network");
		return -1;
	}

	//check whether gprs network is ok
	WRITE_IOT_DEBUG_LOG("cmd:%s", AT_GPRS_CHECK_GPRS_NET_NORMAL_FMT);
	if(aliyun_iot_send_at_cmd(AT_GPRS_CHECK_GPRS_NET_NORMAL_FMT, "CGATT: 1", 1000))
	{
		WRITE_IOT_DEBUG_LOG("cmd:%s", AT_GPRS_RESTART_RF_MODULE_FMT);

		//restart rf module
		aliyun_iot_uart_send(AT_GPRS_RESTART_RF_MODULE_FMT, strlen(AT_GPRS_RESTART_RF_MODULE_FMT));
		aliyun_iot_timer_delay(8000);
		loop = 0;
		do
		{
			if(aliyun_iot_send_at_cmd(AT_GPRS_CHECK_GPRS_NET_NORMAL_FMT, "CGATT: 1", 1000))
			{
				loop ++;
			}
			else
			{
				WRITE_IOT_DEBUG_LOG("CGATT: 1 success");
				break;
			}
			aliyun_iot_timer_delay(2000);
		}while(loop < 10);
	}
	
	//set apn
	WRITE_IOT_DEBUG_LOG("cmd:%s", AT_GPRS_SET_APN_FUNC_FMT);
	snprintf(cmd, sizeof(cmd), AT_GPRS_SET_APN_FUNC_FMT, "uninet");
	if(aliyun_iot_send_at_cmd(cmd, "OK", 1000))
	{
		WRITE_IOT_ERROR_LOG("set apn failed");
		return -1;
	}

	//activate 
	WRITE_IOT_DEBUG_LOG("cmd:%s", AT_GPRS_ACTIVATE_FMT);
	if(aliyun_iot_send_at_cmd(AT_GPRS_ACTIVATE_FMT, "OK", 70000))
	{
		WRITE_IOT_ERROR_LOG("activate network failed");
		return -1;
	}
	
	//send loop up ip address at cmd
	WRITE_IOT_DEBUG_LOG("cmd:%s", AT_GPRS_LOOPUP_IP_ADDR_FMT);
	if(aliyun_iot_send_at_cmd(AT_GPRS_LOOPUP_IP_ADDR_FMT, ".", 1000))
	{
		WRITE_IOT_DEBUG_LOG("device get ip address failed");
		return -1;
	}

	//set ip header
	WRITE_IOT_DEBUG_LOG("cmd:%s", AT_TCP_SOCKET_IP_HEADER_SET_FMT);
	if(aliyun_iot_send_at_cmd(AT_TCP_SOCKET_IP_HEADER_SET_FMT, "OK", 1000))
	{
		WRITE_IOT_ERROR_LOG("set ip header failed");
		return -1;
	}
	
	WRITE_IOT_DEBUG_LOG("gprs network init success");
	return 0;
	
}

INT32 aliyun_iot_mqtt_net_connect(INT8 *pHost, INT8 *pPort)
{
	INT8 cmd[64] = {0};
	
	//send establish tcp connection at cmd
	snprintf(cmd, sizeof(cmd), AT_TCP_SOCKET_CONNECT_FMT, pHost, atoi(pPort));
	
	WRITE_IOT_DEBUG_LOG("cmd:%s", cmd);
	if(aliyun_iot_send_at_cmd(cmd, "CONNECT OK", 5000))
	{
		WRITE_IOT_ERROR_LOG("connect iot cloud failed");
		return -1;
	}

	WRITE_IOT_DEBUG_LOG("connect iot cloud success");

	return 0;
}


//+IPD,<len>:<data>
INT32 aliyun_iot_mqtt_nettype_read(Network *pNet, UINT8 *pRecvBuffer, INT32 recvBufferlen, INT32 timeOutMs)
{
	if(!pRecvBuffer)
	{
		WRITE_IOT_ERROR_LOG("param error, recvbuffer is null");
		return -1;
	}

	WRITE_IOT_DEBUG_LOG("mqtt_read: recvBufLen=%d timeout=%d", recvBufferlen, timeOutMs);
	
	INT32 readLen = 0;
	INT8  uartMsg[MQTT_RING_BUFFER_LEN] = {0};
	INT32 uartMsgLen = 0;
	INT32 msgLen = 0;
	INT8  *mqttMsg = NULL;
	INT8  tmp[8] = {0};
	INT32 pos = 0;

	ALIYUN_IOT_TIME_TYPE_S timer;
	aliyun_iot_timer_init(&timer);
	aliyun_iot_timer_cutdown(&timer, timeOutMs);

	do
	{
		INT32 dataLen = aliyun_iot_ring_buffer_used_space(&mqtt_ring_buffer);
		if(dataLen)
		{
			// read mqtt data from ringbuffer
			readLen = (dataLen > recvBufferlen) ? recvBufferlen : dataLen;
			aliyun_iot_ring_buffer_read(&mqtt_ring_buffer, pRecvBuffer, readLen);
			return readLen;
		}
		else // read mqtt data from uart. then put uart data into ringbuffer
		{
			uartMsgLen = aliyun_iot_uart_buffer_len();
			//WRITE_IOT_DEBUG_LOG("mqtt_read: uartMsgLen=%d \n", uartMsgLen);
			if(uartMsgLen)
			{
				if(!aliyun_iot_uart_recv(uartMsg, uartMsgLen, 2000) 
					&& strlen(uartMsg))
				{
					WRITE_IOT_DEBUG_LOG("recv msg:%s len=%d \n", uartMsg, strlen(uartMsg));

					mqttMsg = strstr(uartMsg, "+IPD");
					if(mqttMsg)
					{
						//resolve mqtt msg len
						mqttMsg += 5;
						while((*mqttMsg) != ':')
						{
							tmp[pos] = *mqttMsg;
							pos++;
							mqttMsg++;
						}
						mqttMsg++;
						msgLen = atoi(tmp);
						
						//WRITE_IOT_DEBUG_LOG("recv msg len:%d, real msg len:%d \n", msgLen, strlen(mqttMsg));			
						if(msgLen)
						{
							//put mqtt data into ringbuffer
							aliyun_iot_ring_buffer_write(&mqtt_ring_buffer, (const unsigned char*)mqttMsg, msgLen);
							readLen = (msgLen > recvBufferlen) ? recvBufferlen : msgLen;
							aliyun_iot_ring_buffer_read(&mqtt_ring_buffer, pRecvBuffer, readLen);
							return readLen;
						}
					}
					else
					{
						WRITE_IOT_DEBUG_LOG("not match IPD");
						memset(uartMsg, 0, sizeof(uartMsg));
					}
					
				}
				else
				{
					WRITE_IOT_DEBUG_LOG("read uart data failed");
				}
			}
			else
			{
				//WRITE_IOT_DEBUG_LOG("uart buffer is empty");
				aliyun_iot_timer_delay(500);
			}
		}
	}while(!aliyun_iot_timer_expired(&timer));
	
	return -1;
}


INT32 aliyun_iot_mqtt_nettype_write(Network *pNet, UINT8 *pSendBuffer, INT32 sendBufferlen, INT32 timeOutMs)
{
	WRITE_IOT_DEBUG_LOG("mqtt_write: dataLen=%d timeout=%d", sendBufferlen, timeOutMs);

	INT8 cmd[64] = {0};

	if(!pSendBuffer)
	{
		WRITE_IOT_ERROR_LOG("param error, sendbuffer is null");
		return -1;
	}

	//send tcp data at cmd
	snprintf(cmd, sizeof(cmd), AT_TCP_SOCKET_SEND_DATA_FMT, sendBufferlen);	
	WRITE_IOT_DEBUG_LOG("cmd:%s", cmd);

	if(aliyun_iot_uart_send(cmd, strlen(cmd)))
	{
		WRITE_IOT_ERROR_LOG("mqtt_write: send data at cmd failed");
		return -1;
	}
	
	//wait > , then send mqtt data
	aliyun_iot_timer_delay(1000);

	//send mqtt data
	if(aliyun_iot_uart_send((char*)pSendBuffer, sendBufferlen))
	{
		WRITE_IOT_ERROR_LOG("mqtt_write: send mqtt data failed ");
		return -1;
	}
	else
	{
		WRITE_IOT_DEBUG_LOG("mqtt_write: send mqtt data success");
	}
	
	return sendBufferlen;
}

INT32 aliyun_iot_mqtt_nettype_connect(Network *pNet)
{
	return aliyun_iot_mqtt_net_connect(pNet->connectparams.pHostAddress, pNet->connectparams.pHostPort);
}

void aliyun_iot_mqtt_nettype_disconnect(Network *pNet)
{
	INT8 cmd[32] = {0};
	snprintf(cmd, sizeof(cmd), AT_TCP_SOCKET_CLOSE_FMT);
	aliyun_iot_uart_send(cmd, strlen(cmd));
	aliyun_iot_uart_send(AT_TCP_SOCKET_SHUTDOWN_FMT, strlen(AT_TCP_SOCKET_SHUTDOWN_FMT));
	aliyun_iot_timer_delay(3000);
}



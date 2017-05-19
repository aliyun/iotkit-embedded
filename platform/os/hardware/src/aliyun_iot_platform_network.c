
#include "aliyun_iot_platform_network.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_mqtt_nettype.h"

INT32 aliyun_iot_get_errno(void)
{
    return EAGAIN_IOT;
}

INT32 aliyun_iot_network_send(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags)
{
	return aliyun_iot_mqtt_nettype_write(NULL, (unsigned char *)buf, nbytes, 2000);
}

INT32 aliyun_iot_network_recv(INT32 sockFd, void *buf, INT32 nbytes, UINT32 flags)
{
	return aliyun_iot_mqtt_nettype_read(NULL, (unsigned char *)buf, nbytes, 5000);
}

INT32 aliyun_iot_network_select(INT32 fd,IOT_NET_TRANS_TYPE_E type,int timeoutMs,IOT_NET_FD_ISSET_E* result)
{
	return 0;
}

INT32 aliyun_iot_network_settimeout(INT32 fd,int timeoutMs,IOT_NET_TRANS_TYPE_E type)
{
	return 0;
}

INT32 aliyun_iot_network_get_nonblock(INT32 fd)
{
	return 0;
}

INT32 aliyun_iot_network_set_nonblock(INT32 fd)
{
	return 0;
}

INT32 aliyun_iot_network_set_block(INT32 fd)
{
	return 0;
}

INT32 aliyun_iot_network_close(INT32 fd)
{
    aliyun_iot_mqtt_nettype_disconnect(NULL);
	return 0;
}

INT32 aliyun_iot_network_shutdown(INT32 fd,INT32 how)
{
    return 0;
}

INT32 aliyun_iot_network_create(const INT8*host,const INT8*service,IOT_NET_PROTOCOL_TYPE type)
{	
	WRITE_IOT_ERROR_LOG("host:%s, port:%s", host, service);
	if(aliyun_iot_mqtt_net_connect((char*)host, (char*)service))
	{
		WRITE_IOT_ERROR_LOG("connect network failed ");
		return -1;
	}
	WRITE_IOT_ERROR_LOG("http connect success");
	return 1;
}

INT32 aliyun_iot_network_bind(const INT8*host,const INT8*service,IOT_NET_PROTOCOL_TYPE type)
{
	return 0;
}



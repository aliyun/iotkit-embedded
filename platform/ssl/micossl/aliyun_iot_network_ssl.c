#include "aliyun_iot_network_ssl.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_error.h"

extern int CyaSSL_pending();
int aliyun_iot_network_ssl_read(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms)
{
	int rc = 0;
	int readLen = 0;
	int ret = -1;

	WRITE_IOT_DEBUG_LOG("ssl_recv len=%d timer=%d ms", len, timeout_ms);

	if(!CyaSSL_pending(pTlsData->ssl)) 
	{
		IOT_NET_FD_ISSET_E result;
		ret = aliyun_iot_network_select(pTlsData->socketId, IOT_NET_TRANS_RECV, timeout_ms, &result);
		if(ret < 0)
		{
			INT32 err = aliyun_iot_get_errno();
			if(err == EINTR_IOT)
			{
				WRITE_IOT_DEBUG_LOG("continue");
			}
			else
			{
				WRITE_IOT_ERROR_LOG("mqtt read(select) fail ret=%d", ret);
				return -1;
			}

		}
		else if (ret == 0)
		{
			WRITE_IOT_ERROR_LOG("mqtt read(select) timeout");
			return -2;
		}
		else if (ret == 1)
		{
			WRITE_IOT_DEBUG_LOG("start to read packet");
		}
	}

	while (readLen < len) 
	{
		ret = ssl_recv(pTlsData->ssl, (unsigned char *)(buffer + readLen), (len - readLen));
		if (ret > 0) 
		{
			readLen += ret;
		} 
		else if (ret == 0) 
		{
			WRITE_IOT_ERROR_LOG("ssl_recv read timeout");
			return -2; //eof
		} 
		else 
		{
			WRITE_IOT_ERROR_LOG("ssl_recv error ret = -0x%x", ret);
			return -1; //Connnection error
		}
	}

	WRITE_IOT_DEBUG_LOG("ssl_recv readlen=%d", readLen);
	return readLen;
}


int aliyun_iot_network_ssl_write(TLSDataParams *pTlsData, unsigned char *buffer, int len, int timeout_ms)
{
	int writtenLen = 0;
	int ret = 0;

	WRITE_IOT_DEBUG_LOG("ssl_send len=%d timer=%d", len, timeout_ms);
	while (writtenLen < len) 
	{
		ret = ssl_send(pTlsData->ssl, (unsigned char *)(buffer + writtenLen), (len - writtenLen));
		if (ret > 0) 
		{
			writtenLen += ret;
			continue;
		} 
		else if (ret == 0) 
		{
			WRITE_IOT_ERROR_LOG("ssl_send write timeout");
			return writtenLen;
		} 
		else 
		{
			WRITE_IOT_ERROR_LOG("ssl_send write fail ret= -0x%x% ", ret);
			return -1; //Connnection error
		}
	}
	WRITE_IOT_DEBUG_LOG("ssl_send write len=%d", writtenLen);
	return writtenLen;
}

void aliyun_iot_network_ssl_disconnect(TLSDataParams *pTlsData)
{
	if(pTlsData->ssl != NULL)
	{
		ssl_close(pTlsData->ssl);
	}
	close(pTlsData->socketId);
}

int aliyun_iot_network_ssl_connect(TLSDataParams *pTlsData, const char *addr, const char *port, const char *ca_crt, size_t ca_crt_len)
{
    int rc = 0;
	int err = 0;

	if(NULL == pTlsData)
	{
		WRITE_IOT_ERROR_LOG("network is null");
		return 1;
	}

	pTlsData->socketId = aliyun_iot_network_create(addr, port, IOT_NET_PROTOCOL_TCP);
	if(pTlsData->socketId < 0)
	{
		pTlsData->ssl = NULL;
		WRITE_IOT_ERROR_LOG("MQTT TLS network create failed");
		return -1;
	}
	
	ssl_version_set(TLS_V1_2_MODE);

    pTlsData->ssl = ssl_connect(pTlsData->socketId, 0, (char *)ca_crt, &err); 
	if(NULL ==  pTlsData->ssl)
	{
		WRITE_IOT_ERROR_LOG("MQTT TLS Connect error = %d", err);
		return err;
	}

	WRITE_IOT_DEBUG_LOG("MQTT TLS Connect Success");
	return 0;

}



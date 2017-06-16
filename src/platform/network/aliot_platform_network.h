

#ifndef _ALIOT_PLATFORM_NETWORK_H_
#define _ALIOT_PLATFORM_NETWORK_H_

#include "../aliot_platform_datatype.h"

intptr_t aliot_platform_tcp_establish(const char *host, uint16_t port);
int aliot_platform_tcp_destroy(intptr_t fd);
int32_t aliot_platform_tcp_write(intptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms);



//@timeout_ms: 0, not wait.

//-2, error;
//-1, be closed by remote server;
//0, timeout without any data
//[1, len), timeout with
//len, received specific data from server.
int32_t aliot_platform_tcp_read(intptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms);

#endif /* _ALIOT_PLATFORM_NETWORK_H_ */

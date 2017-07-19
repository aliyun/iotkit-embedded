

#ifndef _IOTX_PLATFORM_NETWORK_H_
#define _IOTX_PLATFORM_NETWORK_H_

#include "../platform_datatype.h"


/**
 * @brief Establish a TCP connection.
 *
 * @param [in] host: @n Specify the hostname(IP) of the TCP server
 * @param [in] port: @n Specify the TCP port of TCP server
 *
 * @return 0, fail; > 0, success, the value is handle of this TCP connection.
 */
uintptr_t iotx_platform_tcp_establish(const char *host, uint16_t port);


/**
 * @brief Destroy the specific TCP connection.
 *
 * @param [in] fd: @n Specify the TCP connection by handle.
 *
 * @return < 0, fail; 0, success.
 */
int32_t iotx_platform_tcp_destroy(uintptr_t fd);


/**
 * @brief Write data into the specific TCP connection.
 *        The API will return immediately if @len be written into the specific TCP connection.
 *
 * @param [in] fd @n A descriptor identifying a connection.
 * @param [in] buf @n A pointer to a buffer containing the data to be transmitted.
 * @param [in] len @n The length, in bytes, of the data pointed to by the @buf parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block @timeout_ms millisecond maximumly.
 * @return
   @verbatim
        < 0 : TCP connection error occur..
          0 : No any data be write into the TCP connection in @timeout_ms timeout period.
   (0, len] : The total number of bytes be written in @timeout_ms timeout period.
   @endverbatim
 * @see None.
 */
int32_t iotx_platform_tcp_write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms);


/**
 * @brief Read data from the specific TCP connection with timeout parameter.
 *        The API will return immediately if @len be received from the specific TCP connection.
 *
 * @param [in] fd @n A descriptor identifying a TCP connection.
 * @param [in] buf @n A pointer to a buffer to receive incoming data.
 * @param [in] len @n The length, in bytes, of the data pointed to by the @buf parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block @timeout_ms millisecond maximumly.
 * @return
   @verbatim
         -2 : TCP connection error occur.
         -1 : TCP connection be closed by remote server.
          0 : No any data be received in @timeout_ms timeout period.
   (0, len] : The total number of bytes be received in @timeout_ms timeout period.
   @endverbatim
 * @see None.
 */
int32_t iotx_platform_tcp_read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms);


#endif /* _IOTX_PLATFORM_NETWORK_H_ */

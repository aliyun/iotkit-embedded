

#ifndef _ALIOT_PLATFORM_SSL_H_
#define _ALIOT_PLATFORM_SSL_H_


/**
 * @brief Establish a SSL connection.
 *
 * @param [in] host: @n Specify the hostname(IP) of the SSL server
 * @param [in] port: @n Specify the SSL port of SSL server
 * @param [in] ca_crt @n Specify the root certificate which is PEM format.
 * @param [in] ca_crt_len @n Length of root certificate, in bytes.
 * @return SSL handle.
 * @see None.
 * @note None.
 */
uintptr_t aliot_platform_ssl_establish(
                const char *host,
                uint16_t port,
                const char *ca_crt,
                size_t ca_crt_len);


/**
 * @brief Destroy the specific SSL connection.
 *
 * @param[in] handle: @n Handle of the specific connection.
 *
 * @return < 0, fail; 0, success.
 */
int32_t aliot_platform_ssl_destroy(uintptr_t handle);


/**
 * @brief Write data into the specific SSL connection.
 *        The API will return immediately if @len be written into the specific SSL connection.
 *
 * @param [in] fd @n A descriptor identifying a connection.
 * @param [in] buf @n A pointer to a buffer containing the data to be transmitted.
 * @param [in] len @n The length, in bytes, of the data pointed to by the @buf parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block @timeout_ms millisecond maximumly.
 * @return
   @verbatim
        < 0 : SSL connection error occur..
          0 : No any data be write into the SSL connection in @timeout_ms timeout period.
   (0, len] : The total number of bytes be written in @timeout_ms timeout period.
   @endverbatim
 * @see None.
 */
int32_t aliot_platform_ssl_write(uintptr_t handle, const char *buf, int len, int timeout_ms);


/**
 * @brief Read data from the specific SSL connection with timeout parameter.
 *        The API will return immediately if @len be received from the specific SSL connection.
 *
 * @param [in] fd @n A descriptor identifying a SSL connection.
 * @param [in] buf @n A pointer to a buffer to receive incoming data.
 * @param [in] len @n The length, in bytes, of the data pointed to by the @buf parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block @timeout_ms millisecond maximumly.
 * @return
   @verbatim
         -2 : SSL connection error occur.
         -1 : SSL connection be closed by remote server.
          0 : No any data be received in @timeout_ms timeout period.
   (0, len] : The total number of bytes be received in @timeout_ms timeout period.
   @endverbatim
 * @see None.
 */
int32_t aliot_platform_ssl_read(uintptr_t handle, char *buf, int len, int timeout_ms);


#endif /* _ALIOT_PLATFORM_SSL_H_ */

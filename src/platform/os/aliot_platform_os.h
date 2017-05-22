
#ifndef _ALIOT_PLATFORM_OS_H_
#define _ALIOT_PLATFORM_OS_H_

#include "../aliot_platform_datatype.h"

/** @defgroup group_platform platform
 *  @{
 */


#define _IN_            /**< indicate that this is a input parameter. */
#define _OUT_           /**< indicate that this is a output parameter. */
#define _INOUT_         /**< indicate that this is a io parameter. */
#define _IN_OPT_        /**< indicate that this is a optional input parameter. */
#define _OUT_OPT_       /**< indicate that this is a optional output parameter. */
#define _INOUT_OPT_     /**< indicate that this is a optional io parameter. */


/*********************************** mutex interface ***********************************/

/** @defgroup group_platform_mutex mutex
 *  @{
 */

/**
 * @brief Create a mutex.
 *
 * @return NULL, initialize mutex failed; not NULL, the mutex handle.
 * @see None.
 * @note None.
 */
void *aliot_platform_mutex_create(void);



/**
 * @brief Destroy the specified mutex object, it will free related resource.
 *
 * @param [in] mutex @n The specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void aliot_platform_mutex_destroy(_IN_ void *mutex);



/**
 * @brief Waits until the specified mutex is in the signaled state.
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void aliot_platform_mutex_lock(_IN_ void *mutex);



/**
 * @brief Releases ownership of the specified mutex object..
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void aliot_platform_mutex_unlock(_IN_ void *mutex);


/** @} */ //end of platform_mutex


/** @defgroup group_platform_memory_manage memory
 *  @{
 */

/**
 * @brief Allocates a block of size bytes of memory, returning a pointer to the beginning of the block.
 *
 * @param [in] size @n specify block size in bytes.
 * @return A pointer to the beginning of the block.
 * @see None.
 * @note Block value is indeterminate.
 */
void *aliot_platform_malloc(_IN_ uint32_t size);


/**
 * @brief Deallocate memory block
 *
 * @param[in] ptr @n Pointer to a memory block previously allocated with platform_malloc.
 * @return None.
 * @see None.
 * @note None.
 */
void aliot_platform_free(_IN_ void *ptr);


/** @} */ //end of platform_memory_manage


/** @defgroup group_platform_ota ota
 *  @{
 */

/**
 * @brief Initialize a OTA upgrade.
 *
 * @param None
 * @return 0, success; -1, failure.
 * @see None.
 * @note None.
 */
int aliot_platform_ota_start(const char *md5, uint32_t file_size);


/**
 * @brief Write OTA data.
 *
 * @param [in] buffer: @n A pointer to a buffer to save data.
 * @param [in] length: @n The length, in bytes, of the data pointed to by the buffer parameter.
 * @return 0, success; -1, failure.
 * @see None.
 * @note None.
 */
int aliot_platform_ota_write(_IN_ char *buffer, _IN_ uint32_t length);


/**
 * @brief indicate OTA complete.
 *
 * @param [in] stat: 0, normal termination; -1, abnormal termination (error occur).
 * @return 0: suuccess; -1: failure.
 * @see None.
 * @note None.
 */
int aliot_platform_ota_finalize(_IN_ int stat);


/** @} */ //end of group_platform_ota


/** @defgroup group_platform_other other
 *  @{
 */

/**
 * @brief Retrieves the number of milliseconds that have elapsed since the system was boot.
 *
 * @param None.
 * @return the number of milliseconds.
 * @see None.
 * @note None.
 */
uint32_t aliot_platform_time_get_ms(void);


/**
 * @brief sleep thread itself.
 *
 * @param [in] ms @n the time interval for which execution is to be suspended, in milliseconds.
 * @return None.
 * @see None.
 * @note None.
 */
void aliot_platform_msleep(_IN_ uint32_t ms);


/**
 * @brief Writes formatted data to stream.
 *
 * @param [in] fmt: @n String that contains the text to be written, it can optionally contain embedded format specifiers
     that specifies how subsequent arguments are converted for output.
 * @param [in] ...: @n the variable argument list, for formatted and inserted in the resulting string replacing their respective specifiers.
 * @return None.
 * @see None.
 * @note None.
 */
void aliot_platform_printf(_IN_ const char *fmt, ...);


/**
 * @brief Get vendor ID of hardware module.
 *
 * @return NULL, Have NOT PID; NOT NULL, point to pid_str.
 */
char *aliot_platform_module_get_pid(char pid_str[]);


/** @} */ //end of group_platform_other

#endif /* _ALIOT_PLATFORM_OS_H_ */

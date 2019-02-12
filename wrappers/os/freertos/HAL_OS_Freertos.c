/**
 * NOTE:
 *
 * HAL_TCP_xxx API reference implementation: wrappers/os/ubuntu/HAL_TCP_linux.c
 *
 */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "infra_types.h"
#include "infra_defs.h"
#include "wrappers_defs.h"
#include "at_wrapper.h"

#define EXAMPLE_PRODUCT_KEY         "a1X2bEnP82z"
#define EXAMPLE_PRODUCT_SECRET      "7jluWm1zql7bt8qK"
#define EXAMPLE_DEVICE_NAME         "example1"
#define EXAMPLE_DEVICE_SECRET       "ga7XA6KdlEeiPXQPpRbAjOZXwG8ydgSe"

#define EXAMPLE_FIRMWARE_VERSION    "app-1.0.0-20190118.1000"

#define RING_BUFFER_SIZE            (128)


#define HAL_SEM_MAX_COUNT           (10)
#define HAL_SEM_INIT_COUNT          (0)

#define DEFAULT_THREAD_NAME         "linkkit_task"
#define DEFAULT_THREAD_SIZE         (256)
#define TASK_STACK_ALIGN_SIZE       (4)


typedef struct
{
  uint8_t  data[RING_BUFFER_SIZE];
  uint16_t tail; 
  uint16_t head;
}uart_ring_buffer_t;

extern UART_HandleTypeDef huart1;

static uint8_t  g_uart_tx_flag;
static uart_ring_buffer_t   g_uart_rx_buf;
static UART_HandleTypeDef* p_at_uart = &huart1;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (++g_uart_rx_buf.tail >= RING_BUFFER_SIZE){
        g_uart_rx_buf.tail = 0;   
    }
    HAL_UART_Receive_IT(p_at_uart, (uint8_t *)&g_uart_rx_buf.data[g_uart_rx_buf.tail], 1);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    g_uart_tx_flag = 1;
}

/**
 * Deinitialises a UART interface
 *
 * @param[in]  uart  the interface which should be deinitialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t HAL_AT_Uart_Deinit(uart_dev_t *uart)
{
    HAL_StatusTypeDef ret;
    ret = HAL_UART_DeInit(p_at_uart);
    if(HAL_OK != ret){
        return -1;
    }
    return 0;
}


/**
 * Initialises a UART interface
 *
 *
 * @param[in]  uart  the interface which should be initialised
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t HAL_AT_Uart_Init(uart_dev_t *uart)
{
    char c;
    HAL_StatusTypeDef ret;
    if (uart == NULL) {
        return -1;
    }

    g_uart_rx_buf.head = 0;
    g_uart_rx_buf.tail = 0;
    ret = HAL_UART_Receive_IT(p_at_uart, (uint8_t *)&c, 1);
    if(HAL_OK != ret){
        return -1;
    }
    return 0;
}


/**
 * Receive data on a UART interface
 *
 * @param[in]   uart         the UART interface
 * @param[out]  data         pointer to the buffer which will store incoming data
 * @param[in]   expect_size  number of bytes to receive
 * @param[out]  recv_size    number of bytes received
 * @param[in]   timeout      timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                           if you want to wait forever
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t HAL_AT_Uart_Recv(uart_dev_t *uart, void *data, uint32_t expect_size,
                         uint32_t *recv_size, uint32_t timeout)
{
    uint32_t ReadData = 0;
    uint8_t *buf = (uint8_t *)data;
    
    /* Loop until data received */
    while (expect_size--)
    {
      uint32_t tickStart = xTaskGetTickCount();
      do
      {
        if(g_uart_rx_buf.head != g_uart_rx_buf.tail)
        {
          /* serial data available, so return data to user */
          *buf++ = g_uart_rx_buf.data[g_uart_rx_buf.head++];
          ReadData++;
    
          /* check for ring buffer wrap */
          if (g_uart_rx_buf.head >= RING_BUFFER_SIZE)
          {
            /* Ring buffer wrap, so reset head pointer to start of buffer */
            g_uart_rx_buf.head = 0;
          }
          break;
        }
      }while((xTaskGetTickCount() - tickStart ) < timeout);
    }
    
    *recv_size = ReadData;
    
    return 0;
}


/**
 * Transmit data on a UART interface
 *
 * @param[in]  uart     the UART interface
 * @param[in]  data     pointer to the start of data
 * @param[in]  size     number of bytes to transmit
 * @param[in]  timeout  timeout in milisecond, set this value to HAL_WAIT_FOREVER
 *                      if you want to wait forever
 *
 * @return  0 : on success, EIO : if an error occurred with any step
 */
int32_t HAL_AT_Uart_Send(uart_dev_t *uart, const void *data, uint32_t size, uint32_t timeout)
{
    uint32_t tickstart;
    g_uart_tx_flag = 0;
    
    if (HAL_UART_Transmit_IT(p_at_uart, (uint8_t*)data, size) != HAL_OK)
    {
       return -1;
    }
    
    tickstart = xTaskGetTickCount();
    while(g_uart_tx_flag!=1)
    {
      if((xTaskGetTickCount()-tickstart) > timeout) {
        return -1;
      }
    }
    
    return 0;
}


/**
 * @brief Deallocate memory block
 *
 * @param[in] ptr @n Pointer to a memory block previously allocated with platform_malloc.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_Free(void *ptr)
{
    vPortFree(ptr);
}


/**
 * @brief Get device name from user's system persistent storage
 *
 * @param [ou] device_name: array to store device name, max length is IOTX_DEVICE_NAME_LEN
 * @return the actual length of device name
 */
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN])
{
    int len = strlen(EXAMPLE_DEVICE_NAME);
    if(len >= IOTX_DEVICE_NAME_LEN){
        return -1;
    }
    memset(device_name, 0x0, IOTX_DEVICE_NAME_LEN); 
    strncpy(device_name, EXAMPLE_DEVICE_NAME, len); 
    return strlen(device_name);
}


/**
 * @brief Get device secret from user's system persistent storage
 *
 * @param [ou] device_secret: array to store device secret, max length is IOTX_DEVICE_SECRET_LEN
 * @return the actual length of device secret
 */
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN])
{
    int len = strlen(EXAMPLE_DEVICE_SECRET); 
    if(len >= IOTX_DEVICE_SECRET_LEN){
        return -1;
    }
    
    memset(device_secret, 0x0, IOTX_DEVICE_SECRET_LEN); 
    strncpy(device_secret, EXAMPLE_DEVICE_SECRET, len); 
    return len;
}


/**
 * @brief Get firmware version
 *
 * @param [ou] version: array to store firmware version, max length is IOTX_FIRMWARE_VER_LEN
 * @return the actual length of firmware version
 */
int HAL_GetFirmwareVersion(char *version)
{
    int len = strlen(EXAMPLE_FIRMWARE_VERSION);
    if(len >= IOTX_FIRMWARE_VER_LEN-1){
        return -1;
    }
    memset(version, 0x0, IOTX_FIRMWARE_VER_LEN);
    strncpy(version, EXAMPLE_FIRMWARE_VERSION, len);
    version[len] = '\0';
    return strlen(version);

}


/**
 * @brief Get product key from user's system persistent storage
 *
 * @param [ou] product_key: array to store product key, max length is IOTX_PRODUCT_KEY_LEN
 * @return  the actual length of product key
 */
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN])
{
    int len = strlen(EXAMPLE_PRODUCT_KEY); 
    if(len >= IOTX_PRODUCT_KEY_LEN){
        return -1;
    }
    memset(product_key, 0x0, IOTX_PRODUCT_KEY_LEN); 
    strncpy(product_key, EXAMPLE_PRODUCT_KEY, len); 
    return len;
}

/**
 * @brief Allocates a block of size bytes of memory, returning a pointer to the beginning of the block.
 *
 * @param [in] size @n specify block size in bytes.
 * @return A pointer to the beginning of the block.
 * @see None.
 * @note Block value is indeterminate.
 */
void *HAL_Malloc(uint32_t size)
{
    return pvPortMalloc(size);
}


/**
 * @brief Create a mutex.
 *
 * @retval NULL : Initialize mutex failed.
 * @retval NOT_NULL : The mutex handle.
 * @see None.
 * @note None.
 */
void *HAL_MutexCreate(void)
{
    QueueHandle_t sem;

    sem = xSemaphoreCreateMutex();
    if (0 == sem){
        return NULL;
    }

    return sem;
}


/**
 * @brief Destroy the specified mutex object, it will release related resource.
 *
 * @param [in] mutex @n The specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexDestroy(void *mutex)
{   
    QueueHandle_t sem;
    if(mutex == NULL){
        return;
    }
    sem = (QueueHandle_t )mutex;
    vSemaphoreDelete(sem);
}


/**
 * @brief Waits until the specified mutex is in the signaled state.
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexLock(void *mutex)
{
    BaseType_t ret;
    QueueHandle_t sem;
    if(mutex == NULL){
        return;
    }
    
    sem = (QueueHandle_t)mutex;
    ret = xSemaphoreTake(sem,0xffffffff);
    while(pdPASS != ret){
        ret = xSemaphoreTake(sem, 0xffffffff);
    }
}


/**
 * @brief Releases ownership of the specified mutex object..
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexUnlock(void *mutex)
{
    QueueHandle_t sem;
    if(mutex == NULL){
        return;
    }
    sem = (QueueHandle_t)mutex;
    (void)xSemaphoreGive(sem);    
}


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
void HAL_Printf(const char *fmt, ...)
{
    va_list args; 
    va_start(args, fmt); 
    vprintf(fmt, args); 
    va_end(args); 
    fflush(stdout);
}


/**
 * @brief   create a semaphore
 *
 * @return semaphore handle.
 * @see None.
 * @note The recommended value of maximum count of the semaphore is 255.
 */
void *HAL_SemaphoreCreate(void)
{
    QueueHandle_t sem = 0; 
    sem = xSemaphoreCreateCounting(HAL_SEM_MAX_COUNT,HAL_SEM_INIT_COUNT);
    if (0 == sem){
        return NULL;
    }
    
    return sem;
}


/**
 * @brief   destory a semaphore
 *
 * @param[in] sem @n the specified sem.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SemaphoreDestroy(void *sem)
{
    QueueHandle_t queue;

    if(sem == NULL){
        return;
    }
    queue = (QueueHandle_t )sem;
    
    vSemaphoreDelete(queue);
}


/**
 * @brief   signal thread wait on a semaphore
 *
 * @param[in] sem @n the specified semaphore.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SemaphorePost(void *sem)
{
    QueueHandle_t queue;
    if(sem == NULL){
        return;
    }
    queue = (QueueHandle_t)sem;
    (void)xSemaphoreGive(queue);    
}


/**
 * @brief   wait on a semaphore
 *
 * @param[in] sem @n the specified semaphore.
 * @param[in] timeout_ms @n timeout interval in millisecond.
     If timeout_ms is PLATFORM_WAIT_INFINITE, the function will return only when the semaphore is signaled.
 * @return
   @verbatim
   =  0: The state of the specified object is signaled.
   =  -1: The time-out interval elapsed, and the object's state is nonsignaled.
   @endverbatim
 * @see None.
 * @note None.
 */
int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms)
{
    BaseType_t ret = 0;
    QueueHandle_t queue;
    if(sem == NULL){
        return -1;
    }
    
    queue = (QueueHandle_t)sem;
    ret = xSemaphoreTake(queue,timeout_ms);
    if(pdPASS != ret){
        return -1;
    }
    return 0;
}


/**
 * @brief Sleep thread itself.
 *
 * @param [in] ms @n the time interval for which execution is to be suspended, in milliseconds.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SleepMs(uint32_t ms)
{
    osDelay(ms);
}


/**
 * @brief Writes formatted data to string.
 *
 * @param [out] str: @n String that holds written text.
 * @param [in] len: @n Maximum length of character will be written
 * @param [in] fmt: @n Format that contains the text to be written, it can optionally contain embedded format specifiers
     that specifies how subsequent arguments are converted for output.
 * @param [in] ...: @n the variable argument list, for formatted and inserted in the resulting string replacing their respective specifiers.
 * @return bytes of character successfully written into string.
 * @see None.
 * @note None.
 */
int HAL_Snprintf(char *str, const int len, const char *fmt, ...)
{
    va_list args; 
    int rc; 

    va_start(args, fmt); 
    rc = vsnprintf(str, len, fmt, args); 
    va_end(args); 
    return rc;
}


/**
 * @brief  create a thread
 *
 * @param[out] thread_handle @n The new thread handle, memory allocated before thread created and return it, free it after thread joined or exit.
 * @param[in] start_routine @n A pointer to the application-defined function to be executed by the thread.
        This pointer represents the starting address of the thread.
 * @param[in] arg @n A pointer to a variable to be passed to the start_routine.
 * @param[in] hal_os_thread_param @n A pointer to stack params.
 * @param[out] stack_used @n if platform used stack buffer, set stack_used to 1, otherwise set it to 0.
 * @return
   @verbatim
     = 0: on success.
     = -1: error occur.
   @endverbatim
 * @see None.
 * @note None.
 */
int HAL_ThreadCreate(
                     void **thread_handle,
                     void *(*work_routine)(void *),
                     void *arg,
                     hal_os_thread_param_t *hal_os_thread_param,
                     int *stack_used)
{
    char *name;
    size_t stacksize;
    //osPriority prio;
    osThreadDef_t thread_def;
    
    osThreadId handle;
    
    if (thread_handle == NULL){
        return -1;
    }
    
    if (work_routine == NULL){
        return -1;
    }
    
    if (hal_os_thread_param == NULL){
        return -1;
    }
    if (stack_used == NULL){
        return -1;
    }
    
    if (stack_used != NULL) {
        *stack_used = 0;
    }
    
    if (!hal_os_thread_param->name) {
        name = DEFAULT_THREAD_NAME;
    } else {
        name = hal_os_thread_param->name;
    }

    if (hal_os_thread_param->stack_size == 0) {
        stacksize = DEFAULT_THREAD_SIZE;
    } else {
        stacksize = hal_os_thread_param->stack_size;
    }

    thread_def.name = name;
    thread_def.pthread = (os_pthread)work_routine;
    thread_def.tpriority = (osPriority)hal_os_thread_param->priority;
    thread_def.instances = 0;
    thread_def.stacksize = (stacksize + TASK_STACK_ALIGN_SIZE-1)/TASK_STACK_ALIGN_SIZE;

    handle = osThreadCreate(&thread_def, arg);
    if( NULL == handle){
        return -1;
    }
    *thread_handle = (void*)handle;
    return 0;
}


/**
 * @brief Retrieves the number of milliseconds that have elapsed since the system was boot.
 *
 * @return the number of milliseconds.
 * @see None.
 * @note None.
 */
uint64_t HAL_UptimeMs(void)
{
    return (uint64_t)xTaskGetTickCount();
}



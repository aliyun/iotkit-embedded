#ifndef _ATPARSER_OPTS_H_
#define _ATPARSER_OPTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "infra_config.h"

#define AT_UART_PORT 1
#define AT_UART_LINUX_DEV "/dev/ttyUSB0"

/* AT command */
#define AT_CMD_ENET_SEND       "AT+ENETRAWSEND"
#define AT_CMD_ENTER_ENET_MODE "AT+ENETRAWMODE=ON"
#define AT_CMD_EHCO_OFF        "AT+UARTE=OFF"
#define AT_CMD_TEST            "AT"

/* Delimiter */
#define AT_RECV_PREFIX          "\r\n"
#define AT_RECV_SUCCESS_POSTFIX "OK\r\n"
#define AT_RECV_FAIL_POSTFIX    "ERROR\r\n"
#define AT_SEND_DELIMITER       "\r"

/* AT event */
#define AT_EVENT_ENET_DATA "+ENETEVENT:"

/* uart config */
#define AT_UART_BAUDRATE     115200
#define AT_UART_DATA_WIDTH   DATA_WIDTH_8BIT
#define AT_UART_PARITY       NO_PARITY
#define AT_UART_STOP_BITS    STOP_BITS_1
#define AT_UART_FLOW_CONTROL FLOW_CONTROL_DISABLED
#define AT_UART_MODE         MODE_TX_RX
#define AT_UART_TIMEOUT_MS   1000

#if defined(SAL_HAL_IMPL_SIM800)
#define AT_CMD_DATA_INTERVAL_MS 50
#endif

#ifdef WITH_OS_ENABLED
#define AT_SINGLE_TASK    0
#else
#define AT_SINGLE_TASK    1
#endif

#ifdef __cplusplus
}
#endif
#endif



#ifndef __IOT_EXPORT_H__
#define __IOT_EXPORT_H__
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>

typedef enum _IOT_LogLevel {
    IOT_LOG_EMERG = 0,
    IOT_LOG_CRIT,
    IOT_LOG_ERROR,
    IOT_LOG_WARNING,
    IOT_LOG_INFO,
    IOT_LOG_DEBUG,
} IOT_LogLevel;

void    IOT_OpenLog(const char *ident);
void    IOT_CloseLog(void);
void    IOT_SetLogLevel(IOT_LogLevel level);
void    IOT_DumpMemoryStats(IOT_LogLevel level);
int     IOT_SetupConnInfo(void);

#include "sub_export_errno.h"
#include "sub_export_mqtt.h"
#include "sub_export_device.h"
#include "sub_export_shadow.h"
#include "sub_export_coap.h"

#if defined(__cplusplus)
}
#endif
#endif  /* __IOT_EXPORT_H__ */

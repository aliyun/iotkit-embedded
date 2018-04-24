#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "interface/log_abstract.h"

#define LOGGER_CLASS get_logger_class()

typedef struct {
    const void* _;
    log_level_t _log_level;
    char* _log_name;
    char* _log_buffer;
} logger_t;

extern const void* get_logger_class();

extern void* _g_default_logger;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOGGER_H */

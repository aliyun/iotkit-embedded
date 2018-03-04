#ifndef DM_IMPL_H
#define DM_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "interface/thing_abstract.h"
#include "interface/thing_manager_abstract.h"
#include "interface/log_abstract.h"
#include "interface/list_abstract.h"

#define DM_IMPL_CLASS get_dm_impl_class()

typedef struct {
    const void*  _;
    void*        _logger;
    char*        _name; /* dm thing manager object name. */
    int          _get_tsl_from_cloud;
    void*        _thing_manager; /* thing manager object */
    int          _log_level;
    int          _domain_type;
} dm_impl_t;

extern const void* get_dm_impl_class();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DM_IMPL_H */

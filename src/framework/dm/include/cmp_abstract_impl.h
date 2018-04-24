#ifndef CMP_ABSTRACT_IMPL_H
#define CMP_ABSTRACT_IMPL_H

#include "interface/cmp_abstract.h"

#include "iot_import.h"
#include "iot_export_errno.h"
#include "iot_export_cmp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "dm_import.h"

#define CMP_ABSTRACT_IMPL_CLASS get_cmp_impl_class()

typedef struct {
    const void* _;
    int         cmp_inited;
} cmp_abstract_impl_t;

extern const void* get_cmp_impl_class();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_ABSTRACT_IMPL_H */

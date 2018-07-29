#ifndef _LINKKIT_GATEWAY_LEGACY_H_
#define _LINKKIT_GATEWAY_LEGACY_H_

#include "linkkit_gateway_export.h"
#include "lite-list.h"

typedef struct {
    int devid;
    linkkit_cbs_t *callback;
    void *callback_ctx;
    struct list_head linked_list;
}linkkit_gateway_dev_node_t;

typedef struct {
    int is_started;
    linkkit_params_t init_params;
    void *dispatch_thread;
    handle_service_fota_callback_fp_t fota_callback;
    struct list_head callback_list;
}linkkit_gateway_legacy_ctx_t;


#endif
#ifndef _IOTX_DM_OPT_H
#define _IOTX_DM_OPT_H

typedef enum {
	IOTX_DOPT_DOWNSTREAM_PROPERTY_POST_REPLY,
	IOTX_DOPT_DOWNSTREAM_EVENT_POST_REPLY,
	IOTX_DOPT_UPSTREAM_PROPERTY_SET_REPLY
}iotx_dopt_t;

typedef struct {
	int prop_post_reply_opt;
	int event_post_reply_opt;
	int prop_set_reply_opt;
}iotx_dopt_ctx;

int iotx_dopt_set(iotx_dopt_t opt, void *data);
int iotx_dopt_get(iotx_dopt_t opt, void *data);

#endif
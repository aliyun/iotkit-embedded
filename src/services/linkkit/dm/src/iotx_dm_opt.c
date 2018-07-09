#include "iot_import.h"
#include "iotx_dm_common.h"
#include "iotx_dm_opt.h"

static iotx_dopt_ctx g_iotx_dopt = {
	0,0,1
};

int iotx_dopt_set(iotx_dopt_t opt, void *data)
{
	int res = SUCCESS_RETURN;

	if (data == NULL) {return FAIL_RETURN;}
	
	switch (opt) {
		case IOTX_DOPT_DOWNSTREAM_PROPERTY_POST_REPLY: {
			int opt = *(int *)(data);
			g_iotx_dopt.prop_post_reply_opt = opt;
		}
		break;
		case IOTX_DOPT_DOWNSTREAM_EVENT_POST_REPLY: {
			int opt = *(int *)(data);
			g_iotx_dopt.event_post_reply_opt = opt;
		}
		break;
		case IOTX_DOPT_UPSTREAM_PROPERTY_SET_REPLY: {
			int opt = *(int *)(data);
			g_iotx_dopt.prop_set_reply_opt = opt;
		}
		break;
		default: {
			dm_log_err(IOTX_DM_LOG_DOPT_UNKNOWN_OPT);
			res = FAIL_RETURN;
		}
		break;
	}

	return res;
}

int iotx_dopt_get(iotx_dopt_t opt, void *data)
{
	int res = SUCCESS_RETURN;
	
	if (data == NULL) {return FAIL_RETURN;}

	switch (opt) {
		case IOTX_DOPT_DOWNSTREAM_PROPERTY_POST_REPLY: {
			*(int *)(data) = g_iotx_dopt.prop_post_reply_opt;
		}
		break;
		case IOTX_DOPT_DOWNSTREAM_EVENT_POST_REPLY: {
			*(int *)(data) = g_iotx_dopt.event_post_reply_opt;
		}
		break;
		case IOTX_DOPT_UPSTREAM_PROPERTY_SET_REPLY: {
			*(int *)(data) = g_iotx_dopt.prop_set_reply_opt;
		}
		break;
		default: {
			dm_log_err(IOTX_DM_LOG_DOPT_UNKNOWN_OPT);
			res = FAIL_RETURN;
		}
		break;
	}

	return res;
}
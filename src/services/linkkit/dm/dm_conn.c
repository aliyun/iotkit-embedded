#include "iotx_dm_internal.h"
#include "dm_cm_wrapper.h"
#include "dm_conn.h"

static iotx_dconn_ctx g_iotx_dconn_ctx;

static iotx_dconn_ctx* _iotx_dconn_get_ctx(void)
{
	return &g_iotx_dconn_ctx;
}

int iotx_dconn_init(void)
{
	int res = 0;
	iotx_dconn_ctx *ctx = _iotx_dconn_get_ctx();
	memset(ctx,0,sizeof(iotx_dconn_ctx));
	
	/* Create Cloud Connectivity */
	res = dm_cmw_conn_cloud_mqtt_init(&ctx->cloud_connectivity);
	if (res == ERROR_NO_MEM) {goto ERROR;}

	/* Create Local Connectivity */
#ifdef CONFIG_DM_SUPPORT_LOCAL_CONN
	res = dm_cmw_conn_local_alcs_init(&ctx->local_connectivity);
	if (res == ERROR_NO_MEM) {goto ERROR;}
#endif

	return SUCCESS_RETURN;
ERROR:
#ifdef CONFIG_DM_SUPPORT_LOCAL_CONN
	if (ctx->local_connectivity) {dm_cmw_conn_destroy(&ctx->local_connectivity);}
#endif
	if (ctx->cloud_connectivity) {dm_cmw_conn_destroy(&ctx->cloud_connectivity);}

	return FAIL_RETURN;
}

int iotx_dconn_deinit(void)
{
	iotx_dconn_ctx *ctx = _iotx_dconn_get_ctx();
#ifdef CONFIG_DM_SUPPORT_LOCAL_CONN
	if (ctx->local_connectivity) {dm_cmw_conn_destroy(&ctx->local_connectivity);}
#endif
	if (ctx->cloud_connectivity) {dm_cmw_conn_destroy(&ctx->cloud_connectivity);}

	return SUCCESS_RETURN;
}

void* iotx_dconn_get_cloud_conn(void)
{
	iotx_dconn_ctx *ctx = _iotx_dconn_get_ctx();

	return ctx->cloud_connectivity;
}

void* iotx_dconn_get_local_conn(void)
{
	iotx_dconn_ctx *ctx = _iotx_dconn_get_ctx();

	return ctx->local_connectivity;
}

void iotx_dconn_set_cloud_conn_state(int connected)
{
	iotx_dconn_ctx *ctx = _iotx_dconn_get_ctx();

	ctx->cloud_connected = (connected == 0)?(0):(1);
}

int iotx_dconn_get_cloud_conn_state(void)
{
	iotx_dconn_ctx *ctx = _iotx_dconn_get_ctx();

	return ctx->cloud_connected;
}

void iotx_dconn_set_local_conn_state(int connected)
{
	iotx_dconn_ctx *ctx = _iotx_dconn_get_ctx();

	ctx->local_connected = (connected == 0)?(0):(1);
}

int iotx_dconn_get_local_conn_state(void)
{
	iotx_dconn_ctx *ctx = _iotx_dconn_get_ctx();

	return ctx->local_connected;
}

#ifndef _IOTX_DM_CM_MANAGER_H_
#define _IOTX_DM_CM_MANAGER_H_

typedef struct {
	int cloud_connected;
	void *cloud_connectivity;
	int local_connected;
	void *local_connectivity;
}iotx_dconn_ctx;

int iotx_dconn_init(void);
int iotx_dconn_deinit(void);
void* iotx_dconn_get_cloud_conn(void);
void* iotx_dconn_get_local_conn(void);
void iotx_dconn_set_cloud_conn_state(int connected);
int iotx_dconn_get_cloud_conn_state(void);
void iotx_dconn_set_local_conn_state(int connected);
int iotx_dconn_get_local_conn_state(void);

#endif
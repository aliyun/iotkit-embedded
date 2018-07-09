#include "iot_import.h"
#include "lite-utils.h"
#include "lite-list.h"
#include "iotx_dm_common.h"
#include "iotx_dm_msg_dispatch.h"
#include "iotx_dm_manager.h"
#include "iotx_dm_message.h"
#include "iotx_dm_cm_wrapper.h"
#include "iotx_dm_message_cache.h"
#include "iotx_dm_conn.h"
#include "iotx_dm_subscribe.h"
#include "iotx_dm_opt.h"
#include "iot_export_dm.h"

/*****************************Internal Definition*****************************/

const char IOTX_DCS_SYS_PREFIX[]                        DM_READ_ONLY = "/sys/%s/%s/";
const char IOTX_DCS_EXT_SESSION_PREFIX[]                DM_READ_ONLY = "/ext/session/%s/%s/";
const char IOTX_DCS_REPLY_SUFFIX[]                      DM_READ_ONLY = "_reply";

/* From Cloud To Local Request And Response*/
const char IOTX_DCS_THING_TOPO_ADD_NOTIFY[]             DM_READ_ONLY = "thing/topo/add/notify";
const char IOTX_DCS_THING_TOPO_ADD_NOTIFY_REPLY[]       DM_READ_ONLY = "thing/topo/add/notify_reply";
const char IOTX_DCS_THING_SERVICE_PROPERTY_SET[]        DM_READ_ONLY = "thing/service/property/set";
const char IOTX_DCS_THING_SERVICE_PROPERTY_SET_REPLY[]  DM_READ_ONLY = "thing/service/property/set_reply";
const char IOTX_DCS_THING_SERVICE_PROPERTY_GET[]        DM_READ_ONLY = "thing/service/property/get";
const char IOTX_DCS_THING_SERVICE_REQUEST[]             DM_READ_ONLY = "thing/service/%s";
const char IOTX_DCS_THING_SERVICE_RESPONSE[]            DM_READ_ONLY = "thing/service/%.*s_reply";
const char IOTX_DCS_THING_DISABLE[]                     DM_READ_ONLY = "thing/disable";
const char IOTX_DCS_THING_DISABLE_REPLY[]               DM_READ_ONLY = "thing/disable_reply";
const char IOTX_DCS_THING_ENABLE[]                      DM_READ_ONLY = "thing/enable";
const char IOTX_DCS_THING_ENABLE_REPLY[]                DM_READ_ONLY = "thing/enable_reply";
const char IOTX_DCS_THING_DELETE[]                      DM_READ_ONLY = "thing/delete";
const char IOTX_DCS_THING_DELETE_REPLY[]                DM_READ_ONLY = "thing/delete_reply";
const char IOTX_DCS_THING_MODEL_DOWN_RAW[]              DM_READ_ONLY = "thing/model/down_raw";
const char IOTX_DCS_THING_MODEL_DOWN_RAW_REPLY[]        DM_READ_ONLY = "thing/model/down_raw_reply";
const char IOTX_DCS_THING_GATEWAY_PERMIT[]              DM_READ_ONLY = "thing/gateway/permit";
const char IOTX_DCS_THING_GATEWAY_PERMIT_REPLY[]        DM_READ_ONLY = "thing/gateway/permit_reply";

/* From Local To Cloud Request And Response*/
const char IOTX_DCS_THING_SUB_REGISTER[]                DM_READ_ONLY = "thing/sub/register";
const char IOTX_DCS_THING_SUB_REGISTER_REPLY[]          DM_READ_ONLY = "thing/sub/register_reply";
const char IOTX_DCS_THING_SUB_UNREGISTER[]              DM_READ_ONLY = "thing/sub/unregister";
const char IOTX_DCS_THING_SUB_UNREGISTER_REPLY[]        DM_READ_ONLY = "thing/sub/unregister_reply";
const char IOTX_DCS_THING_TOPO_ADD[]                    DM_READ_ONLY = "thing/topo/add";
const char IOTX_DCS_THING_TOPO_ADD_REPLY[]              DM_READ_ONLY = "thing/topo/add_reply";
const char IOTX_DCS_THING_TOPO_DELETE[]                 DM_READ_ONLY = "thing/topo/delete";
const char IOTX_DCS_THING_TOPO_DELETE_REPLY[]           DM_READ_ONLY = "thing/topo/delete_reply";
const char IOTX_DCS_THING_TOPO_GET[]                    DM_READ_ONLY = "thing/topo/get";
const char IOTX_DCS_THING_TOPO_GET_REPLY[]              DM_READ_ONLY = "thing/topo/get_reply";
const char IOTX_DCS_THING_LIST_FOUND[]                  DM_READ_ONLY = "thing/list/found";
const char IOTX_DCS_THING_LIST_FOUND_REPLY[]            DM_READ_ONLY = "thing/list/found_reply";
const char IOTX_DCS_THING_EVENT_PROPERTY_POST[]         DM_READ_ONLY = "thing/event/property/post";
const char IOTX_DCS_THING_EVENT_PROPERTY_POST_REPLY[]   DM_READ_ONLY = "thing/event/property/post_reply";
const char IOTX_DCS_THING_EVENT_POST[]                  DM_READ_ONLY = "thing/event/%.*s/post";
const char IOTX_DCS_THING_EVENT_POST_REPLY[]            DM_READ_ONLY = "thing/event/%s/post_reply";
const char IOTX_DCS_THING_DEVICEINFO_UPDATE[]           DM_READ_ONLY = "thing/deviceinfo/update";
const char IOTX_DCS_THING_DEVICEINFO_UPDATE_REPLY[]     DM_READ_ONLY = "thing/deviceinfo/update_reply";
const char IOTX_DCS_THING_DEVICEINFO_DELETE[]           DM_READ_ONLY = "thing/deviceinfo/delete";
const char IOTX_DCS_THING_DEVICEINFO_DELETE_REPLY[]     DM_READ_ONLY = "thing/deviceinfo/delete_reply";
const char IOTX_DCS_THING_DSLTEMPLATE_GET[]             DM_READ_ONLY = "thing/dsltemplate/get";
const char IOTX_DCS_THING_DSLTEMPLATE_GET_REPLY[]       DM_READ_ONLY = "thing/dsltemplate/get_reply";
const char IOTX_DCS_THING_DYNAMICTSL_GET[]              DM_READ_ONLY = "thing/dynamicTsl/get";
const char IOTX_DCS_THING_DYNAMICTSL_GET_REPLY[]        DM_READ_ONLY = "thing/dynamicTsl/get_reply";
const char IOTX_DCS_COMBINE_LOGIN[]                     DM_READ_ONLY = "combine/login";
const char IOTX_DCS_COMBINE_LOGIN_REPLY[]               DM_READ_ONLY = "combine/login_reply";
const char IOTX_DCS_COMBINE_LOGOUT[]                    DM_READ_ONLY = "combine/logout";
const char IOTX_DCS_COMBINE_LOGOUT_REPLY[]              DM_READ_ONLY = "combine/logout_reply";
const char IOTX_DCS_THING_MODEL_UP_RAW[]                DM_READ_ONLY = "thing/model/up_raw";
const char IOTX_DCS_THING_MODEL_UP_RAW_REPLY[]          DM_READ_ONLY = "thing/model/up_raw_reply";
const char IOTX_DCS_DEV_CORE_SERVICE_DEV[]              DM_READ_ONLY = "/dev/core/service/dev";
const char IOTX_DCS_THING_LAN_PREFIX_GET[]              DM_READ_ONLY = "thing/lan/prefix/get";
const char IOTX_DCS_THING_LAN_PREFIX_GET_REPLY[]        DM_READ_ONLY = "thing/lan/prefix/get_reply";
const char IOTX_DCS_THING_LAN_PREFIX_UPDATE[]           DM_READ_ONLY = "thing/lan/prefix/update";
const char IOTX_DCS_THING_LAN_PREFIX_UPDATE_REPLY[]     DM_READ_ONLY = "thing/lan/prefix/update_reply";
const char IOTX_DCS_THING_LAN_BLACKLIST_UPDATE[]        DM_READ_ONLY = "thing/lan/blacklist/update";
const char IOTX_DCS_THING_LAN_BLACKLIST_UPDATE_REPLY[]  DM_READ_ONLY = "thing/lan/blacklist/update_reply";

static const iotx_dcs_topic_mapping_t g_iotx_dcs_topic_mapping[] DM_READ_ONLY = {
	{IOTX_DCS_THING_TOPO_ADD_NOTIFY,            IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_topo_add_notify              },
	{IOTX_DCS_THING_SERVICE_PROPERTY_SET,       IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_service_property_set         },
	{IOTX_DCS_THING_SERVICE_PROPERTY_GET,       IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_service_property_get         },
	{IOTX_DCS_THING_DISABLE,                    IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_disable                      },
	{IOTX_DCS_THING_ENABLE,                     IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_enable                       },
	{IOTX_DCS_THING_DELETE,                     IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_delete                       },
	{IOTX_DCS_THING_MODEL_DOWN_RAW,             IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_model_down_raw               },
	{IOTX_DCS_THING_GATEWAY_PERMIT,             IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_gateway_permit               },
	{IOTX_DCS_THING_SUB_REGISTER_REPLY,         IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_sub_register_reply           },
	{IOTX_DCS_THING_SUB_UNREGISTER_REPLY,       IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_sub_unregister_reply         },
	{IOTX_DCS_THING_TOPO_ADD_REPLY,             IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_topo_add_reply               },
	{IOTX_DCS_THING_TOPO_DELETE_REPLY,          IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_topo_delete_reply            },
	{IOTX_DCS_THING_TOPO_GET_REPLY,             IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_topo_get_reply               },
	{IOTX_DCS_THING_LIST_FOUND_REPLY,           IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_GATEWAY, IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_list_found_reply             },
	{IOTX_DCS_THING_EVENT_PROPERTY_POST_REPLY,  IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_event_property_post_reply    },
	{IOTX_DCS_THING_DEVICEINFO_UPDATE_REPLY,    IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_deviceinfo_update_reply      },
	{IOTX_DCS_THING_DEVICEINFO_DELETE_REPLY,    IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_deviceinfo_delete_reply      },
	{IOTX_DCS_THING_DSLTEMPLATE_GET_REPLY,      IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_dsltemplate_get_reply        },
	{IOTX_DCS_THING_DYNAMICTSL_GET_REPLY,       IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_dynamictsl_get_reply         },
	{IOTX_DCS_COMBINE_LOGIN_REPLY,              IOTX_DCS_EXT_SESSION_PREFIX, IOTX_DM_DEVICE_GATEWAY, IOTX_DM_SERVICE_ALL,   iotx_dcs_combine_login_reply                },
	{IOTX_DCS_COMBINE_LOGOUT_REPLY,             IOTX_DCS_EXT_SESSION_PREFIX, IOTX_DM_DEVICE_GATEWAY, IOTX_DM_SERVICE_ALL,   iotx_dcs_combine_logout_reply               },
	{IOTX_DCS_THING_MODEL_UP_RAW_REPLY,         IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_ALL,     IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_model_up_raw_reply           },
	{IOTX_DCS_DEV_CORE_SERVICE_DEV,             NULL,                        IOTX_DM_DEVICE_MAIN,    IOTX_DM_LOCAL_NO_AUTH, iotx_dcs_thing_dev_core_service_dev         },
	//{IOTX_DCS_THING_LAN_PREFIX_GET_REPLY,       IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_MAIN,    IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_lan_prefix_get_reply         },
	//{IOTX_DCS_THING_LAN_PREFIX_UPDATE_REPLY,    IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_MAIN,    IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_lan_prefix_update_reply      },
	//{IOTX_DCS_THING_LAN_BLACKLIST_UPDATE_REPLY, IOTX_DCS_SYS_PREFIX,         IOTX_DM_DEVICE_MAIN,    IOTX_DM_SERVICE_ALL,   iotx_dcs_thing_lan_blacklist_update_reply   }
};

static const iotx_dcs_event_mapping_t g_iotx_dcs_event_mapping[] DM_READ_ONLY = {
	{IOTX_CM_EVENT_CLOUD_CONNECTED,         iotx_dcs_event_cloud_connected_handler      },
	{IOTX_CM_EVENT_CLOUD_DISCONNECT,        iotx_dcs_event_cloud_disconnect_handler     },
	{IOTX_CM_EVENT_CLOUD_RECONNECT,         iotx_dcs_event_cloud_reconnect_handler      },
	{IOTX_CM_EVENT_LOCAL_CONNECTED,         iotx_dcs_event_local_connected_handler      },
	{IOTX_CM_EVENT_LOCAL_DISCONNECT,        iotx_dcs_event_local_disconnect_handler     },
	{IOTX_CM_EVENT_LOCAL_RECONNECT,         iotx_dcs_event_local_reconnect_handler      },
	{IOTX_CM_EVENT_FOUND_DEVICE,            iotx_dcs_event_found_device_handler         },
	{IOTX_CM_EVENT_REMOVE_DEVICE,           iotx_dcs_event_remove_device_handler        },
	{IOTX_CM_EVENT_REGISTER_RESULT,         iotx_dcs_event_register_result_handler      },
	{IOTX_CM_EVENT_UNREGISTER_RESULT,       iotx_dcs_event_unregister_result_handler    },
	{IOTX_CM_EVENT_SEND_RESULT,             iotx_dcs_event_send_result_handler          },
	{IOTX_CM_EVENT_ADD_SERVICE_RESULT,      iotx_dcs_event_add_service_result_handler   },
	{IOTX_CM_EVENT_REMOVE_SERVICE_RESULT,   iotx_dcs_event_remove_service_result_handler},
	{IOTX_CM_EVENT_NEW_DATA_RECEIVED,       iotx_dcs_event_new_data_received_handler    }
};

/*****************************************************************************/

int iotx_dcs_uri_prefix_sys_split(_IN_ char *uri, _IN_ int uri_len, _OU_ int *start, _OU_ int *end)
{
	int res = 0, offset = 0;

	if (uri == NULL || uri_len <= 0) {
		dm_log_err(IOTX_DM_LOG_INVALID_PARAMETER);
		return FAIL_RETURN;
	}

	/*"/sys/%s/%s/"*/
	res = iotx_dcm_memtok(uri,uri_len,IOTX_DCS_SERVICE_DELIMITER,2,&offset);
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

	if (memcmp(IOTX_DCS_SYS_PREFIX,uri,offset+1) != 0) {return FAIL_RETURN;}

	if (start) {*start = offset;}
	if (end) {*end = uri_len - offset - 1;}

	return SUCCESS_RETURN;
}

int iotx_dcs_uri_prefix_ext_session_split(_IN_ char *uri, _IN_ int uri_len, _OU_ int *start, _OU_ int *end)
{
	int res = 0, offset = 0;

	if (uri == NULL || uri_len <= 0) {
		dm_log_err(IOTX_DM_LOG_INVALID_PARAMETER);
		return FAIL_RETURN;
	}

	/*"/ext/session/%s/%s/"*/
	res = iotx_dcm_memtok(uri,uri_len,IOTX_DCS_SERVICE_DELIMITER,3,&offset);
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

	if (memcmp(IOTX_DCS_EXT_SESSION_PREFIX,uri,offset+1) != 0) {return FAIL_RETURN;}
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

	if (start) {*start = offset;}
	if (end) {*end = uri_len - offset - 1;}

	return SUCCESS_RETURN;
}

int iotx_dcs_uri_pkdn_split(_IN_ char *uri, _IN_ int uri_len, _OU_ int *start, _OU_ int *end)
{
	int res = 0, offset = 0;

	if (uri == NULL || uri_len <= 0 || start == NULL || end == NULL) {
		dm_log_err(IOTX_DM_LOG_INVALID_PARAMETER);
		return FAIL_RETURN;
	}

	/* "/pk/dn/" */
	res = iotx_dcm_memtok(uri,uri_len,IOTX_DCS_SERVICE_DELIMITER,3,&offset);
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

	*start = offset;
	*end = uri_len - offset - 1;

	return SUCCESS_RETURN;
}

int iotx_dcs_uri_service_specific_split(_IN_ char *uri, _IN_ int uri_len, _OU_ int *start, _OU_ int *end)
{
	int res = 0, offset = 0;
	const char *prefix = "/thing/service/";

	if (uri == NULL || uri_len <= 0 || start == NULL || end == NULL) {
		dm_log_err(IOTX_DM_LOG_INVALID_PARAMETER);
		return FAIL_RETURN;
	}

	/* "/thing/service/%s" */
	res = iotx_dcm_memtok(uri,uri_len,IOTX_DCS_SERVICE_DELIMITER,3,&offset);
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

	/* Check "thing/service" */
	if (memcmp(IOTX_DCS_THING_SERVICE_REQUEST,uri + 1,offset) != 0) {return FAIL_RETURN;}

	if (uri_len <= strlen(prefix)) {return FAIL_RETURN;}

	*start = offset;
	*end = uri_len - 1;

	return SUCCESS_RETURN;
}

int iotx_dcs_uri_event_specific_split(_IN_ char *uri, _IN_ int uri_len, _OU_ int *start, _OU_ int *end)
{
	int res = 0, offset = 0;
	const char *suffix = "post_reply";

	if (uri == NULL || uri_len <= 0 || start == NULL || end == NULL) {
		dm_log_err(IOTX_DM_LOG_INVALID_PARAMETER);
		return FAIL_RETURN;
	}

	/* "/thing/event/%s/post_reply" */
	res = iotx_dcm_memtok(uri,uri_len,IOTX_DCS_SERVICE_DELIMITER,3,&offset);
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

	/* Check "thing/event/" */
	if (memcmp(IOTX_DCS_THING_EVENT_POST_REPLY,uri + 1,offset) != 0) {return FAIL_RETURN;}

	/* Check "post_reply" */
	if (uri_len <= strlen(suffix)) {return FAIL_RETURN;}
	if (memcmp(uri + (uri_len - strlen(suffix)),suffix,strlen(suffix)) != 0) {return FAIL_RETURN;}

	*start = offset;
	*end = uri_len - strlen(suffix) - 1;

	return SUCCESS_RETURN;
}

#if 0
int iotx_dcs_topic_generic_unsubscribe(int devid)
{
	int res = 0, index = 0;
	char product_key[PRODUCT_KEY_MAXLEN] = {0};
	char device_name[DEVICE_NAME_MAXLEN] = {0};
	char *service_name = NULL;
	iotx_dcs_ctx_t *ctx = _iotx_dcs_get_ctx();
	iotx_dcs_topic_mapping_t *dcs_mapping = (iotx_dcs_topic_mapping_t *)g_iotx_dcs_topic_mapping;

	if (devid < 0 || index < 0 || index >= sizeof(g_iotx_dcs_topic_mapping)/sizeof(iotx_dcs_topic_mapping_t)) {
		dm_log_err(IOTX_DM_LOG_INVALID_PARAMETER);
		return FAIL_RETURN;
	}

	res = iotx_dmgr_get_product_key(devid,product_key);
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

	res = iotx_dmgr_get_device_name(devid,device_name);
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

	for (index = 0;index < sizeof(g_iotx_dcs_topic_mapping)/sizeof(iotx_dcs_topic_mapping_t);index++) {
		service_name = NULL;

		res = iotx_dcm_service_name((char *)dcs_mapping[index].service_prefix,(char *)dcs_mapping[index].service_name,product_key,device_name,&service_name);
		if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

		/* Unsubscribe Cloud Service */
		dm_log_debug("Current Unsubscribe Topic: %s",service_name);
		res = iotx_dcw_cloud_unregister(ctx->cloud_connectivity,service_name);
		if (res == FAIL_RETURN) {dm_log_warning(IOTX_DM_LOG_DMGR_SERVICE_CLOUD_REGISTER_FAILED,strlen(service_name),service_name);}
		DM_free(service_name);
	}

	return SUCCESS_RETURN;
}


int iotx_dcs_topic_service_event_unsubscribe(int devid)
{
	int res = 0, index = 0, service_number = 0, event_number = 0;
	char product_key[PRODUCT_KEY_MAXLEN] = {0};
	char device_name[DEVICE_NAME_MAXLEN] = {0};
	iotx_dcs_ctx_t *ctx = _iotx_dcs_get_ctx();
	void *reference = NULL;
	char *service_name = NULL, *method = NULL;

	if (devid < 0) {return FAIL_RETURN;}

	res = iotx_dmgr_get_product_key(devid,product_key);
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}
	dm_log_debug("Current Shadow Product Key: %s",product_key);

	res = iotx_dmgr_get_device_name(devid,device_name);
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}
	dm_log_debug("Current Shadow Device Name: %s",device_name);

	res = iotx_dmgr_get_service_number(devid,&service_number);
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

	res = iotx_dmgr_get_event_number(devid,&event_number);
	if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

	dm_log_debug("Current Shadow Service Number: %d",service_number);
	for (index = 0;index < service_number;index++) {
		service_name = NULL;reference = NULL;method = NULL;

		res = iotx_dmgr_get_service_by_index(devid,index,&reference);
		if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

		res = iotx_dmgr_get_service_method(reference,&method);
		if (res != SUCCESS_RETURN) {return FAIL_RETURN;}
		dm_log_debug("Service Index: %d, Method: %s",index,method);

		res = iotx_dcm_replace_char(method,strlen(method),'.','/');
		if (res != SUCCESS_RETURN) {DM_free(method);return FAIL_RETURN;}

		res = iotx_dcm_service_name((char *)IOTX_DCS_SYS_PREFIX,method,product_key,device_name,&service_name);
		if (res != SUCCESS_RETURN) {DM_free(method);return FAIL_RETURN;}

		dm_log_debug("Current Unsubscribe Topic: %s",service_name);

		res = iotx_dcw_cloud_unregister(ctx->cloud_connectivity,service_name);
		if (res == FAIL_RETURN) {dm_log_warning(IOTX_DM_LOG_DMGR_SERVICE_CLOUD_REGISTER_FAILED,strlen(service_name),service_name);}
		DM_free(service_name);DM_free(method);
	}

	dm_log_debug("Current Shadow TSL Event Number: %d",event_number);
	for (index = 0;index < event_number;index++) {
		service_name = NULL;reference = NULL;method = NULL;

		res = iotx_dmgr_get_event_by_index(devid,index,&reference);
		if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

		res = iotx_dmgr_get_event_method(reference,&method);
		if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

		dm_log_debug("TSL Event Index: %d, Method: %s",index,method);

		res = iotx_dcm_replace_char(method,strlen(method),'.','/');
		if (res != SUCCESS_RETURN) {DM_free(method);return FAIL_RETURN;}

		res = iotx_dcm_service_name((char *)IOTX_DCS_SYS_PREFIX,method,product_key,device_name,&service_name);
		if (res != SUCCESS_RETURN) {DM_free(method);return FAIL_RETURN;}

		dm_log_debug("Current Unsubscribe Topic: %s",service_name);

		res = iotx_dcw_cloud_unregister(ctx->cloud_connectivity,service_name);
		if (res == FAIL_RETURN) {dm_log_warning(IOTX_DM_LOG_DMGR_SERVICE_CLOUD_REGISTER_FAILED,strlen(service_name),service_name);}
		DM_free(service_name);DM_free(method);
	}

	return SUCCESS_RETURN;
}
#endif

iotx_dcs_topic_mapping_t* iotx_dcs_get_topic_mapping(void)
{
	return (iotx_dcs_topic_mapping_t *)g_iotx_dcs_topic_mapping;
}

int iotx_dcs_get_topic_mapping_size(void)
{
	return sizeof(g_iotx_dcs_topic_mapping)/sizeof(iotx_dcs_topic_mapping_t);
}

int iotx_dcs_get_topic_mapping_dev_type_mask(_IN_ int index, _OU_ int *dev_type_mask)
{
	iotx_dcs_topic_mapping_t *dcs_mapping = (iotx_dcs_topic_mapping_t *)g_iotx_dcs_topic_mapping;

	if (index < 0 || dev_type_mask == NULL ||
		index >= sizeof(g_iotx_dcs_topic_mapping)/sizeof(iotx_dcs_topic_mapping_t)) {
		dm_log_err(IOTX_DM_LOG_INVALID_PARAMETER);
		return FAIL_RETURN;
	}

	*dev_type_mask = dcs_mapping[index].dev_type;

	return SUCCESS_RETURN;
}

iotx_dcs_event_mapping_t* iotx_dcs_get_event_mapping(void)
{
	return (iotx_dcs_event_mapping_t *)g_iotx_dcs_event_mapping;
}

int iotx_dcs_get_event_mapping_size(void)
{
	return sizeof(g_iotx_dcs_event_mapping)/sizeof(iotx_dcs_event_mapping_t);
}

void iotx_dcs_thing_topo_add_notify(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_request_payload_t request;
	iotx_dmsg_response_t response;

	dm_log_debug(IOTX_DCS_THING_TOPO_ADD_NOTIFY);

	/* Request */
	res = iotx_dmsg_request_parse(msg->payload,msg->payload_length,&request);
	if (res == SUCCESS_RETURN) {
		res = iotx_dmsg_topo_add_notify(request.params.value,request.params.value_length);
	}

	/* Response */
	response.service_prefix = IOTX_DCS_SYS_PREFIX;
	response.service_name = IOTX_DCS_THING_TOPO_ADD_NOTIFY_REPLY;
	memcpy(response.product_key,source->product_key,strlen(source->product_key));
	memcpy(response.device_name,source->device_name,strlen(source->device_name));
	response.code = (res == SUCCESS_RETURN)?(IOTX_DM_ERR_CODE_SUCCESS):(IOTX_DM_ERR_CODE_REQUEST_ERROR);
	iotx_dmsg_response_without_data(&request,&response);
}

void iotx_dcs_thing_service_property_set(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0, devid = 0;
	char product_key[PRODUCT_KEY_MAXLEN] = {0};
	char device_name[DEVICE_NAME_MAXLEN] = {0};
	iotx_dmsg_request_payload_t request;
	iotx_dmsg_response_t response;

	memset(&request,0,sizeof(iotx_dmsg_request_payload_t));
	memset(&response,0,sizeof(iotx_dmsg_response_t));

	/* Request */
	res = iotx_dmsg_uri_parse_pkdn(msg->URI,msg->URI_length,2,4,product_key,device_name);
	if (res != SUCCESS_RETURN) {return;}

	res = iotx_dmgr_search_device_by_pkdn(product_key,device_name,&devid);
	if (res != SUCCESS_RETURN) {return;}

	res = iotx_dmsg_request_parse(msg->payload,msg->payload_length,&request);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	res = iotx_dmsg_property_set(devid,&request);

	/* Response */
	response.service_prefix = IOTX_DCS_SYS_PREFIX;
	response.service_name = IOTX_DCS_THING_SERVICE_PROPERTY_SET_REPLY;
	memcpy(response.product_key,product_key,strlen(product_key));
	memcpy(response.device_name,device_name,strlen(device_name));
	response.code = (res == SUCCESS_RETURN)?(IOTX_DM_ERR_CODE_SUCCESS):(IOTX_DM_ERR_CODE_REQUEST_ERROR);

	int prop_set_reply_opt = 0;
	res = iotx_dopt_get(IOTX_DOPT_UPSTREAM_PROPERTY_SET_REPLY,&prop_set_reply_opt);
	if (res == SUCCESS_RETURN) {
		if (prop_set_reply_opt) {iotx_dmsg_response_without_data(&request,&response);}
	}
}

void iotx_dcs_thing_service_property_get(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	/* Never Used */
	dm_log_debug("Serivce Property Get, Payload: %.*s",msg->payload_length,msg->payload);
}

void iotx_dcs_thing_service_request(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, char *identifier, int identifier_len, void* user_data)
{
	int res = 0;
	char product_key[PRODUCT_KEY_MAXLEN] = {0};
	char device_name[DEVICE_NAME_MAXLEN] = {0};
	iotx_dmsg_request_payload_t request;

	dm_log_debug(IOTX_DCS_THING_SERVICE_REQUEST,"{Identifier}");
	dm_log_debug("Current URI: %.*s",msg->URI_length,msg->URI);
	dm_log_debug("Current Identifier: %.*s",identifier_len,identifier);

	/* Parse Product Key And Device Name */
	res = iotx_dmsg_uri_parse_pkdn(msg->URI,msg->URI_length,2,4,product_key,device_name);
	if (res != SUCCESS_RETURN) {return;}

	/* Request */
	res = iotx_dmsg_request_parse(msg->payload,msg->payload_length,&request);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	res = iotx_dmsg_thing_service_request(product_key,device_name,identifier,identifier_len,&request);
	if (res != SUCCESS_RETURN) {return;}
}

void iotx_dcs_thing_disable(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_request_payload_t request;
	iotx_dmsg_response_t response;

	dm_log_debug(IOTX_DCS_THING_DISABLE);

	memset(&request,0,sizeof(iotx_dmsg_request_payload_t));
	memset(&response,0,sizeof(iotx_dmsg_response_t));

	/* Request */
	res = iotx_dmsg_request_parse(msg->payload,msg->payload_length,&request);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	res = iotx_dmsg_thing_disable(source->product_key,source->device_name);

	/* Response */
	response.service_prefix = IOTX_DCS_SYS_PREFIX;
	response.service_name = IOTX_DCS_THING_DISABLE_REPLY;
	memcpy(response.product_key,source->product_key,strlen(source->product_key));
	memcpy(response.device_name,source->device_name,strlen(source->device_name));
	response.code = (res == SUCCESS_RETURN)?(IOTX_DM_ERR_CODE_SUCCESS):(IOTX_DM_ERR_CODE_REQUEST_ERROR);
	iotx_dmsg_response_without_data(&request,&response);
}

void iotx_dcs_thing_enable(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_request_payload_t request;
	iotx_dmsg_response_t response;

	dm_log_debug(IOTX_DCS_THING_ENABLE);

	memset(&request,0,sizeof(iotx_dmsg_request_payload_t));
	memset(&response,0,sizeof(iotx_dmsg_response_t));

	/* Request */
	res = iotx_dmsg_request_parse(msg->payload,msg->payload_length,&request);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	res = iotx_dmsg_thing_enable(source->product_key,source->device_name);

	/* Response */
	response.service_prefix = IOTX_DCS_SYS_PREFIX;
	response.service_name = IOTX_DCS_THING_ENABLE_REPLY;
	memcpy(response.product_key,source->product_key,strlen(source->product_key));
	memcpy(response.device_name,source->device_name,strlen(source->device_name));
	response.code = (res == SUCCESS_RETURN)?(IOTX_DM_ERR_CODE_SUCCESS):(IOTX_DM_ERR_CODE_REQUEST_ERROR);
	iotx_dmsg_response_without_data(&request,&response);
}

void iotx_dcs_thing_delete(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_request_payload_t request;
	iotx_dmsg_response_t response;

	dm_log_debug(IOTX_DCS_THING_DELETE);

	memset(&request,0,sizeof(iotx_dmsg_request_payload_t));
	memset(&response,0,sizeof(iotx_dmsg_response_t));

	/* Request */
	res = iotx_dmsg_request_parse(msg->payload,msg->payload_length,&request);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	res = iotx_dmsg_thing_delete(source->product_key,source->device_name);

	/* Response */
	response.service_prefix = IOTX_DCS_SYS_PREFIX;
	response.service_name = IOTX_DCS_THING_DELETE_REPLY;
	memcpy(response.product_key,source->product_key,strlen(source->product_key));
	memcpy(response.device_name,source->device_name,strlen(source->device_name));
	response.code = (res == SUCCESS_RETURN)?(IOTX_DM_ERR_CODE_SUCCESS):(IOTX_DM_ERR_CODE_REQUEST_ERROR);
	iotx_dmsg_response_without_data(&request,&response);

}

void iotx_dcs_thing_model_down_raw(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	char product_key[PRODUCT_KEY_MAXLEN] = {0};
	char device_name[DEVICE_NAME_MAXLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_MODEL_DOWN_RAW);

	res = iotx_dmsg_uri_parse_pkdn(msg->URI,msg->URI_length,2,4,product_key,device_name);
	if (res != SUCCESS_RETURN) {return;}

	dm_log_debug("Product Key: %s, Device Name: %s",product_key,device_name);

	iotx_dmsg_thing_model_down_raw(product_key,device_name,msg->payload,msg->payload_length);
}

void iotx_dcs_thing_gateway_permit(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_request_payload_t request;
	iotx_dmsg_response_t response;

	dm_log_debug(IOTX_DCS_THING_GATEWAY_PERMIT);

	memset(&request,0,sizeof(iotx_dmsg_request_payload_t));
	memset(&response,0,sizeof(iotx_dmsg_response_t));

	/* Request */
	res = iotx_dmsg_request_parse(msg->payload,msg->payload_length,&request);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	res = iotx_dmsg_thing_gateway_permit(request.params.value,request.params.value_length);

	/* Response */
	response.service_prefix = IOTX_DCS_SYS_PREFIX;
	response.service_name = IOTX_DCS_THING_GATEWAY_PERMIT_REPLY;
	memcpy(response.product_key,source->product_key,strlen(source->product_key));
	memcpy(response.device_name,source->device_name,strlen(source->device_name));
	response.code = (res == SUCCESS_RETURN)?(IOTX_DM_ERR_CODE_SUCCESS):(IOTX_DM_ERR_CODE_REQUEST_ERROR);
	iotx_dmsg_response_without_data(&request,&response);
}

void iotx_dcs_thing_sub_register_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_SUB_REGISTER_REPLY);

	memset(&response,0,sizeof(iotx_dmsg_response_payload_t));

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_thing_sub_register_reply(&response);

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_thing_sub_unregister_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_SUB_UNREGISTER_REPLY);

	memset(&response,0,sizeof(iotx_dmsg_response_payload_t));

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_thing_sub_unregister_reply(&response);

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_thing_topo_add_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_TOPO_ADD_REPLY);

	memset(&response,0,sizeof(iotx_dmsg_response_payload_t));

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_thing_topo_add_reply(&response);

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_thing_topo_delete_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_TOPO_DELETE_REPLY);


	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_thing_topo_delete_reply(&response);

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_thing_topo_get_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_TOPO_GET_REPLY);

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_topo_get_reply(&response);

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_thing_list_found_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;

	dm_log_debug(IOTX_DCS_THING_LIST_FOUND_REPLY);

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_thing_list_found_reply(&response);
}

void iotx_dcs_thing_event_property_post_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_EVENT_PROPERTY_POST_REPLY);

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_thing_event_property_post_reply(&response);

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_thing_event_post_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, char *identifier, int identifier_len, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_EVENT_POST_REPLY,"{Identifier}");
	dm_log_debug("Current URI: %.*s",msg->URI_length,msg->URI);
	dm_log_debug("Current Identifier: %.*s",identifier_len,identifier);

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_thing_event_post_reply(identifier,identifier_len,&response);

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_thing_deviceinfo_update_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_DEVICEINFO_UPDATE_REPLY);

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_thing_deviceinfo_update_reply(&response);

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_thing_deviceinfo_delete_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_DEVICEINFO_DELETE_REPLY);

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_thing_deviceinfo_delete_reply(&response);

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_thing_dsltemplate_get_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_DSLTEMPLATE_GET_REPLY);

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_thing_dsltemplate_get_reply(&response);

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_thing_dynamictsl_get_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_DYNAMICTSL_GET_REPLY);

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	iotx_dmsg_thing_dynamictsl_get_reply(&response);

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_combine_login_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_COMBINE_LOGIN_REPLY);


	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	res = iotx_dmsg_combine_login_reply(&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_combine_logout_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char int_id[IOTX_DCM_UINT32_STRLEN] = {0};

	dm_log_debug(IOTX_DCS_COMBINE_LOGOUT_REPLY);


	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	res = iotx_dmsg_combine_logout_reply(&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Remove Message From Cache */
	memcpy(int_id,response.id.value,response.id.value_length);
	iotx_dmc_msg_remove(atoi(int_id));
}

void iotx_dcs_thing_model_up_raw_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;
	char product_key[PRODUCT_KEY_MAXLEN] = {0};
	char device_name[DEVICE_NAME_MAXLEN] = {0};

	dm_log_debug(IOTX_DCS_THING_MODEL_UP_RAW_REPLY);

	res = iotx_dmsg_uri_parse_pkdn(msg->URI,msg->URI_length,2,4,product_key,device_name);
	if (res != SUCCESS_RETURN) {return;}

	dm_log_debug("Product Key: %s, Device Name: %s",product_key,device_name);

	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	res = iotx_dmsg_thing_model_up_raw_reply(product_key,device_name,&response);
	if (res != SUCCESS_RETURN) {return;}
}

void iotx_dcs_thing_dev_core_service_dev(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{

}

void iotx_dcs_thing_lan_prefix_get_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	int res = 0;
	iotx_dmsg_response_payload_t response;

	dm_log_debug(IOTX_DCS_THING_LAN_PREFIX_GET_REPLY);


	/* Response */
	res = iotx_dmsg_response_parse(msg->payload,msg->payload_length,&response);
	if (res != SUCCESS_RETURN) {return;}

	/* Operation */
	//res = iotx_dmsg_thing_lan_prefix_get_reply(&response);
	//if (res != SUCCESS_RETURN) {return;}
}

void iotx_dcs_thing_lan_prefix_update_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{

}

void iotx_dcs_thing_lan_blacklist_update_reply(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{

}

void iotx_dcs_user_defined_handler(iotx_cm_send_peer_t* source, iotx_cm_message_info_t* msg, void* user_data)
{
	dm_log_info("USER Defined Handler");
	dm_log_info("URI: %.*s",msg->URI_length,msg->URI);
	dm_log_info("Payload: %.*s",msg->payload_length,msg->payload);
}

void iotx_dcs_event_cloud_connected_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_CLOUD_CONNECTED");

	iotx_dconn_set_cloud_conn_state(1);
	
	/* Re-Subscribe Topic */
	/* Start From Subscribe Generic Topic */
	iotx_dsub_multi_next(IOTX_DMGR_LOCAL_NODE_DEVID,0);

	/* Set Service Event Topic Index To IOTX_DMGR_DEV_SUB_START */
	/* Service Event Topic Subscribe Will Be Execute After All Generic Topic Subscribed */
	iotx_dmgr_set_dev_sub_service_event_index(IOTX_DMGR_LOCAL_NODE_DEVID,IOTX_DMGR_DEV_SUB_START);

#ifdef CONFIG_DM_SUPPORT_LOCAL_CONN
	iotx_dcw_local_init_second(iotx_dconn_get_local_conn());
#endif

	iotx_dmsg_cloud_connected();
}

void iotx_dcs_event_cloud_disconnect_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_CLOUD_DISCONNECT");

	iotx_dconn_set_cloud_conn_state(0);

	iotx_dmsg_cloud_disconnect();
}

void iotx_dcs_event_cloud_reconnect_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_CLOUD_RECONNECT");

	iotx_dconn_set_cloud_conn_state(1);
	
	iotx_dmsg_cloud_reconnect();
}

void iotx_dcs_event_local_connected_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_LOCAL_CONNECTED");

	iotx_dconn_set_local_conn_state(1);

	iotx_dsub_local_register();

	_iotx_dmsg_send_to_user(IOTX_DM_EVENT_LOCAL_CONNECTED,NULL);
}

void iotx_dcs_event_local_disconnect_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_LOCAL_DISCONNECT");
		
	iotx_dconn_set_local_conn_state(0);

	_iotx_dmsg_send_to_user(IOTX_DM_EVENT_LOCAL_DISCONNECT,NULL);
}

void iotx_dcs_event_local_reconnect_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_LOCAL_RECONNECT");

	iotx_dconn_set_local_conn_state(1);

	_iotx_dmsg_send_to_user(IOTX_DM_EVENT_LOCAL_RECONNECT,NULL);
}

void iotx_dcs_event_found_device_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_FOUND_DEVICE");

	iotx_cm_send_peer_t *cm_send_peer = (iotx_cm_send_peer_t *)msg->msg;

	iotx_dmsg_found_device(cm_send_peer->product_key,cm_send_peer->device_name);
}

void iotx_dcs_event_remove_device_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_REMOVE_DEVICE");

	iotx_cm_send_peer_t *cm_send_peer = (iotx_cm_send_peer_t *)msg->msg;

	iotx_dmsg_remove_device(cm_send_peer->product_key,cm_send_peer->device_name);
}

void iotx_dcs_event_register_result_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_REGISTER_RESULT");

	iotx_cm_event_result_t *cm_event_result = (iotx_cm_event_result_t *)msg->msg;

	dm_log_debug("Topic Subscribe Result: %d, URI: %s",cm_event_result->result,cm_event_result->URI);

	iotx_dmsg_register_result(cm_event_result->URI,cm_event_result->result);
}

void iotx_dcs_event_unregister_result_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_UNREGISTER_RESULT");

	//iotx_cm_event_result_t *cm_event_result = (iotx_cm_event_result_t *)msg->msg;

	//iotx_dmsg_unregister_result(cm_event_result->URI,cm_event_result->result);
}

void iotx_dcs_event_send_result_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_SEND_RESULT");

	iotx_cm_event_result_t *cm_event_result = (iotx_cm_event_result_t *)msg->msg;

	iotx_dmsg_send_result(cm_event_result->URI,cm_event_result->result);
}

void iotx_dcs_event_add_service_result_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_ADD_SERVICE_RESULT");

	iotx_cm_event_result_t *cm_event_result = (iotx_cm_event_result_t *)msg->msg;

	iotx_dmsg_add_service_result(cm_event_result->URI,cm_event_result->result);
}

void iotx_dcs_event_remove_service_result_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_REMOVE_SERVICE_RESULT");

	iotx_cm_event_result_t *cm_event_result = (iotx_cm_event_result_t *)msg->msg;

	iotx_dmsg_remove_service_result(cm_event_result->URI,cm_event_result->result);
}

void iotx_dcs_event_new_data_received_handler(void* pcontext, iotx_cm_event_msg_t* msg, void* user_data)
{
	dm_log_info("IOTX_CM_EVENT_NEW_DATA_RECEIVED");

	iotx_cm_new_data_t *cm_new_data = msg->msg;
	iotx_dcw_topic_callback(cm_new_data->peer,cm_new_data->message_info,user_data);
}

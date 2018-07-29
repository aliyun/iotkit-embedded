#include "iotx_dm_internal.h"
#include "linkkit_gateway_legacy.h"
#include "dm_opt.h"

static linkkit_gateway_legacy_ctx_t g_linkkit_gateway_legacy_ctx = {0};

static linkkit_gateway_legacy_ctx_t* _linkkit_gateway_legacy_get_ctx(void) {
    return &g_linkkit_gateway_legacy_ctx;
}

static int _linkkit_gateway_callback_list_insert(int devid, linkkit_cbs_t *callback, void *context)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();
    linkkit_gateway_dev_node_t *search_node = NULL;

    list_for_each_entry(search_node,&linkkit_gateway_ctx->callback_list,linked_list,linkkit_gateway_dev_node_t) {
        if (search_node->devid == devid) {
            dm_log_info("Device Already Exist: %d",devid);
            return SUCCESS_RETURN;
        }
    }

    linkkit_gateway_dev_node_t *node = DM_malloc(sizeof(linkkit_gateway_dev_node_t));
    if (node == NULL) {
        dm_log_err(DM_UTILS_LOG_MEMORY_NOT_ENOUGH);
        return FAIL_RETURN;
    }
    memset(node,0,sizeof(linkkit_gateway_dev_node_t));
    node->devid = devid;
    node->callback = callback;
    node->callback_ctx = context;
    INIT_LIST_HEAD(&node->linked_list);

    list_add(&node->linked_list,&linkkit_gateway_ctx->callback_list);

    return SUCCESS_RETURN;
}

static int _linkkit_gateway_callback_list_remove(int devid)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();
    linkkit_gateway_dev_node_t *search_node = NULL;

    list_for_each_entry(search_node,&linkkit_gateway_ctx->callback_list,linked_list,linkkit_gateway_dev_node_t) {
        if (search_node->devid == devid) {
            dm_log_info("Device Found: %d, Delete It",devid);
            DM_free(search_node);
            return SUCCESS_RETURN;
        }
    }

    return FAIL_RETURN;
}

static void _linkkit_gateway_callback_list_destroy(void)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();
    linkkit_gateway_dev_node_t *search_node = NULL;
    linkkit_gateway_dev_node_t *next_node = NULL;

    list_for_each_entry_safe(search_node,next_node,&linkkit_gateway_ctx->callback_list,linked_list,linkkit_gateway_dev_node_t) {
        DM_free(search_node);
    }
}

linkkit_params_t *linkkit_gateway_get_default_params(void)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    memset(&linkkit_gateway_ctx->init_params,0,sizeof(linkkit_params_t));

    /* Legacy Parameter */
    linkkit_gateway_ctx->init_params.maxMsgSize = 20 * 1024;
    linkkit_gateway_ctx->init_params.maxMsgQueueSize = 16;
    linkkit_gateway_ctx->init_params.threadPoolSize = 4;
    linkkit_gateway_ctx->init_params.threadStackSize = 8 * 1024;

    return &linkkit_gateway_ctx->init_params;
}

int linkkit_gateway_setopt(linkkit_params_t *params, int option, void *value, int value_len)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (params == NULL || value == NULL) {
        dm_log_err(DM_UTILS_LOG_INVALID_PARAMETER);
        return FAIL_RETURN;
    }

    switch (option) {
    case LINKKIT_OPT_MAX_MSG_SIZE:
        {
            if (value_len != sizeof(int)) {return FAIL_RETURN;}
            if (*((int *)value) < 256) {
                 dm_log_err("maxMsgSize should not less than 256 bytes\n");
                 return FAIL_RETURN;
            }
            linkkit_gateway_ctx->init_params.maxMsgSize = *((int *)value);
        }
        break;
    case LINKKIT_OPT_MAX_MSG_QUEUE_SIZE:
        {
            if (value_len != sizeof(int)) {return FAIL_RETURN;}
            if (*((int *)value) < 1) {
                dm_log_err("maxMsgQueueSize should not less than 1\n");
                return FAIL_RETURN;
            }
            linkkit_gateway_ctx->init_params.maxMsgQueueSize = *((int *)value);
        }
        break;
    case LINKKIT_OPT_THREAD_POOL_SIZE:
        {
            if (value_len != sizeof(int)) {return FAIL_RETURN;}
            if (*((int *)value) < 1) {
                dm_log_err("threadPoolSize should not less than 1\n");
                return FAIL_RETURN;
            }
            linkkit_gateway_ctx->init_params.threadPoolSize = *((int *)value);
        }
        break;
    case LINKKIT_OPT_THREAD_STACK_SIZE:
        {
            if (value_len != sizeof(int)) {return FAIL_RETURN;}
            if (*((int *)value) < 1024) {
                dm_log_err("threadStackSize should not less than 1024\n");
                return FAIL_RETURN;
            }
            linkkit_gateway_ctx->init_params.threadStackSize = *((int *)value);
        }
        break;
	case LINKKIT_OPT_PROPERTY_POST_REPLY:
		iotx_dm_set_opt(0,value);
		break;
	case LINKKIT_OPT_EVENT_POST_REPLY:
		iotx_dm_set_opt(1,value);
		break;
	case LINKKIT_OPT_PROPERTY_SET_REPLY:
		iotx_dm_set_opt(2,value);
		break;
    default:
        dm_log_err("unknow option: %d\n", option);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

int linkkit_gateway_set_event_callback(linkkit_params_t *params, int (*event_cb)(linkkit_event_t *ev, void *ctx), void *ctx)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (params == NULL || event_cb == NULL) {
        dm_log_err(DM_UTILS_LOG_INVALID_PARAMETER);
        return FAIL_RETURN;
    }

    linkkit_gateway_ctx->init_params.event_cb = event_cb;
    linkkit_gateway_ctx->init_params.ctx = ctx;

    return SUCCESS_RETURN;
}

int linkkit_gateway_init(linkkit_params_t *initParams)
{
    if (initParams == NULL) {
        dm_log_err(DM_UTILS_LOG_INVALID_PARAMETER);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

int linkkit_gateway_exit(void)
{
    return SUCCESS_RETURN;
}

static void _linkkit_gateway_event_callback(iotx_dm_event_types_t type, char *payload)
{

}

static void* _linkkit_gateway_dispatch(void *params)
{
	while (1) {
		iotx_dm_dispatch();
		HAL_SleepMs(20);
	}
	return NULL;
}

int linkkit_gateway_start(linkkit_cbs_t *cbs, void *ctx)
{
    int res = 0, stack_used = 0;
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();
    iotx_dm_init_params_t dm_init_params;

    if (cbs == NULL) {
        dm_log_err(DM_UTILS_LOG_INVALID_PARAMETER);
        return FAIL_RETURN;
    }

    if (linkkit_gateway_ctx->is_started) {
        dm_log_info("Linkkit Gateway Already Started");
        return SUCCESS_RETURN;
    }

    memset(linkkit_gateway_ctx,0,sizeof(linkkit_gateway_legacy_ctx_t));
    linkkit_gateway_ctx->is_started = 1;

    /* Initialize Device Manager */
    memset(&dm_init_params,0,sizeof(iotx_dm_init_params_t));
    dm_init_params.secret_type = IOTX_DM_DEVICE_SECRET_DEVICE;
    dm_init_params.domain_type = IOTX_DM_CLOUD_DOMAIN_SHANGHAI;
    dm_init_params.event_callback = _linkkit_gateway_event_callback;

    res = iotx_dm_construct(&dm_init_params);
	if (res != SUCCESS_RETURN) {
        linkkit_gateway_ctx->is_started = 0;
        return FAIL_RETURN;
    }

    /* Get Gateway TSL From Cloud */
    res = iotx_dm_set_tsl(IOTX_DM_LOCAL_NODE_DEVID,IOTX_DM_TSL_SOURCE_CLOUD,NULL,0);
	if (res != SUCCESS_RETURN) {
        linkkit_gateway_ctx->is_started = 0;
        return FAIL_RETURN;
    }

    res = HAL_ThreadCreate(&linkkit_gateway_ctx->dispatch_thread,_linkkit_gateway_dispatch,NULL,NULL,&stack_used);
	if (res != SUCCESS_RETURN) {
        iotx_dm_destroy();
        linkkit_gateway_ctx->is_started = 0;
        return FAIL_RETURN;
    }

    /* Insert Gateway Callback And Callback Context Into Device Callback Linkked List */
    INIT_LIST_HEAD(&linkkit_gateway_ctx->callback_list);
    
    res = _linkkit_gateway_callback_list_insert(IOTX_DM_LOCAL_NODE_DEVID,cbs,ctx);
    if (res != SUCCESS_RETURN) {
        linkkit_gateway_ctx->is_started = 0;
        iotx_dm_destroy();
        HAL_ThreadDelete(linkkit_gateway_ctx->dispatch_thread);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

int linkkit_gateway_stop(int devid)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (linkkit_gateway_ctx->is_started  == 0) {return FAIL_RETURN;}
    if (devid != IOTX_DM_LOCAL_NODE_DEVID) {return FAIL_RETURN;}
    
    HAL_ThreadDelete(linkkit_gateway_ctx->dispatch_thread);
    iotx_dm_destroy();
    _linkkit_gateway_callback_list_destroy();

    linkkit_gateway_ctx->is_started = 0;

    return SUCCESS_RETURN;
}

int linkkit_gateway_subdev_register(char *productKey, char *deviceName, char *deviceSecret)
{
    int res = 0, devid = 0;
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (productKey == NULL || strlen(productKey) >= PRODUCT_KEY_MAXLEN ||
        deviceName == NULL || strlen(deviceName) >= DEVICE_NAME_MAXLEN) {
        dm_log_err(DM_UTILS_LOG_INVALID_PARAMETER);
        return FAIL_RETURN;
    }

    if (linkkit_gateway_ctx->is_started == 0) {return FAIL_RETURN;}

    res = iotx_dm_legacy_get_devid_by_pkdn(productKey, deviceName, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    return iotx_dm_subdev_register(devid);
}

int linkkit_gateway_subdev_unregister(char *productKey, char *deviceName)
{
    int res = 0, devid = 0;
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (productKey == NULL || strlen(productKey) >= PRODUCT_KEY_MAXLEN ||
        deviceName == NULL || strlen(deviceName) >= DEVICE_NAME_MAXLEN) {
        dm_log_err(DM_UTILS_LOG_INVALID_PARAMETER);
        return FAIL_RETURN;
    }

    if (linkkit_gateway_ctx->is_started == 0) {return FAIL_RETURN;}

    res = iotx_dm_legacy_get_devid_by_pkdn(productKey, deviceName, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    res = iotx_dm_subdev_topo_del(devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    return iotx_dm_subdev_unregister(devid);
}

int linkkit_gateway_subdev_create(char *productKey, char *deviceName, linkkit_cbs_t *cbs, void *ctx)
{
    int res = 0, devid = 0;
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (productKey == NULL || strlen(productKey) >= PRODUCT_KEY_MAXLEN ||
        deviceName == NULL || strlen(deviceName) >= DEVICE_NAME_MAXLEN || cbs == NULL) {
        dm_log_err(DM_UTILS_LOG_INVALID_PARAMETER);
        return FAIL_RETURN;
    }

    if (linkkit_gateway_ctx->is_started == 0) {return FAIL_RETURN;}

    res = iotx_dm_subdev_create(productKey, deviceName, &devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    res = _linkkit_gateway_callback_list_insert(devid,cbs,ctx);
    if (res != SUCCESS_RETURN) {
        iotx_dm_subdev_destroy(devid);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

int linkkit_gateway_subdev_destroy(int devid)
{
    int res = 0;
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (devid < 0) {
        dm_log_err(DM_UTILS_LOG_INVALID_PARAMETER);
        return FAIL_RETURN;
    }

    if (linkkit_gateway_ctx->is_started == 0) {return FAIL_RETURN;}

    res = _linkkit_gateway_callback_list_remove(devid);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

int linkkit_gateway_subdev_login(int devid)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (linkkit_gateway_ctx->is_started == 0) {return FAIL_RETURN;}

    return iotx_dm_subdev_login(devid);
}

int linkkit_gateway_subdev_logout(int devid)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (linkkit_gateway_ctx->is_started == 0) {return FAIL_RETURN;}

    return iotx_dm_subdev_logout(devid);
}

int linkkit_gateway_get_devinfo(int devid, linkkit_devinfo_t *devinfo)
{
    int res = 0, type = 0;
    iotx_dm_dev_status_t status;
    iotx_dm_dev_avail_t available;
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (linkkit_gateway_ctx->is_started == 0) {return FAIL_RETURN;}

    if (devid < 0 || devinfo == NULL) {
        return FAIL_RETURN;
    }

    memset(devinfo, 0, sizeof(linkkit_devinfo_t));
    res = iotx_dm_legacy_get_pkdn_ptr_by_devid(devid, &(devinfo->productKey), &(devinfo->deviceName));
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }
    
    res = iotx_dm_get_device_type(devid, &type);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }
    if (type == IOTX_DM_DEVICE_GATEWAY) {
        devinfo->devtype = 0;
    } else if (type == IOTX_DM_DEVICE_SUBDEV) {
        devinfo->devtype = 1;
    } else {
        dm_log_info("wrong device type\n");
        return FAIL_RETURN;
    }

    res = iotx_dm_get_device_status(devid, &status);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }
    if (status >= IOTX_DM_DEV_STATUS_LOGINED) {
        devinfo->login = 1;
    }
    if (status == IOTX_DM_DEV_STATUS_ONLINE) {
        devinfo->online = 1;
    }

    res = iotx_dm_get_device_avail_status(devid, &available);
    if (res != SUCCESS_RETURN) {
        return FAIL_RETURN;
    }
    devinfo->state = available;

    return SUCCESS_RETURN;
}

int linkkit_gateway_trigger_event_json_sync(int devid, char *identifier, char *event, int timeout_ms)
{
    return SUCCESS_RETURN;
}

int linkkit_gateway_trigger_event_json(int devid, char *identifier, char *event, int timeout_ms, void (*func)(int retval, void *ctx), void *ctx)
{
    return SUCCESS_RETURN;
}

int linkkit_gateway_post_property_json_sync(int devid, char *json, int timeout_ms)
{
    return SUCCESS_RETURN;
}

int linkkit_gateway_post_property_json(int devid, char *property, int timeout_ms, void (*func)(int retval, void *ctx), void *ctx)
{
    return SUCCESS_RETURN;
}

int linkkit_gateway_post_rawdata(int devid, void *data, int len)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (linkkit_gateway_ctx->is_started == 0) {return FAIL_RETURN;}

    return iotx_dm_post_rawdata(devid, data, len);
}

int linkkit_gateway_fota_init(handle_service_fota_callback_fp_t callback_fp)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (linkkit_gateway_ctx->is_started == 0) {return FAIL_RETURN;}

    linkkit_gateway_ctx->fota_callback = callback_fp;

    return SUCCESS_RETURN;
}

int linkkit_gateway_invoke_fota_service(void* data_buf, int data_buf_length)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (linkkit_gateway_ctx->is_started == 0) {return FAIL_RETURN;}

    return iotx_dm_fota_perform_sync(data_buf,data_buf_length);
}

int linkkit_gateway_post_extinfos(int devid, linkkit_extinfo_t *extinfos, int nb_extinfos, int timeout_ms)
{
    return SUCCESS_RETURN;
}

int linkkit_gateway_delete_extinfos(int devid, linkkit_extinfo_t *extinfos, int nb_extinfos, int timeout_ms)
{
    return SUCCESS_RETURN;
}

int linkkit_gateway_get_num_devices(void)
{
    linkkit_gateway_legacy_ctx_t *linkkit_gateway_ctx = _linkkit_gateway_legacy_get_ctx();

    if (linkkit_gateway_ctx->is_started == 0) {return FAIL_RETURN;}
    
    return iotx_dm_subdev_number();
}
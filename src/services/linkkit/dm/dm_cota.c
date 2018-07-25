#include "iotx_dm_internal.h"
#include "iot_export_ota.h"
#include "dm_cota.h"
#include "dm_conn.h"
#include "dm_cm_wrapper.h"
#include "dm_message.h"

static dm_cota_ctx_t g_dm_cota_ctx;

static dm_cota_ctx_t* _dm_cota_get_ctx(void)
{
    return &g_dm_cota_ctx;
}

int dm_cota_init(void)
{
    int res = 0;
    dm_cota_ctx_t *ctx = _dm_cota_get_ctx();
    void *cloud_connectivity = NULL;
    void *mqtt_handle = NULL;

    memset(ctx,0,sizeof(dm_cota_ctx_t));

    /* Get Connectivity Initialized By CM */
    cloud_connectivity = dm_conn_get_cloud_conn();
    if (cloud_connectivity == NULL) {
        dm_log_err(DM_UTILS_LOG_CM_CLOUD_CONNECTIVITY_NOT_EXIST);
        return FAIL_RETURN;
    }

    /* Get MQTT Protocol Handle By Connectivity */
    res = dm_cmw_conn_get_prototol_handle(cloud_connectivity,&mqtt_handle);
    if (res != SUCCESS_RETURN) {return FAIL_RETURN;}

    HAL_GetProductKey(ctx->product_key);
    HAL_GetDeviceName(ctx->device_name);

    /* Init OTA Handle */
    ctx->ota_handle = IOT_OTA_Init(ctx->product_key,ctx->device_name,mqtt_handle);
    if (ctx->ota_handle == NULL) {
        dm_log_err(DM_UTILS_LOG_COTA_INIT_FAILED);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

int dm_cota_deinit(void)
{
     dm_cota_ctx_t *ctx = _dm_cota_get_ctx();

     if (ctx->ota_handle) {IOT_OTA_Deinit(ctx->ota_handle);}

     return SUCCESS_RETURN;
}

static int _dm_cota_send_new_config_to_user(void *ota_handle)
{
    int res = 0, message_len = 0;
    char *message = NULL;
    uint32_t config_size = 0;
    char *config_id = NULL, *sign = NULL, *sign_method = NULL, *url = NULL, *get_type = NULL;
    const char *cota_new_config_fmt = 
        "{\"configId\":\"%s\",\"configSize\":%d,\"getType\":\"%s\",\"sign\":\"%s\",\"signMethod\":\"%s\",\"url\":\"%s\"}";

    IOT_OTA_Ioctl(ota_handle, IOT_OTAG_COTA_CONFIG_ID, (void *)&config_id, 1);
    IOT_OTA_Ioctl(ota_handle, IOT_OTAG_COTA_CONFIG_SIZE, &config_size, 4);
    IOT_OTA_Ioctl(ota_handle, IOT_OTAG_COTA_SIGN, (void *)&sign, 1);
    IOT_OTA_Ioctl(ota_handle, IOT_OTAG_COTA_SIGN_METHOD, (void *)&sign_method, 1);
    IOT_OTA_Ioctl(ota_handle, IOT_OTAG_COTA_URL, (void *)&url, 1);
    IOT_OTA_Ioctl(ota_handle, IOT_OTAG_COTA_GETTYPE, (void *)&get_type, 1);

    if (config_id == NULL || sign == NULL || sign_method == NULL || url == NULL || get_type == NULL) {
        res = FAIL_RETURN;
        goto ERROR;
    }

    message_len = strlen(cota_new_config_fmt) + strlen(config_id) + DM_UTILS_UINT32_STRLEN + strlen(get_type) + 
                    strlen(sign) + strlen(sign_method) + strlen(url) + 1;
    
    message = DM_malloc(message_len);
    if (message == NULL) {
        dm_log_err(DM_UTILS_LOG_MEMORY_NOT_ENOUGH);
        res = FAIL_RETURN;
        goto ERROR;
    }
    memset(message,0,message_len);
    HAL_Snprintf(message,message_len,cota_new_config_fmt,config_id,config_size,get_type,sign,sign_method,url);

    dm_log_info("Send To User: %s",message);

    res = _dm_msg_send_to_user(IOTX_DM_EVENT_COTA_NEW_CONFIG,message);
    if (res != SUCCESS_RETURN) {
        if (message) {DM_free(message);}
        res = FAIL_RETURN;
        goto ERROR;
    }

    res = SUCCESS_RETURN;
ERROR:
    if (config_id) {free(config_id);}
    if (sign) {free(sign);}
    if (sign_method) {free(sign_method);}
    if (url) {free(url);}
    if (get_type) {free(get_type);}

    return res;
}

int dm_cota_perform_sync(_OU_ char *output, _IN_ int output_len)
{
    uint32_t file_size = 0, file_downloaded = 0, file_download = 0;
    uint32_t percent_pre = 0, percent_now = 0;
    unsigned long long report_pre = 0, report_now = 0;
    dm_cota_ctx_t *ctx = _dm_cota_get_ctx();
    
    if (output == NULL || output_len <= 0) {
        dm_log_err(DM_UTILS_LOG_INVALID_PARAMETER);
		return FAIL_RETURN;
    }

    if (ctx->ota_handle == NULL) {return FAIL_RETURN;}

    /* Prepare Write Data To Storage */
    HAL_Firmware_Persistence_Start();
    ctx->is_report_new_config = 0;

    while (1) {
        file_download = IOT_OTA_FetchYield(ctx->ota_handle,output,output_len,1);
        if (file_download < 0) {
            dm_log_err(DM_UTILS_LOG_OTA_FETCH_FAILED);
            IOT_OTA_ReportProgress(ctx->ota_handle, IOT_OTAP_FETCH_FAILED, NULL);
            HAL_Firmware_Persistence_Stop();
            return FAIL_RETURN;
        }

        /* Write Config File Into Stroage */
        HAL_Firmware_Persistence_Write(output,file_download);

        /* Get OTA information */
        IOT_OTA_Ioctl(ctx->ota_handle, IOT_OTAG_FETCHED_SIZE, &file_downloaded, 4);
        IOT_OTA_Ioctl(ctx->ota_handle, IOT_OTAG_FILE_SIZE, &file_size, 4);

        /* Calculate Download Percent And Update Report Timestamp*/
        percent_now = (file_downloaded * 100) / file_size;
        report_now = HAL_UptimeMs();

        /* Report Download Process To Cloud */
        if (report_now < report_pre) {report_pre = report_now;}
        if ((((percent_now - percent_pre) > 5) && 
            ((report_now - report_pre) > 50)) || (percent_now >= IOT_OTAP_FETCH_PERCENTAGE_MAX)) {
                IOT_OTA_ReportProgress(ctx->ota_handle, percent_now, NULL);
                percent_pre = percent_now;
                report_pre = report_now;
            }

        /* Check If OTA Finished */
        if (IOT_OTA_IsFetchFinish(ctx->ota_handle)) {
            uint32_t file_isvalid = 0;
            IOT_OTA_Ioctl(ctx->ota_handle, IOT_OTAG_CHECK_CONFIG, &file_isvalid, 4);
            if (file_isvalid == 0) {
                HAL_Firmware_Persistence_Stop();
                return FAIL_RETURN;
            }else{
                break;
            }
        }
    }

    HAL_Firmware_Persistence_Stop();
    return SUCCESS_RETURN;
}

int dm_cota_get_config(const char* config_scope, const char* get_type, const char* attribute_keys)
{
    dm_cota_ctx_t *ctx = _dm_cota_get_ctx();

    return IOT_OTA_GetConfig(ctx->ota_handle,config_scope,get_type,attribute_keys);
}

int dm_cota_status_check(void)
{
    int res = 0;
    dm_cota_ctx_t *ctx = _dm_cota_get_ctx();

    if (IOT_OTA_IsFetching(ctx->ota_handle)) {
        /* Send New Config Information To User */
        if (ctx->is_report_new_config == 0) {
            res = _dm_cota_send_new_config_to_user(ctx->ota_handle);
            if (res == SUCCESS_RETURN) {ctx->is_report_new_config = 1;}
        }
    }

    return SUCCESS_RETURN;
}
#ifdef SERVICE_OTA_ENABLED

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include "lite-utils.h"
#include "lite-system.h"

#include "iotx_cmp_ota.h"
#include "ota_internal.h"
#include "iotx_cmp_common.h"
#include "iot_export_cmp.h"
#include "iot_import.h"

static int g_cmp_ota_status = 0;

static void _fota_fetch(void* user_data, int is_fetch, uint32_t size_file, char *purl, char *version)
{
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)user_data;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return;
    }

    /* start */
    if (0 == is_fetch) {
        iotx_cmp_fota_parameter_t ota_parameter = {0};

        ota_parameter.purl = purl;
        ota_parameter.size_file = size_file;
        ota_parameter.version = version;

        if (cmp_pt->fota_func)
            cmp_pt->fota_func(cmp_pt, &ota_parameter, cmp_pt->fota_user_context);
    } else {
        g_cmp_ota_status = 1;
    }
}


static void _cota_fetch(void* user_data, int is_fetch, char* configId, uint32_t configSize, char *sign, \
                        char *signMethod, char* url, char* getType)
{
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)user_data;

    if (NULL == cmp_pt) {
        CMP_ERR(cmp_log_error_parameter);
        return;
    }

    /* start */
    if (0 == is_fetch) {
        iotx_cmp_cota_parameter_t ota_parameter = {0};

        ota_parameter.configId = configId;
        ota_parameter.configSize = configSize;
        ota_parameter.sign = sign;
        ota_parameter.signMethod = signMethod;
        ota_parameter.url = url;
        ota_parameter.getType = getType;

        if (cmp_pt->cota_func)
            cmp_pt->cota_func(cmp_pt, &ota_parameter, cmp_pt->cota_user_context);
    } else {
        g_cmp_ota_status = 1;
    }
}

void* iotx_cmp_ota_init(iotx_cmp_conntext_pt cmp_pt, const char* version)
{
    void *h_ota = NULL;
    iotx_device_info_pt pdevice_info = iotx_device_info_get();

    if (NULL == cmp_pt || NULL == version) {
        CMP_ERR(cmp_log_error_parameter);
        return NULL;
    }

    if (!cmp_pt->connectivity_list->node->is_connected) {
       CMP_WARNING(cmp_log_warning_cloud_disconnected);
       return NULL;
    }

    h_ota = IOT_OTA_Init(pdevice_info->product_key,
                    pdevice_info->device_name,
                    cmp_pt->connectivity_list->node->context);
    if (NULL == h_ota) {
        CMP_ERR(cmp_log_error_fail);
        return NULL;
    }

    cmp_pt->ota_handler = h_ota;

    if (0 != IOT_OTA_ReportVersion(h_ota, version)) {
        CMP_ERR(cmp_log_error_fail);
        IOT_OTA_Deinit(h_ota);
        return NULL;
    }

    iotx_ota_set_fetch_callback(h_ota, _fota_fetch, cmp_pt);
    iotx_ota_set_cota_fetch_callback(h_ota, _cota_fetch, cmp_pt);

    return h_ota;
}

int iotx_cmp_ota_yield(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_ota_pt ota_pt)
{
    uint32_t len, size_downloaded, size_file;
    uint32_t percent = 0;
    static uint32_t pre_percent = 0;
    static unsigned long long pre_report_time = 0;
    unsigned long long report_time = 0;

    if (NULL == cmp_pt || NULL == cmp_pt->ota_handler || NULL == ota_pt || NULL == ota_pt->buffer) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    len = IOT_OTA_FetchYield(cmp_pt->ota_handler, ota_pt->buffer, ota_pt->buffer_length, 1);
    if (len <= 0) {
        IOT_OTA_ReportProgress(cmp_pt->ota_handler, IOT_OTAP_FETCH_FAILED, NULL);
        ota_pt->is_more = 0;
        ota_pt->result = IOT_OTAP_FETCH_FAILED;
        ota_pt->progress = IOT_OTAP_FETCH_FAILED;
        CMP_ERR(cmp_log_error_fail);
        return FAIL_RETURN;
    }

    ota_pt->buffer_length = len;

    /* get OTA information */
    IOT_OTA_Ioctl(cmp_pt->ota_handler, IOT_OTAG_FETCHED_SIZE, &size_downloaded, 4);
    IOT_OTA_Ioctl(cmp_pt->ota_handler, IOT_OTAG_FILE_SIZE, &size_file, 4);
    if (IOTX_CMP_OTA_TYPE_FOTA == ota_pt->ota_type) {
        char version[128], md5sum[33];
        IOT_OTA_Ioctl(cmp_pt->ota_handler, IOT_OTAG_MD5SUM, md5sum, 33);
        IOT_OTA_Ioctl(cmp_pt->ota_handler, IOT_OTAG_VERSION, version, 128);
    }

    percent = (size_downloaded * 100) / size_file;
    report_time = HAL_UptimeMs();
    if ((((percent - pre_percent) > 5) && ((report_time - pre_report_time) > 50)) || (percent >= IOT_OTAP_FETCH_PERCENTAGE_MAX) ){
        IOT_OTA_ReportProgress(cmp_pt->ota_handler, percent, NULL);
        pre_percent = percent;
        pre_report_time = report_time;
    }

    if (!IOT_OTA_IsFetchFinish(cmp_pt->ota_handler)) {
        ota_pt->is_more = 1;
        ota_pt->result = 0;
        ota_pt->progress = percent;
    } else {
        uint32_t firmware_valid;

        ota_pt->is_more = 0;
        ota_pt->progress = percent;
        /* finished */
        if (1 == g_cmp_ota_status) {
        	if (IOTX_CMP_OTA_TYPE_FOTA == ota_pt->ota_type)
        	{
        		IOT_OTA_Ioctl(cmp_pt->ota_handler, IOT_OTAG_CHECK_FIRMWARE, &firmware_valid, 4);
				if (0 == firmware_valid) {
					CMP_ERR(cmp_log_error_fail);
					ota_pt->result = IOT_OTAP_CHECK_FALIED;
				} else {
					CMP_INFO(cmp_log_info_firmware);
					ota_pt->result = 0;
				}
        	}else if(IOTX_CMP_OTA_TYPE_COTA == ota_pt->ota_type)
        	{
        		IOT_OTA_Ioctl(cmp_pt->ota_handler, IOT_OTAG_CHECK_CONFIG, &firmware_valid, 4);
				if (0 == firmware_valid) {
					CMP_ERR(cmp_log_error_fail);
					ota_pt->result = IOT_OTAP_CHECK_FALIED;
				} else {
					CMP_INFO(cmp_log_info_firmware);
					ota_pt->result = 0;
				}
        	}
        } else {
            CMP_ERR(cmp_log_error_fail_ota);
            ota_pt->result = IOT_OTAP_GENERAL_FAILED;
        }
    }

    return SUCCESS_RETURN;
}

int iotx_cmp_ota_deinit(iotx_cmp_conntext_pt cmp_pt)
{
    if (NULL == cmp_pt || NULL == cmp_pt->ota_handler) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    return IOT_OTA_Deinit(cmp_pt->ota_handler);
}


int iotx_cmp_ota_request_image(iotx_cmp_conntext_pt cmp_pt, const char* version)
{
    if (NULL == cmp_pt || NULL == cmp_pt->ota_handler) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    return IOT_OTA_RequestImage(cmp_pt->ota_handler, version);
}


int iotx_cmp_ota_get_config(iotx_cmp_conntext_pt cmp_pt, const char* configScope, const char* getType, const char* attributeKeys)
{
    if (NULL == cmp_pt || NULL == cmp_pt->ota_handler) {
        CMP_ERR(cmp_log_error_parameter);
        return FAIL_RETURN;
    }

    return IOT_OTA_GetConfig(cmp_pt->ota_handler, configScope, getType, attributeKeys);
}

#endif /* SERVICE_OTA_ENABLED */


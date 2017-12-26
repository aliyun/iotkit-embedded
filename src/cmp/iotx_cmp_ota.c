
#ifdef CMP_SUPPORT_OTA


#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include "lite-utils.h"
#include "device.h"

#include "ota_internal.h"
#include "iotx_cmp_common.h"
#include "iot_export_cmp.h"

static int g_cmp_ota_status = 0;

static void _ota_fetch(void* user_data, int is_fetch)
{
    iotx_cmp_conntext_pt cmp_pt = (iotx_cmp_conntext_pt)user_data;
    iotx_cmp_event_msg_t event_msg = {0};

    if (NULL == cmp_pt) {
        log_info("parameter error");
        return;
    }

    /* start */
    if (0 == is_fetch) {
        event_msg.event_id = IOTX_CMP_EVENT_NEW_VERSION_DETECTED;
        event_msg.msg = NULL;

        if (cmp_pt->event_func) 
            cmp_pt->event_func(cmp_pt, &event_msg, cmp_pt->user_data);
    } else {
        g_cmp_ota_status = 1;
    }
    
}

void* iotx_cmp_ota_init(iotx_cmp_conntext_pt cmp_pt, const char* version)
{
    void *h_ota = NULL;
    iotx_device_info_pt pdevice_info = iotx_device_info_get();
    
    if (NULL == cmp_pt || NULL == cmp_pt->ota_handler || NULL == version) {
        log_info("parameter error");
        return NULL;
    }

    h_ota = IOT_OTA_Init(pdevice_info->product_key, pdevice_info->device_name, cmp_pt->cloud_connection);
    if (NULL == h_ota) {
        log_info("initialize OTA failed");
        return NULL;
    }

    if (0 != IOT_OTA_ReportVersion(h_ota, version)) {
        log_info("report OTA version failed");
        IOT_OTA_Deinit(h_ota);
        return NULL;
    }

    iotx_ota_set_fetch_callback(h_ota, _ota_fetch, cmp_pt);

    return h_ota;
}

int iotx_cmp_ota_yield(iotx_cmp_conntext_pt cmp_pt, iotx_cmp_ota_pt ota_pt)
{
    uint32_t len, size_downloaded, size_file;
    uint32_t percent = 0;    

    if (NULL == cmp_pt || NULL == cmp_pt->ota_handler || NULL == ota_pt || NULL == ota_pt->buffer) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    len = IOT_OTA_FetchYield(cmp_pt->ota_handler, ota_pt->buffer, ota_pt->buffer_length, 1);
    if (len <= 0) {
        IOT_OTA_ReportProgress(cmp_pt->ota_handler, IOT_OTAP_FETCH_FAILED, NULL);
        ota_pt->is_more = 0;
        ota_pt->result = IOT_OTAP_FETCH_FAILED;
        ota_pt->progress = IOT_OTAP_FETCH_FAILED;
        log_info("ota fetch fail");
        return FAIL_RETURN;
    }

    /* get OTA information */
    IOT_OTA_Ioctl(cmp_pt->ota_handler, IOT_OTAG_FETCHED_SIZE, &size_downloaded, 4);
    IOT_OTA_Ioctl(cmp_pt->ota_handler, IOT_OTAG_FILE_SIZE, &size_file, 4);

    percent = (size_downloaded * 100) / size_file;
    IOT_OTA_ReportProgress(cmp_pt->ota_handler, percent, NULL);
    if (percent < IOT_OTAP_FETCH_PERCENTAGE_MAX) {
        ota_pt->is_more = 1;
        ota_pt->result = 0;
        ota_pt->progress = percent;
    } else {
        uint32_t firmware_valid;
        
        ota_pt->is_more = 0;
        ota_pt->progress = percent;
        /* finished */
        if (1 == g_cmp_ota_status) {
            IOT_OTA_Ioctl(cmp_pt->ota_handler, IOT_OTAG_CHECK_FIRMWARE, &firmware_valid, 4);
            if (0 == firmware_valid) {
                log_info("The firmware is invalid");
                ota_pt->result = IOT_OTAP_CHECK_FALIED;
            } else {
                log_info("The firmware is valid");
                ota_pt->result = 1;
            }
        } else {
            log_info("ota is not finished, error");
            ota_pt->result = IOT_OTAP_GENERAL_FAILED;
        }
    }
    
    return SUCCESS_RETURN;        
}


int iotx_cmp_ota_deinit(iotx_cmp_conntext_pt cmp_pt)
{
    if (NULL == cmp_pt || NULL == cmp_pt->ota_handler) {
        log_info("parameter error");
        return FAIL_RETURN;
    }

    return IOT_OTA_Deinit(cmp_pt->ota_handler);
}

#endif /* CMP_SUPPORT_OTA */


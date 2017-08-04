
#ifndef __OTA_FETCH_C_H__
#define __OTA_FETCH_C_H__

#include <string.h>
#include "iot_import_ota.h"

//ofc, OTA fetch channel

typedef struct {

    const char *url;
    httpclient_t http;          //http client
    httpclient_data_t http_data;//http client data

}otahttp_Struct_t, *otahttp_Struct_pt;


void *ofc_Init(const char *url)
{
    otahttp_Struct_pt h_odc;

    if (NULL == (h_odc = OTA_MALLOC(sizeof(otahttp_Struct_t)))) {
        OTA_LOG_ERROR("malloc failed");
        return NULL;
    }

    memset(h_odc, 0, sizeof(otahttp_Struct_t));

    //set http request-header parameter
    h_odc->http.header = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
                         "Accept-Encoding: gzip, deflate\r\n";

    h_odc->url = url;

    return h_odc;
}


int32_t ofc_Fetch(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_ms)
{
    int diff;
    otahttp_Struct_pt h_odc = (otahttp_Struct_pt)handle;

    h_odc->http_data.response_buf = buf;
    h_odc->http_data.response_buf_len = buf_len;
    diff = h_odc->http_data.response_content_len - h_odc->http_data.retrieve_len;;

    if (0 != httpclient_common(&h_odc->http, h_odc->url, 80, NULL, HTTPCLIENT_GET, timeout_ms, &h_odc->http_data)){
        OTA_LOG_ERROR("fetch firmware failed");
        return -1;
    }

    return h_odc->http_data.response_content_len - h_odc->http_data.retrieve_len - diff;
}


int ofc_Deinit(void *handle)
{
    if (NULL != handle) {
        OTA_FREE(handle);
    }

    return 0;
}

#endif

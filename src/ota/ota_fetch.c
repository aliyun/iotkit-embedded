/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include "iotx_ota_internal.h"

/* ofc, OTA fetch channel */

typedef struct {
    char *payload;
    int alread_download;
    int payload_len;
} ota_http_response_t;

typedef struct {

    const char *url;
    void *http_handle;
    int fetch_size;
} otahttp_Struct_t, *otahttp_Struct_pt;

void *ofc_Init(char *url)
{
    otahttp_Struct_pt h_odc;
    char *header = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
                         "Accept-Encoding: gzip, deflate\r\n";
#if defined(SUPPORT_TLS)
    extern const char *iotx_ca_crt;
    char *pub_key = (char *)iotx_ca_crt;
    int port = 443;
#else
    char *pub_key = NULL;
    int port = 80;
#endif
    iotx_http_method_t method = IOTX_HTTP_GET;

    if (NULL == (h_odc = OTA_MALLOC(sizeof(otahttp_Struct_t)))) {
        OTA_LOG_ERROR("allocate for h_odc failed");
        return NULL;
    }

    memset(h_odc, 0, sizeof(otahttp_Struct_t));

    h_odc->http_handle = wrapper_http_init();
    if (h_odc->http_handle == NULL) {
        OTA_FREE(h_odc);
        return NULL;
    }

#if defined(SUPPORT_ITLS)
    char *s_ptr = strstr(url, "://");
    if (strlen("https") == (s_ptr - url) && (0 == strncmp(url, "https", strlen("https")))) {
        strncpy(url + 1, url, strlen("http"));
        url++;
    }
#endif

    wrapper_http_setopt(h_odc->http_handle, IOTX_HTTPOPT_URL, (void *)url);
    wrapper_http_setopt(h_odc->http_handle, IOTX_HTTPOPT_PORT, (void *)&port);
    wrapper_http_setopt(h_odc->http_handle, IOTX_HTTPOPT_METHOD, (void *)&method);
    wrapper_http_setopt(h_odc->http_handle, IOTX_HTTPOPT_HEADER, (void *)header);
    wrapper_http_setopt(h_odc->http_handle, IOTX_HTTPOPT_CERT, (void *)pub_key);

    return h_odc;
}

static int _ota_fetch_callback(char *ptr, int length, int total_length, void *userdata)
{
    ota_http_response_t *response = (ota_http_response_t *)userdata;
    if (response->alread_download + length > response->payload_len) {
        return FAIL_RETURN;
    }

    memcpy(response->payload + response->alread_download, ptr, length);
    response->alread_download += length;

    return length;
}

int32_t ofc_Fetch(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s)
{
    int                 current_fetch_size = 0;
    int                 http_timeout_s = timeout_s * 1000;
    int                 http_recv_maxlen = buf_len;
    otahttp_Struct_pt   h_odc = (otahttp_Struct_pt)handle;
    ota_http_response_t response;

    memset(&response, 0, sizeof(ota_http_response_t));
    memset(buf, 0, buf_len);
    response.payload = buf;
    response.payload_len = buf_len;

    wrapper_http_setopt(h_odc->http_handle, IOTX_HTTPOPT_TIMEOUT, (void *)&http_timeout_s);
    wrapper_http_setopt(h_odc->http_handle, IOTX_HTTPOPT_RECVCALLBACK, (void *)_ota_fetch_callback);
    wrapper_http_setopt(h_odc->http_handle, IOTX_HTTPOPT_RECVMAXLEN, (void *)&http_recv_maxlen);
    wrapper_http_setopt(h_odc->http_handle, IOTX_HTTPOPT_RECVCONTEXT, (void *)&response);
    current_fetch_size = wrapper_http_perform(h_odc->http_handle,NULL,0);

    if (current_fetch_size < 0) {
        OTA_LOG_ERROR("fetch firmware failed");
        return -1;
    }

    h_odc->fetch_size += current_fetch_size;

/*     OTA_LOG_ERROR("Download This Time: %d",current_fetch_size);
    OTA_LOG_ERROR("Download Total    : %d",h_odc->fetch_size); */

    return current_fetch_size;
}


int ofc_Deinit(void *handle)
{
    otahttp_Struct_pt h_odc = (otahttp_Struct_pt)handle;

    wrapper_http_deinit(&h_odc->http_handle);

    if (NULL != handle) {
        OTA_FREE(handle);
    }

    return 0;
}




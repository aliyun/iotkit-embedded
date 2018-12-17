/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include <stdio.h>
#include "string.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_string.h"
#include "infra_httpc.h"
#include "dynreg_internal.h"
#include "dynreg_wrapper.h"

#define HTTP_RESPONSE_PAYLOAD_LEN           256

#define DYNREG_RANDOM_KEY_LENGTH            (15)
#define DYNREG_SIGN_LENGTH                  (65)

static int _calc_dynreg_sign(
            char product_key[IOTX_PRODUCT_KEY_LEN],
            char product_secret[IOTX_PRODUCT_SECRET_LEN],
            char device_name[IOTX_DEVICE_NAME_LEN],
            char random[DYNREG_RANDOM_KEY_LENGTH + 1],
            char sign[DYNREG_SIGN_LENGTH])
{
    int res = 0, sign_source_len = 0;
    uint8_t signnum[32];
    uint8_t  *sign_source = NULL;
    const char *dynamic_register_sign_fmt = "deviceName%sproductKey%srandom%s";

    /* Start Dynamic Register */
    res = infra_randstr(random, DYNREG_RANDOM_KEY_LENGTH);
    if (res != SUCCESS_RETURN) {
        dynreg_err("Random Key Generate Failed");
    }
    dynreg_info("Random Key: %s", random);

    /* Calculate SHA256 Value */
    sign_source_len = strlen(dynamic_register_sign_fmt) + strlen(device_name) + strlen(product_key) + strlen(random) + 1;
    sign_source = dynreg_malloc(sign_source_len);
    if (sign_source == NULL) {
        dynreg_err("Memory Not Enough");
        return FAIL_RETURN;
    }
    memset(sign_source, 0, sign_source_len);
    HAL_Snprintf((char *)sign_source, sign_source_len, dynamic_register_sign_fmt, device_name, product_key, random);

    algo_hmac_sha256_wrapper(sign_source,strlen((const char *)sign_source),(uint8_t *)product_secret,strlen(product_secret),signnum);
    infra_hex2str(signnum,32,sign);
    dynreg_free(sign_source);
    dynreg_info("Sign: %s", sign);

    return SUCCESS_RETURN;
}

static int _fetch_dynreg_http_resp(char *request_payload, char *response_payload,
                                   char device_secret[IOTX_DEVICE_SECRET_LEN])
{
    int                 res = 0;
    const char         *domain = NULL;
    const char         *url_format = "http://%s/auth/register/device";
    char               *url = NULL;
    int                 url_len = 0;
    httpclient_t        http_client;
    httpclient_data_t   http_client_data;
    lite_cjson_t        lite, lite_item_code, lite_item_data, lite_item_ds;

    memset(&http_client, 0, sizeof(httpclient_t));
    memset(&http_client_data, 0, sizeof(httpclient_data_t));

    domain = iotx_guider_get_domain(GUIDER_DOMAIN_HTTP);
    if (NULL == domain) {
        sdk_err("Get domain failed");
        return FAIL_RETURN;
    }
    url_len = strlen(url_format) + strlen(domain) + 1;
    url = (char *)dynreg_malloc(url_len);
    if (NULL == url) {
        sdk_err("Not Enough Memory");
        return FAIL_RETURN;
    }
    memset(url, 0, url_len);
    HAL_Snprintf(url, url_len, url_format, domain);

    http_client.header = "Accept: text/xml,text/javascript,text/html,application/json\r\n";

    http_client_data.post_content_type = "application/x-www-form-urlencoded";
    http_client_data.post_buf = request_payload;
    http_client_data.post_buf_len = strlen(request_payload);
    http_client_data.response_buf = response_payload;
    http_client_data.response_buf_len = HTTP_RESPONSE_PAYLOAD_LEN;

    res = httpclient_common(&http_client, url, 443, iotx_ca_get(), HTTPCLIENT_POST, 10000, &http_client_data);
    if (res != SUCCESS_RETURN) {
        sdk_err("Http Download Failed");
        dynreg_free(url);
        return FAIL_RETURN;
    }
    dynreg_free(url);
    sdk_info("Http Response Payload: %s", http_client_data.response_buf);

    /* Parse Http Response */
    memset(&lite, 0, sizeof(lite_cjson_t));
    res = lite_cjson_parse(response_payload, strlen(response_payload), &lite);
    if (res != SUCCESS_RETURN || !lite_cjson_is_object(&lite)) {
        sdk_err("Http Response Payload Parse Failed");
        return FAIL_RETURN;
    }

    /* Parse Code */
    memset(&lite_item_code, 0, sizeof(lite_cjson_t));
    res = lite_cjson_object_item(&lite, "code", strlen("code"), &lite_item_code);
    if (res != SUCCESS_RETURN || !lite_cjson_is_number(&lite_item_code)) {
        sdk_err("Http Response Payload Parse Failed");
        return FAIL_RETURN;
    }
    sdk_info("Dynamic Register Code: %d", lite_item_code.value_int);

    if (lite_item_code.value_int != 200) {
        return FAIL_RETURN;
    }

    /* Parse Data */
    memset(&lite_item_data, 0, sizeof(lite_cjson_t));
    res = lite_cjson_object_item(&lite, "data", strlen("data"), &lite_item_data);
    if (res != SUCCESS_RETURN || !lite_cjson_is_object(&lite_item_data)) {
        sdk_err("Http Response Payload Parse Failed");
        return FAIL_RETURN;
    }

    /* Ignore ProductKey and DeviceName, just parse DeviceSecret */
    memset(&lite_item_ds, 0, sizeof(lite_cjson_t));
    res = lite_cjson_object_item(&lite_item_data, "deviceSecret", strlen("deviceSecret"), &lite_item_ds);
    if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_ds)) {
        sdk_err("Http Response Payload Parse Failed");
        return FAIL_RETURN;
    }
    sdk_info("Dynamic Register Device Secret: %.*s", lite_item_ds.value_length, lite_item_ds.value);
    if (lite_item_ds.value_length >= DEVICE_SECRET_MAXLEN) {
        return FAIL_RETURN;
    }

    memcpy(device_secret, lite_item_ds.value, lite_item_ds.value_length);

    return SUCCESS_RETURN;
}

int perform_dynamic_register(_IN_ char product_key[PRODUCT_KEY_MAXLEN],
                             _IN_ char product_secret[PRODUCT_SECRET_MAXLEN],
                             _IN_ char device_name[DEVICE_NAME_MAXLEN],
                             _OU_ char device_secret[DEVICE_SECRET_MAXLEN])
{
    int             res = 0, dynamic_register_request_len = 0;
    char            sign[DYNREG_SIGN_LENGTH] = {0};
    char            random[DYNREG_RANDOM_KEY_LENGTH + 1] = {0};
    const char     *dynamic_register_format = "productKey=%s&deviceName=%s&random=%s&sign=%s&signMethod=%s";
    char           *dynamic_register_request = NULL;
    char           *dynamic_register_response = NULL;

    if ((product_key == NULL || strlen(product_key) >= PRODUCT_KEY_MAXLEN) ||
        (product_secret == NULL || strlen(product_secret) >= PRODUCT_SECRET_MAXLEN) ||
        (device_name == NULL || strlen(device_name) >= DEVICE_NAME_MAXLEN) ||
        (device_secret == NULL)) {
        sdk_err("Invalid Parameter");
        return FAIL_RETURN;
    }

    /* Calcute Signature */
    res = _calc_dynreg_sign(product_key, product_secret, device_name, random, sign);
    if (res != SUCCESS_RETURN) {
        sdk_err("Calculate Sign Failed");
        return FAIL_RETURN;
    }

    /* Assemble Http Dynamic Register Request Payload */
    dynamic_register_request_len = strlen(dynamic_register_format) + strlen(product_key) + strlen(device_name) +
                                   strlen(random) + strlen(sign) + strlen(DYNAMIC_REGISTER_SIGN_METHOD_HMACSHA256) + 1;
    dynamic_register_request = dynreg_malloc(dynamic_register_request_len);
    if (dynamic_register_request == NULL) {
        sdk_err("Not Enough Memory");
        return FAIL_RETURN;
    }
    memset(dynamic_register_request, 0, dynamic_register_request_len);
    HAL_Snprintf(dynamic_register_request, dynamic_register_request_len, dynamic_register_format,
                 product_key, device_name, random, sign, DYNAMIC_REGISTER_SIGN_METHOD_HMACSHA256);

    dynamic_register_response = dynreg_malloc(HTTP_RESPONSE_PAYLOAD_LEN);
    if (dynamic_register_response == NULL) {
        sdk_err("Not Enough Memory");
        dynreg_free(dynamic_register_request);
        return FAIL_RETURN;
    }

    /* Send Http Request For Getting Device Secret */
    res = _fetch_dynreg_http_resp(dynamic_register_request, dynamic_register_response, device_secret);
    dynreg_free(dynamic_register_request);
    dynreg_free(dynamic_register_response);
    if (res != SUCCESS_RETURN) {
        sdk_err("Get Device Secret Failed");
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

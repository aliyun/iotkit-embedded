
/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "iotx_utils.h"
#include "iotx_system.h"
#include "sdk-impl_internal.h"

static sdk_impl_ctx_t g_sdk_impl_ctx = {0};

static sdk_impl_ctx_t *_iotx_sdk_impl_get_ctx(void)
{
    return &g_sdk_impl_ctx;
}

static const char *_get_domain_region(void)
{
    sdk_impl_ctx_t *ctx = _iotx_sdk_impl_get_ctx();

    if (ctx->domain_type == 0) {
        return DYNAMIC_REGISTER_REGION_SHANGHAI;
    } else if (ctx->domain_type == 1) {
        return DYNAMIC_REGISTER_REGION_SOUTHEAST;
    } else if (ctx->domain_type == 2) {
        return DYNAMIC_REGISTER_REGION_NORTHEAST;
    } else if (ctx->domain_type == 3) {
        return DYNAMIC_REGISTER_REGION_US_WEST;
    } else if (ctx->domain_type == 4) {
        return DYNAMIC_REGISTER_REGION_EU_CENTRAL;
    } else {
        sdk_err("Unknown Region Type");
        return NULL;
    }

    return NULL;
}

static int _calc_dynreg_sign(
            _IN_ char product_key[PRODUCT_KEY_MAXLEN],
            _IN_ char product_secret[PRODUCT_SECRET_MAXLEN],
            _IN_ char device_name[DEVICE_NAME_MAXLEN],
            _OU_ char random[DYNAMIC_REGISTER_RANDOM_KEY_LENGTH + 1],
            _OU_ char sign[DYNAMIC_REGISTER_SIGN_LENGTH])
{
    int             res = 0, sign_source_len = 0;
    char           *sign_source = NULL;
    const char     *dynamic_register_sign_fmt = "deviceName%sproductKey%srandom%s";

    /* Start Dynamic Register */
    res = LITE_get_randstr(random, DYNAMIC_REGISTER_RANDOM_KEY_LENGTH);
    if (res != SUCCESS_RETURN) {
        sdk_err("Random Key Generate Failed");
    }
    sdk_info("Random Key: %s", random);

    /* Calculate SHA256 Value */
    sign_source_len = strlen(dynamic_register_sign_fmt) + strlen(device_name) + strlen(product_key) + strlen(random) + 1;
    sign_source = LITE_malloc(sign_source_len);
    if (sign_source == NULL) {
        sdk_err("Memory Not Enough");
        return FAIL_RETURN;
    }
    memset(sign_source, 0, sign_source_len);
    HAL_Snprintf(sign_source, sign_source_len, dynamic_register_sign_fmt, device_name, product_key, random);

    utils_hmac_sha256(sign_source, strlen(sign_source), sign, product_secret, strlen(product_secret));
    LITE_free(sign_source);
    sdk_info("Sign: %s", sign);

    return SUCCESS_RETURN;
}

static int _fetch_dynreg_http_resp(_IN_ char *request_payload, _OU_ char device_secret[DEVICE_SECRET_MAXLEN])
{
    int                 res = 0;
    char                response_payload[256] = {0};
    const char         *url = NULL;
    httpclient_t        http_client;
    httpclient_data_t   http_client_data;
    lite_cjson_t        lite, lite_item_code, lite_item_data, lite_item_pk, lite_item_dn, lite_item_ds;

    memset(&http_client, 0, sizeof(httpclient_t));
    memset(&http_client_data, 0, sizeof(httpclient_data_t));

    url = _get_domain_region();
    if (url == NULL) {
        sdk_err("Invalid Url");
        return FAIL_RETURN;
    }

    http_client.header = "Accept: text/xml,text/javascript,text/html,application/json\r\n";

    http_client_data.post_content_type = "application/x-www-form-urlencoded";
    http_client_data.post_buf = request_payload;
    http_client_data.post_buf_len = strlen(request_payload);
    http_client_data.response_buf = response_payload;
    http_client_data.response_buf_len = 256;

    res = httpclient_common(&http_client, url, 443, iotx_ca_get(), HTTPCLIENT_POST, 10000, &http_client_data);
    if (res != SUCCESS_RETURN) {
        sdk_err("Http Download Failed");
        return FAIL_RETURN;
    }
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

    /* Parse Product Key */
    memset(&lite_item_pk, 0, sizeof(lite_cjson_t));
    res = lite_cjson_object_item(&lite_item_data, "productKey", strlen("productKey"), &lite_item_pk);
    if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_pk)) {
        sdk_err("Http Response Payload Parse Failed");
        return FAIL_RETURN;
    }
    sdk_info("Dynamic Register Product Key: %.*s", lite_item_pk.value_length, lite_item_pk.value);

    /* Parse Device Name */
    memset(&lite_item_dn, 0, sizeof(lite_cjson_t));
    res = lite_cjson_object_item(&lite_item_data, "deviceName", strlen("deviceName"), &lite_item_dn);
    if (res != SUCCESS_RETURN || !lite_cjson_is_string(&lite_item_dn)) {
        sdk_err("Http Response Payload Parse Failed");
        return FAIL_RETURN;
    }
    sdk_info("Dynamic Register Device Name: %.*s", lite_item_dn.value_length, lite_item_dn.value);

    /* Parse Device Secret */
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

static int _perform_dynamic_register(_IN_ char product_key[PRODUCT_KEY_MAXLEN],
                                     _IN_ char product_secret[PRODUCT_SECRET_MAXLEN],
                                     _IN_ char device_name[DEVICE_NAME_MAXLEN], 
                                     _OU_ char device_secret[DEVICE_SECRET_MAXLEN])
{
    int             res = 0, dynamic_register_request_len = 0;
    char            sign[DYNAMIC_REGISTER_SIGN_LENGTH] = {0};
    char            random[DYNAMIC_REGISTER_RANDOM_KEY_LENGTH + 1] = {0};
    const char     *dynamic_register_format = "productKey=%s&deviceName=%s&random=%s&sign=%s&signMethod=%s";
    char           *dynamic_register_request = NULL;

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
    dynamic_register_request = LITE_malloc(dynamic_register_request_len);
    if (dynamic_register_request == NULL) {
        sdk_err("Not Enough Memory");
        return FAIL_RETURN;
    }
    memset(dynamic_register_request, 0, dynamic_register_request_len);
    HAL_Snprintf(dynamic_register_request, dynamic_register_request_len, dynamic_register_format,
                 product_key, device_name, random, sign, DYNAMIC_REGISTER_SIGN_METHOD_HMACSHA256);

    /* Send Http Request For Getting Device Secret */
    res = _fetch_dynreg_http_resp(dynamic_register_request, device_secret);
    LITE_free(dynamic_register_request);
    if (res != SUCCESS_RETURN) {
        sdk_err("Get Device Secret Failed");
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

void IOT_OpenLog(const char *ident)
{
    const char         *mod = ident;

    if (NULL == mod) {
        mod = "---";
    }

    LITE_openlog(mod);
}

void IOT_CloseLog(void)
{
    LITE_closelog();
}

void IOT_SetLogLevel(IOT_LogLevel level)
{
    LOGLEVEL            lvl = (LOGLEVEL)level;

    if (lvl > LOG_DEBUG_LEVEL) {
        sdk_err("Invalid input level: %d out of [%d, %d]", level,
                LOG_EMERG_LEVEL,
                LOG_DEBUG_LEVEL);
        return;
    }

    LITE_set_loglevel(lvl);
}

void IOT_DumpMemoryStats(IOT_LogLevel level)
{
    LOGLEVEL            lvl = (LOGLEVEL)level;

    if (lvl > LOG_DEBUG_LEVEL) {
        lvl = LOG_DEBUG_LEVEL;
        sdk_warning("Invalid input level, using default: %d => %d", level, lvl);
    }

    LITE_dump_malloc_free_stats(lvl);
}

#if defined(MQTT_COMM_ENABLED)
int IOT_SetupConnInfo(const char *product_key,
                      const char *device_name,
                      const char *device_secret,
                      void **info_ptr)
{
    int                 rc = 0;
    char                device_secret_actual[DEVICE_SECRET_MAXLEN] = {0};
    char                product_secret[PRODUCT_SECRET_MAXLEN] = {0};
    sdk_impl_ctx_t     *ctx = _iotx_sdk_impl_get_ctx();

    if (!info_ptr) {
        sdk_err("Invalid argument, info_ptr = %p", info_ptr);
        return -1;
    }

    STRING_PTR_SANITY_CHECK(product_key, -1);
    STRING_PTR_SANITY_CHECK(device_name, -1);

    /* Dynamic Register Device If Need */
    if (ctx->dynamic_register == 0) {
        STRING_PTR_SANITY_CHECK(device_secret, -1);
        memcpy(device_secret_actual, device_secret, strlen(device_secret));
    } else {
        sdk_info("Now We Need Dynamic Register...");
        /* Check If Product Secret Exist */
        HAL_GetProductSecret(product_secret);
        if (strlen(product_secret) == 0) {
            sdk_err("Product Secret Is Not Exist");
            return FAIL_RETURN;
        }
        STRING_PTR_SANITY_CHECK(product_secret, -1);

        rc = _perform_dynamic_register((char *)product_key, (char *)product_secret, (char *)device_name, device_secret_actual);
        if (rc != SUCCESS_RETURN) {
            sdk_err("Dynamic Register Failed");
            return FAIL_RETURN;
        }
        HAL_SetDeviceSecret(device_secret_actual);
    }

    iotx_device_info_init();
    iotx_device_info_set(product_key, device_name, device_secret_actual);

    if (0 == iotx_guider_auth_get()) {
        rc = iotx_guider_authenticate();
    }
    if (rc == 0) {
        iotx_guider_auth_set(1);
        *info_ptr = (void *)iotx_conn_info_get();
    } else {
        iotx_guider_auth_set(0);
        *info_ptr = NULL;
    }

    return rc;
}

#endif  /* #if defined(MQTT_COMM_ENABLED)   */

int IOT_Ioctl(int option, void *data)
{
    int                 res = SUCCESS_RETURN;
    sdk_impl_ctx_t     *ctx = _iotx_sdk_impl_get_ctx();

    if (option < 0 || data == NULL) {
        sdk_err("Invalid Parameter");
        return FAIL_RETURN;
    }

    switch (option) {
        case IOTX_IOCTL_SET_DOMAIN: {
            ctx->domain_type = *(int *)data;
            iotx_guider_set_domain_type(*(int *)data);

            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_SET_DYNAMIC_REGISTER: {
            ctx->dynamic_register = *(int *)data;

            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_GET_DYNAMIC_REGISTER: {
            *(int *)data = ctx->dynamic_register;

            res = SUCCESS_RETURN;
        }
        break;
        default: {
            sdk_err("Unknown Ioctl Option");
            res = FAIL_RETURN;
        }
        break;
    }

    return res;
}

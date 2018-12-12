/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include "config.h"
#include "md.h"
#include "sha256.h"
#include "infra_types.h"

uint8_t algo_sha256_wrapper(_IN_ uint8_t *input, _IN_ uint16_t input_len, _OU_ uint8_t output[32])
{
    mbedtls_sha256(input,input_len,output,0);
    return 0;
}

uint8_t algo_hmac_sha256_wrapper(_IN_ uint8_t *input, _IN_ uint16_t input_len, _IN_ uint8_t *key, _IN_ uint16_t key_len, _OU_ uint8_t output[32])
{
    int res = 0;
    mbedtls_md_context_t md_ctx;

    mbedtls_md_init(&md_ctx);
    res = mbedtls_md_setup(&md_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    if (res != 0)
    {
        printf("mbedtls setup error\n");
        return -1;
    }

    mbedtls_md_hmac_starts(&md_ctx, key, key_len);
    mbedtls_md_hmac_update(&md_ctx, input, input_len);
    mbedtls_md_hmac_finish(&md_ctx, output);
    mbedtls_md_free(&md_ctx);

    return 0;
}
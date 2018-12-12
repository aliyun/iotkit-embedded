/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include "ref_sha256.h"

uint8_t algo_sha256_wrapper(uint8_t *input, uint16_t input_len, uint8_t output[32])
{
    utils_sha256(input, input_len,output);
    return 0;
}

uint8_t algo_hmac_sha256_wrapper(uint8_t *input, uint16_t input_len, uint8_t *key, uint16_t key_len, uint8_t output[32])
{
    utils_hmac_sha256(input, input_len, key, key_len, output);
    return 0;
}
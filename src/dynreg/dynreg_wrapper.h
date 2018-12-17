#ifndef _DYNREG_WRAPPER_H_
#define _DYNREG_WRAPPER_H_

#include "infra_types.h"

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);

uint8_t algo_hmac_sha256_wrapper(uint8_t *input, uint16_t input_len, uint8_t *key, uint16_t key_len, uint8_t output[32]);

#endif
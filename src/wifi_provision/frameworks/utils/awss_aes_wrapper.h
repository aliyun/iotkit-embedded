#include "infra_compat.h"

typedef void *p_Aes128_t;

p_Aes128_t awss_Aes128_Init(
            const uint8_t *key,
            const uint8_t *iv,
            AES_DIR_t dir);

int awss_Aes128_Destroy(p_Aes128_t aes);

int awss_Aes128_Cbc_Decrypt(
            p_Aes128_t aes,
            const void *src,
            size_t blockNum,
            void *dst);

int awss_Aes128_Cfb_Decrypt(
            p_Aes128_t aes,
            const void *src,
            size_t length,
            void *dst);

int awss_Aes128_Cfb_Encrypt(
            p_Aes128_t aes,
            const void *src,
            size_t length,
            void *dst);

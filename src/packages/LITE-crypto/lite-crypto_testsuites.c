#include "lite-crypto_internal.h"

#if UT_FOR_LITE_CRYPTO
int unittest_base64(const char *arg)
{
#if WITH_BASE64_ENCODE
    const unsigned char    *from = (const unsigned char *)arg;
    char                   *res;
    int                     res_len = 0;
    int                     i;
    int                     rc = -1;

    res_len = LITE_BASE64_DEST_LEN(strlen((char *)from));

    res = LITE_malloc(res_len + 1);
    LITE_printf("\r\n");
    log_info("%12s : '%s'", "SOURCE-TEXT", from);
    rc = LITE_base64_encode(from, strlen((char *)from), res, res_len);
    if (rc) {
        LITE_free(res);
        return rc;
    }

    LITE_printf("[inf] %s(%d): %12s : '", __func__, __LINE__, "ENCODED");
    for (i = 0; i < res_len; ++i) {
        LITE_printf("%c", res[i]);
    }
    LITE_printf("'\r\n");

#if WITH_BASE64_DECODE
    unsigned char          *orig;

    orig = LITE_malloc(res_len);
    rc = LITE_base64_decode(res, strlen(res), orig, res_len);
    if (rc < 0) {
        LITE_free(res);
        return rc;
    }
    log_info("%12s : '%s'", "DECODED", orig);
    LITE_free(orig);
#endif  /* WITH_BASE64_DECODE */

    LITE_free(res);
#endif  /* WITH_BASE64_ENCODE */

    return 0;
}

#if WITH_MD5_DIGEST
static unsigned char md5_test_buf[7][81] = {
    { "" },
    { "a" },
    { "abc" },
    { "message digest" },
    { "abcdefghijklmnopqrstuvwxyz" },
    { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" },
    {
        "12345678901234567890123456789012345678901234567890123456789012" \
        "345678901234567890"
    }
};

static const int md5_test_buflen[7] = {
    0, 1, 3, 14, 26, 62, 80
};

static const unsigned char md5_test_sum[7][16] = {
    {
        0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04,
        0xE9, 0x80, 0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E
    },
    {
        0x0C, 0xC1, 0x75, 0xB9, 0xC0, 0xF1, 0xB6, 0xA8,
        0x31, 0xC3, 0x99, 0xE2, 0x69, 0x77, 0x26, 0x61
    },
    {
        0x90, 0x01, 0x50, 0x98, 0x3C, 0xD2, 0x4F, 0xB0,
        0xD6, 0x96, 0x3F, 0x7D, 0x28, 0xE1, 0x7F, 0x72
    },
    {
        0xF9, 0x6B, 0x69, 0x7D, 0x7C, 0xB7, 0x93, 0x8D,
        0x52, 0x5A, 0x2F, 0x31, 0xAA, 0xF1, 0x61, 0xD0
    },
    {
        0xC3, 0xFC, 0xD3, 0xD7, 0x61, 0x92, 0xE4, 0x00,
        0x7D, 0xFB, 0x49, 0x6C, 0xCA, 0x67, 0xE1, 0x3B
    },
    {
        0xD1, 0x74, 0xAB, 0x98, 0xD2, 0x77, 0xD9, 0xF5,
        0xA5, 0x61, 0x1C, 0x2C, 0x9F, 0x41, 0x9D, 0x9F
    },
    {
        0x57, 0xED, 0xF4, 0xA2, 0x2B, 0xE3, 0xC9, 0x55,
        0xAC, 0x49, 0xDA, 0x2E, 0x21, 0x07, 0xB6, 0x7A
    }
};

#if WITH_MD5_HMAC
/*
 * RFC 2202 test vectors
 */
static unsigned char md5_hmac_test_key[4][26] = {
    { "\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B\x0B" },
    { "Jefe" },
    { "\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA" },
    {
        "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10"
        "\x11\x12\x13\x14\x15\x16\x17\x18\x19"
    },
};

static const int md5_hmac_test_keylen[4] = {
    16, 4, 16, 25
};

static unsigned char md5_hmac_test_buf[4][74] = {
    { "Hi There" },
    { "what do ya want for nothing?" },
    {
        "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
        "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
        "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
        "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
        "\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD\xDD"
    },
    {
        "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
        "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
        "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
        "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
        "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
    },
};

static const int md5_hmac_test_buflen[4] = {
    8, 28, 50, 50
};

static const unsigned char md5_hmac_test_sum[4][16] = {
    {
        0x92, 0x94, 0x72, 0x7A, 0x36, 0x38, 0xBB, 0x1C,
        0x13, 0xF4, 0x8E, 0xF8, 0x15, 0x8B, 0xFC, 0x9D
    },
    {
        0x75, 0x0C, 0x78, 0x3E, 0x6A, 0xB0, 0xB5, 0x03,
        0xEA, 0xA8, 0x6E, 0x31, 0x0A, 0x5D, 0xB7, 0x38
    },
    {
        0x56, 0xBE, 0x34, 0x52, 0x1D, 0x14, 0x4C, 0x88,
        0xDB, 0xB8, 0xC7, 0x33, 0xF0, 0xE8, 0xB3, 0xF6
    },
    {
        0x69, 0x7E, 0xAF, 0x0A, 0xCA, 0x3A, 0x3A, 0xEA,
        0x3A, 0x75, 0x16, 0x47, 0x46, 0xFF, 0xAA, 0x79
    },

};
#endif  /* MD5_HMAC */
#endif  /* MD5_DIGEST */

int unittest_md5(const char *arg)
{
#if WITH_MD5_DIGEST
    unsigned char           md5sum[16];
    int                     i;

    LITE_printf("\r\n");
    for (i = 0; i < 7; i++) {
        LITE_printf("  MD5 test #%d: ", i + 1);

        LITE_md5_digest(md5_test_buf[i], md5_test_buflen[i], md5sum);
        if (memcmp(md5sum, md5_test_sum[i], 16) != 0) {
            LITE_printf("failed\r\n");
            return -1;
        }
        LITE_printf("passed\r\n");
    }
    LITE_printf("\r\n");

#if WITH_MD5_HMAC
    for (i = 0; i < 4; i++) {
        LITE_printf("  HMAC-MD5 test #%d: ", i + 1);

        LITE_hmac_md5(md5_hmac_test_key[i], md5_hmac_test_keylen[i], md5_hmac_test_buf[i], md5_hmac_test_buflen[i], md5sum);
        if (memcmp(md5sum, md5_hmac_test_sum[i], 16) != 0) {
            LITE_printf("failed\r\n");
            return -1;
        }
        LITE_printf("passed\r\n");
    }
    LITE_printf("\r\n");

#endif  /* WITH_MD5_HMAC */

    unsigned char           res[16];

    log_info("%12s : '%s'", "SOURCE-TEXT", arg);

    LITE_md5_digest((const unsigned char *)arg, strlen(arg), res);
    LITE_printf("[inf] %s(%d): %12s : '", __func__, __LINE__, "MD5-DIGEST");
    for (i = 0; i < sizeof(res); ++i) {
        LITE_printf("%02x", res[i]);
    }
    LITE_printf("'\r\n");

#if WITH_MD5_HMAC

    const unsigned char    *demo_key;

    demo_key = (const unsigned char *)
                    "MBEDTLS_MD5_HMAC_"
                    "demo_key_longer_than_64bytes_"
                    "to_make_test_coverage_better";

    LITE_hmac_md5(demo_key, strlen((char *)demo_key), (const unsigned char *)arg, strlen(arg), res);
    LITE_printf("[inf] %s(%d): %12s : '", __func__, __LINE__, "HMAC-MD5");
    for (i = 0; i < sizeof(res); ++i) {
        LITE_printf("%02x", res[i]);
    }
    LITE_printf("'\r\n");

#endif  /* WITH_MD5_HMAC */
    LITE_printf("\r\n");
#endif  /* WITH_MD5_DIGEST */

    return 0;
}

#endif  /* UT_FOR_LITE_CRYPTO */

int unittest_crypto(void)
{
#if UT_FOR_LITE_CRYPTO
    unittest_base64("Base64-Demo_Input");
    unittest_md5("MD5_Demo-Input/Data");
#endif
    return 0;
}


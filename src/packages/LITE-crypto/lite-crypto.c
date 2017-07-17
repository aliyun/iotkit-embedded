#include "lite-crypto_internal.h"

#if WITH_BASE64_ENCODE
int LITE_base64_encode(const unsigned char *from, const int from_len,
                       char *to, const int to_len)
{
    int         rc;
    size_t      dlen = to_len;
    size_t      olen = 0;

    rc = mbedtls_base64_encode((unsigned char *)to, dlen, &olen, from, from_len);

    if (rc < 0) {
        log_err("rc = %d", rc);
        return -1;
    }
    // log_debug("dlen = %d, olen = %d", (int)dlen, (int)olen);

    return rc;
}
#endif

#if WITH_BASE64_DECODE
int LITE_base64_decode(const char *from, const int from_len,
                       unsigned char *to, const int to_len)
{
    int         rc;
    size_t      dlen = to_len;
    size_t      olen = 0;

    rc = mbedtls_base64_decode((unsigned char *)to, dlen, &olen, (const unsigned char *)from, from_len);
    if (rc < 0) {
        log_err("rc = %d", rc);
        return -1;
    }
    // log_debug("dlen = %d, olen = %d", (int)dlen, (int)olen);

    return dlen;
}
#endif

#if WITH_MD5_DIGEST
int LITE_md5_digest(const unsigned char *in, const int in_len, unsigned char out[16])
{
    size_t      ilen = in_len;

    mbedtls_md5(in, ilen, out);
    return 0;
}
#endif

#if WITH_MD5_HMAC
int LITE_hmac_md5(const unsigned char *key, const int keylen,
                  const unsigned char *in_buffer, const int in_len,
                  unsigned char out_buffer[16])
{
    size_t      klen = keylen;
    size_t      ilen = in_len;

    mbedtls_md5_hmac(key, klen, in_buffer, ilen, out_buffer);
    return 0;
}
#endif


#ifndef __LITE_CRYPTO_H__
#define __LITE_CRYPTO_H__
#if defined(__cplusplus)
extern "C" {
#endif

#define LITE_BASE64_DEST_LEN(src)   (((src) + 2) / 3 * 4 + 1)

int     LITE_base64_encode(const unsigned char *from, const int from_len,
                           char *to, const int to_len);
int     LITE_base64_decode(const char *from, const int from_len,
                           unsigned char *to, const int to_len);
int     LITE_md5_digest(const unsigned char *in, const int in_len, unsigned char out[16]);
int     LITE_hmac_md5(const unsigned char *key, const int keylen,
                      const unsigned char *in_buffer, const int in_len,
                      unsigned char out_buffer[16]);

int unittest_crypto(void);

#if defined(__cplusplus)
}
#endif
#endif  /* __LITE_CRYPTO_H__ */


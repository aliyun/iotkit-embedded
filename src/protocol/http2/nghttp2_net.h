/*
 * nghttp2 - HTTP/2 C Library
 *
 * Copyright (c) 2012 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef NGHTTP2_NET_H
#define NGHTTP2_NET_H

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

#define IOT_BYTE_ORDER LITTLE_ENDIAN

#if IOT_BYTE_ORDER == BIG_ENDIAN
#define nghttp2_htonl(x) (x)
#define nghttp2_htons(x) (x)
#define nghttp2_ntohl(x) (x)
#define nghttp2_ntohs(x) (x)
#else
/* Windows requires ws2_32 library for ntonl family functions.  We
   define inline functions for those function so that we don't have
   dependeny on that lib. */

#ifdef _MSC_VER
#define STIN static __inline
#else
#define STIN static inline
#endif

STIN uint32_t nghttp2_htonl(uint32_t hostlong) {
  uint32_t res;
  unsigned char *p = (unsigned char *)&res;
  *p++ = hostlong >> 24;
  *p++ = (hostlong >> 16) & 0xffu;
  *p++ = (hostlong >> 8) & 0xffu;
  *p = hostlong & 0xffu;
  return res;
}

STIN uint16_t nghttp2_htons(uint16_t hostshort) {
  uint16_t res;
  unsigned char *p = (unsigned char *)&res;
  *p++ = hostshort >> 8;
  *p = hostshort & 0xffu;
  return res;
}

STIN uint32_t nghttp2_ntohl(uint32_t netlong) {
  uint32_t res;
  unsigned char *p = (unsigned char *)&netlong;
  res = *p++ << 24;
  res += *p++ << 16;
  res += *p++ << 8;
  res += *p;
  return res;
}

STIN uint16_t nghttp2_ntohs(uint16_t netshort) {
  uint16_t res;
  unsigned char *p = (unsigned char *)&netshort;
  res = *p++ << 8;
  res += *p;
  return res;
}
#endif
#endif /* NGHTTP2_NET_H */

/**
 * \file md5_hmac.c
 *
 * \brief MD5 message digest algorithm (hash function)
 *
 *  Copyright (C) 2006-2013, Brainspark B.V.
 *
 *  This file is part of PolarSSL (http://www.polarssl.org)
 *  Lead Maintainer: Paul Bakker <polarssl_maintainer at polarssl.org>
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/md5.h"
#include "mbedtls/md5_hmac.h"

/* Implementation that should never be optimized out by the compiler */
static void mbedtls_zeroize( void *v, size_t n ) {
    volatile unsigned char *p = v; while( n-- ) *p++ = 0;
}

static void _md5_hmac_init( md5_hmac_context *ctx )
{
    memset( ctx, 0, sizeof( md5_hmac_context ) );
}

static void _md5_hmac_free( md5_hmac_context *ctx )
{
    if( ctx == NULL )
        return;

    mbedtls_zeroize( ctx, sizeof( md5_hmac_context ) );
}

/*
 * MD5 HMAC context setup
 */
static void _md5_hmac_starts( md5_hmac_context *ctx, const unsigned char *key,
                      size_t keylen )
{
    size_t i;
    unsigned char sum[16];

    if( keylen > 64 )
    {
        mbedtls_md5( key, keylen, sum );
        keylen = 16;
        key = sum;
    }

    memset( ctx->ipad, 0x36, 64 );
    memset( ctx->opad, 0x5C, 64 );

    for( i = 0; i < keylen; i++ )
    {
        ctx->ipad[i] = (unsigned char)( ctx->ipad[i] ^ key[i] );
        ctx->opad[i] = (unsigned char)( ctx->opad[i] ^ key[i] );
    }

    mbedtls_md5_starts( &ctx->md5_ctx );
    mbedtls_md5_update( &ctx->md5_ctx, ctx->ipad, 64 );

    mbedtls_zeroize( sum, sizeof( sum ) );
}

/*
 * MD5 HMAC process buffer
 */
static void _md5_hmac_update( md5_hmac_context *ctx, const unsigned char *input,
                      size_t ilen )
{
    mbedtls_md5_update( &ctx->md5_ctx, input, ilen );
}

/*
 * MD5 HMAC final digest
 */
static void _md5_hmac_finish( md5_hmac_context *ctx, unsigned char output[16] )
{
    unsigned char tmpbuf[16];

    mbedtls_md5_finish( &ctx->md5_ctx, tmpbuf );
    mbedtls_md5_starts( &ctx->md5_ctx );
    mbedtls_md5_update( &ctx->md5_ctx, ctx->opad, 64 );
    mbedtls_md5_update( &ctx->md5_ctx, tmpbuf, 16 );
    mbedtls_md5_finish( &ctx->md5_ctx, output );

    mbedtls_zeroize( tmpbuf, sizeof( tmpbuf ) );
}

/*
 * output = HMAC-MD5( hmac key, input buffer )
 */
void mbedtls_md5_hmac( const unsigned char *key, size_t keylen,
               const unsigned char *input, size_t ilen,
               unsigned char output[16] )
{
    md5_hmac_context ctx;

    _md5_hmac_init( &ctx );
    _md5_hmac_starts( &ctx, key, keylen );
    _md5_hmac_update( &ctx, input, ilen );
    _md5_hmac_finish( &ctx, output );

    _md5_hmac_free( &ctx );
}

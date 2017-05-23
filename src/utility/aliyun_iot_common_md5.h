/*********************************************************************************
 * 文件名称: aliyun_iot_common_md5.h
 * 作       者:
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/

#ifndef ALIYUN_IOT_COMMON_MD5_H
#define ALIYUN_IOT_COMMON_MD5_H

#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_common_error.h"

typedef struct
{
    uint32_t total[2];          /*!< number of bytes processed  */
    uint32_t state[4];          /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */
}iot_md5_context;

/**
 * \brief          Initialize MD5 context
 *
 * \param ctx      MD5 context to be initialized
 */
void aliyun_iot_md5_init( iot_md5_context *ctx );

/**
 * \brief          Clear MD5 context
 *
 * \param ctx      MD5 context to be cleared
 */
void aliyun_iot_md5_free( iot_md5_context *ctx );

/**
 * \brief          Clone (the state of) an MD5 context
 *
 * \param dst      The destination context
 * \param src      The context to be cloned
 */
void aliyun_iot_md5_clone( iot_md5_context *dst,
                        const iot_md5_context *src );

/**
 * \brief          MD5 context setup
 *
 * \param ctx      context to be initialized
 */
void aliyun_iot_md5_starts( iot_md5_context *ctx );

/**
 * \brief          MD5 process buffer
 *
 * \param ctx      MD5 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void aliyun_iot_md5_update( iot_md5_context *ctx, const unsigned char *input, size_t ilen );

/**
 * \brief          MD5 final digest
 *
 * \param ctx      MD5 context
 * \param output   MD5 checksum result
 */
void aliyun_iot_md5_finish( iot_md5_context *ctx, unsigned char output[16] );

/* Internal use */
void aliyun_iot_md5_process( iot_md5_context *ctx, const unsigned char data[64] );

/**
 * \brief          Output = MD5( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   MD5 checksum result
 */
void aliyun_iot_md5( const unsigned char *input, size_t ilen, unsigned char output[16] );


int8_t aliyun_iot_common_hb2hex(uint8_t hb);

void aliyun_iot_common_md5(const int8_t *dat, int32_t datlen, int8_t *result);

#endif


/*********************************************************************************
 * 文件名称: aliyun_iot_common_base64.h
 * 作       者:
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/

#ifndef ALIYUN_IOT_COMMON_BASE64_H
#define ALIYUN_IOT_COMMON_BASE64_H

#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_common_error.h"

aliot_err_t aliyun_iot_common_base64encode(const uint8_t *data, uint32_t inputLength, uint32_t outputLenMax,uint8_t *encodedData,uint32_t *outputLength);
aliot_err_t aliyun_iot_common_base64decode(const uint8_t *data, uint32_t inputLength, uint32_t outputLenMax,uint8_t *decodedData,uint32_t *outputLength);

#endif

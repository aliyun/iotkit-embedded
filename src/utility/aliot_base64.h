

#ifndef _ALIOT_COMMON_BASE64_H_
#define _ALIOT_COMMON_BASE64_H_

#include "aliot_platform.h"
#include "aliot_error.h"

aliot_err_t aliot_base64encode(const uint8_t *data, uint32_t inputLength, uint32_t outputLenMax,
        uint8_t *encodedData, uint32_t *outputLength);
aliot_err_t aliot_base64decode(const uint8_t *data, uint32_t inputLength, uint32_t outputLenMax,
        uint8_t *decodedData, uint32_t *outputLength);

#endif

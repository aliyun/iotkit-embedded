#ifndef _IOTX_COMMON_BASE64_H_
#define _IOTX_COMMON_BASE64_H_

#include "iot_import.h"
#include "utils_error.h"

iotx_err_t utils_base64encode(const uint8_t *data, uint32_t inputLength, uint32_t outputLenMax,
                              uint8_t *encodedData, uint32_t *outputLength);
iotx_err_t utils_base64decode(const uint8_t *data, uint32_t inputLength, uint32_t outputLenMax,
                              uint8_t *decodedData, uint32_t *outputLength);

#endif

#ifndef _IOTX_TYPES_H_
#define _IOTX_TYPES_H_

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif
#ifndef int8_t
typedef signed char int8_t;
#endif
#ifndef uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef int16_t
typedef signed short int16_t;
#endif
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif
#ifndef int32_t
typedef signed int int32_t;
#endif
#ifndef uint64_t
typedef unsigned long long uint64_t;
#endif
#ifndef int64_t
typedef signed long long int64_t;
#endif

#ifndef _IN_
#define _IN_
#endif
#ifndef _OU_
#define _OU_
#endif

#define IOTX_PRODUCT_KEY_LEN     (20)
#define IOTX_DEVICE_NAME_LEN     (32)
#define IOTX_DEVICE_SECRET_LEN   (64)
#define IOTX_PRODUCT_SECRET_LEN  (64)
#define IOTX_PARTNER_ID_LEN      (64)
#define IOTX_MODULE_ID_LEN       (64)

#endif
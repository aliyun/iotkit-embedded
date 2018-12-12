#ifndef _INFRA_DEFS_H_
#define _INFRA_DEFS_H_

#define IOTX_SDK_VERSION     "2.3.0"

#define IOTX_PRODUCT_KEY_LEN     (20)
#define IOTX_DEVICE_NAME_LEN     (32)
#define IOTX_DEVICE_SECRET_LEN   (64)
#define IOTX_PRODUCT_SECRET_LEN  (64)
#define IOTX_PARTNER_ID_LEN      (64)
#define IOTX_MODULE_ID_LEN       (64)

typedef enum {
    IOTX_CLOUD_REGION_SHANGHAI,   /* Shanghai */
    IOTX_CLOUD_REGION_SINGAPORE,  /* Singapore */
    IOTX_CLOUD_REGION_JAPAN,      /* Japan */
    IOTX_CLOUD_REGION_USA_WEST,   /* America */
    IOTX_CLOUD_REGION_GERMANY,    /* Germany */
    IOTX_CLOUD_REGION_CUSTOM,     /* Custom setting */
    IOTX_CLOUD_DOMAIN_MAX         /* Maximum number of domain */
} iotx_cloud_region_types_t;

#endif
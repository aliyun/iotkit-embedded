CONFIG_ENV_CFLAGS   += \
    -Os -Wall \
    -D_PLATFORM_IS_HOST_ \
    -DUSING_UTILS_JSON \
    -DCM_VIA_CLOUD_CONN \
    -DCM_VIA_CLOUD_CONN_MQTT \
    -DCONFIG_DM_DEVTYPE_SINGLE

LDFLAGS             += -lpthread -lrt

CONFIG_ENV_CFLAGS   += \
    -Os -Wall \
    -D_PLATFORM_IS_HOST_ \
    -DCONFIG_DM_DEVTYPE_SINGLE

LDFLAGS             += -lpthread -lrt

LIBA_TARGET := libiot_hal.a
CFLAGS      := $(filter-out -ansi,$(CFLAGS))

#HDR_REFS    += external_libs src/infra

LIB_SRCS_PATTERN    += os/$(CONFIG_VENDOR)/*.c

ifneq (,$(filter -D_PLATFORM_IS_LINUX_,$(CFLAGS)))
ifneq (,$(filter -DSUPPORT_TLS,$(CFLAGS)))
LIB_SRCS_PATTERN    += tls/*.c
endif

ifeq (,$(filter -DAT_PARSER_ENABLED,$(CFLAGS)))
LIB_SRCS_EXCLUDE   := os/ubuntu/HAL_UART_linux.c
endif
endif

LIB_SRCS_PATTERN    += algo/*.c

ifneq (,$(filter -DAT_TCP_ENABLED, $(CFLAGS)))
ifneq (,$(filter -DAT_TCP_HAL_MK3060, $(CFLAGS)))
LIB_SRCS_PATTERN += atm/at_tcp/mk3060.c
endif

ifneq (,$(filter -DAT_TCP_HAL_SIM800, $(CFLAGS)))
LIB_SRCS_PATTERN += atm/at_tcp/sim800.c
endif
endif

ifneq (,$(filter -DAT_MQTT_ENABLED, $(CFLAGS)))
ifneq (,$(filter -DAT_MQTT_HAL_ICA, $(CFLAGS)))
LIB_SRCS_PATTERN += atm/at_mqtt/mqtt_ica.c
LDFLAGS         += -liot_sdk
endif
endif

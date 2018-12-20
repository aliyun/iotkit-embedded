LIBA_TARGET := libiot_hal.a
CFLAGS      := $(filter-out -ansi,$(CFLAGS))

#HDR_REFS    += external_libs src/infra

LIB_SRCS_PATTERN    += os/$(CONFIG_VENDOR)/*.c

ifneq (,$(filter -D_PLATFORM_IS_LINUX_,$(CFLAGS)))
ifneq (,$(filter -DSUPPORT_TLS,$(CFLAGS)))
LIB_SRCS_PATTERN    += tls/*.c
endif
endif

LIB_SRCS_PATTERN    += algo/*.c


ifneq (,$(filter -DMAL_ENABLED, $(CFLAGS)))
LIB_SRCS_PATTERN += mqtt/mal/mal.c

ifneq (,$(filter -DMAL_ICA_ENABLED, $(CFLAGS)))
LIB_SRCS_PATTERN += at/*.c
LIB_SRCS_PATTERN += mqtt/mal/ica/*.c  \
                    mqtt/mal/ica/test/*.c
endif
endif

ifneq (,$(filter -DSAL_ENABLED, $(CFLAGS)))
LIB_SRCS_PATTERN += sal/src/*.c
ifneq (,$(filter -DSAL_HAL_IMPL_ENABLED, $(CFLAGS)))
LIB_SRCS_PATTERN += at/*.c
LIB_SRCS_PATTERN += sal/hal-impl/*.c
endif
endif

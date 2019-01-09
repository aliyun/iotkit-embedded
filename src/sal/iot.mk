LIBA_TARGET := libiot_sal.a

HDR_REFS        := src/infra

ifneq (,$(filter -DSAL_ENABLED, $(CFLAGS)))
LIB_SRCS_PATTERN += *.c

ifneq (,$(filter -DSAL_HAL_IMPL_MK3060, $(CFLAGS)))
LIB_SRCS_PATTERN += hal-impl/mk3060.c
endif

ifneq (,$(filter -DSAL_HAL_IMPL_SIM800, $(CFLAGS)))
LIB_SRCS_PATTERN += hal-impl/sim800.c
endif
endif

LIBA_TARGET := libiot_tls.a
CFLAGS      := $(filter-out -ansi,$(CFLAGS))
HDR_REFS    += src/infra/utils
HDR_REFS    += src/infra/log

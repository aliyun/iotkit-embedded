TARGET      := sdk-testsuites
HDR_REFS    := src build-rules
SRCS        := $(wildcard $(TOP_DIR)/$(MODULE_NAME)/*.c)
SRCS        += $(TOP_DIR)/build-rules/misc/cut.c
CFLAGS      := $(filter-out -ansi,$(CFLAGS))
LDFLAGS     := -Bstatic
LDFLAGS     += -liot_sdk
LDFLAGS     += -liot_hal

ifneq (,$(filter -DSUPPORT_ITLS,$(CFLAGS)))
LDFLAGS     += -litls -liot_tfs -liot_sdk
endif

ifeq (,$(filter -DIOTX_WITHOUT_TLS,$(CFLAGS)))
LDFLAGS     += -liot_tls
endif

DEPENDS     += src/ref-impl/hal

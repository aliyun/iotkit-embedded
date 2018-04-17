TARGET      := sdk-testsuites
HDR_REFS    := src build-rules
SRCS        := $(wildcard $(TOP_DIR)/$(MODULE_NAME)/*.c)
SRCS        += $(TOP_DIR)/build-rules/misc/cut.c
CFLAGS      := $(filter-out -ansi,$(CFLAGS))


LDFLAGS     += -liot_platform

ifneq (,$(filter -DMQTT_ID2_AUTH,$(CFLAGS)))
LDFLAGS     += -ltfs
endif

LDFLAGS     += -Bstatic -liot_tls
LDFLAGS     += -liot_sdk
TARGET      := sdk-testsuites
HDR_REFS    := src
EXTRA_SRCS  := $(RULE_DIR)/misc/cut.[ch]
CFLAGS      := $(filter-out -ansi,$(CFLAGS))

LDFLAGS     += -liot_sdk
LDFLAGS     += -liot_platform

ifneq (,$(filter -DMQTT_ID2_AUTH,$(CFLAGS)))
LDFLAGS     += -ltfs
endif

LDFLAGS     += -Bstatic -liot_tls

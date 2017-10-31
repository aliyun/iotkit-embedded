TARGET      := sdk-testsuites
HDR_REFS    := src

LDFLAGS     += -liot_sdk
LDFLAGS     += -liot_platform

ifneq (,$(filter -DMQTT_ID2_AUTH,$(CFLAGS)))
LDFLAGS     += -ltfs
endif

LDFLAGS     += -Bstatic -liot_tls

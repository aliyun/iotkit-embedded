TARGET      := sdk-testsuites
HDR_REFS    := src

LDFLAGS     += -liot_sdk -liot_platform

ifneq (,$(filter -DMQTT_ID2_AUTH,$(CFLAGS)))
LDFLAGS     += -ltfs -lmbedcrypto
endif

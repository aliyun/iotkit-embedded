TARGET      := sdk-testsuites
DEPENDS     += src/log

LDFLAGS     += -liot_sdk

ifeq (y,$(strip $(FEATURE_ID2_AUTH)))
LDFLAGS     += -ltfs -lcrypto
endif

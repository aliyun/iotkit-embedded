TARGET      := sdk-testsuites
DEPENDS     += src/log

LDFLAGS     += -liot_sdk

ifeq (y,$(strip $(FEATURE_EQUIP_ID2)))
LDFLAGS     += -ltfs -lcrypto
endif

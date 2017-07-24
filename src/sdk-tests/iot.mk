TARGET      := sdk-testsuites
DEPENDS     += src/log

LDFLAGS     += -liot_sdk -ltfs -lcrypto

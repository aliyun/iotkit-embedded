
ifeq (y,$(strip $(FEATURE_HTTP2_COMM_ENABLED)))
    CFLAGS := $(filter-out -DFORCE_SSL_VERIFY,$(CFLAGS))
endif

ifeq (Darwin,$(shell uname))
    CFLAGS := $(filter-out -DOTA_ENABLED,$(CFLAGS))
    CFLAGS := $(filter-out -DWIFI_PROVISION_ENABLED,$(CFLAGS))
    CFLAGS := $(filter-out -DDEV_BIND_ENABLED,$(CFLAGS))
endif

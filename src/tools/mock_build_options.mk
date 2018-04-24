SKIP_SSL_VERIFY_MODES = \
    ID2_DAILY \
    ID2_PRE \
    MQTT_DAILY \
    OTA_DAILY \

SKIP_MQTT_DIRECT_MODES = \
    MQTT_DAILY \
    OTA_DAILY \

ifneq (,$(filter $(foreach M,$(SKIP_SSL_VERIFY_MODES),-DTEST_$(M)),$(CFLAGS)))
CFLAGS  := $(filter-out -DFORCE_SSL_VERIFY,$(CFLAGS))
endif

ifneq (,$(filter $(foreach M,$(SKIP_MQTT_DIRECT_MODES),-DTEST_$(M)),$(CFLAGS)))
CFLAGS  := $(filter-out -DMQTT_DIRECT,$(CFLAGS))
endif

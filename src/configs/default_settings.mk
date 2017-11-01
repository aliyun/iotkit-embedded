COVERAGE_CMD            := src/scripts/walk_through_examples.sh
DEFAULT_BLD             := src/configs/config.ubuntu.x86
POST_FINAL_OUT_HOOK     := Post_Distro
SUBDIRS                 := directory-not-exist-actually

FEATURE_MQTT_SHADOW         ?= $(FEATURE_MQTT_COMM_ENABLED)
FEATURE_COAP_DTLS_SUPPORT   ?= $(FEATURE_COAP_COMM_ENABLED)
FEATURE_MQTT_ID2_AUTH       ?= n
FEATURE_MQTT_ID2_CRYPTO     ?= n
FEATURE_OTA_ENABLED         ?= y
FEATURE_OTA_FETCH_CHANNEL   ?= HTTP
FEATURE_OTA_SIGNAL_CHANNEL  ?= MQTT
FEATURE_MQTT_ID2_ENV        ?= online

CONFIG_LIB_EXPORT           ?= static

CFLAGS  += -DFORCE_SSL_VERIFY

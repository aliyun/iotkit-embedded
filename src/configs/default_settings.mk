COVERAGE_CMD            := src/scripts/walk_through_examples.sh
DEFAULT_BLD             := src/configs/config.ubuntu.x86
POST_FINAL_OUT_HOOK     := Post_Distro

SUBDIRS                 := directory-not-exist-actually

PLATFORM_CC             ?= gcc
PLATFORM_AR             ?= ar
PLATFORM_OS             ?= linux

FEATURE_MQTT_SHADOW         ?= $(FEATURE_MQTT_COMM_ENABLED)
FEATURE_COAP_DTLS_SUPPORT   ?= $(FEATURE_COAP_COMM_ENABLED)
FEATURE_MQTT_ID2_AUTH       ?= n
FEATURE_MQTT_ID2_CRYPTO     ?= n
FEATURE_OTA_FETCH_CHANNEL   ?= HTTP
FEATURE_HTTP_COMM_ENABLED   ?= y

FEATURE_MQTT_ID2_ENV        ?= online

CFLAGS  += -DFORCE_SSL_VERIFY

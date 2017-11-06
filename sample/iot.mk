DEPENDS             := src/platform
LDFLAGS             := -liot_sdk
LDFLAGS             += -liot_platform
LDFLAGS             += -Bstatic -liot_tls
CFLAGS              := $(filter-out -ansi,$(CFLAGS))

ifneq (,$(filter -D_PLATFORM_IS_WINDOWS_,$(CFLAGS)))
LDFLAGS             += -lws2_32
CFLAGS              := $(filter-out -DCOAP_COMM_ENABLED,$(CFLAGS))
endif

ifneq (,$(filter -DMQTT_COMM_ENABLED,$(CFLAGS)))
TARGET              += mqtt-example mqtt_rrpc-example
SRCS_mqtt-example   := mqtt/mqtt-example.c
SRCS_mqtt_rrpc-example := mqtt/mqtt_rrpc-example.c

    ifneq (,$(filter -DOTA_ENABLED,$(CFLAGS)))
    ifneq (,$(filter -DOTA_SIGNAL_CHANNEL=1,$(CFLAGS)))
    TARGET                += ota_mqtt-example
    SRCS_ota_mqtt-example := ota/ota_mqtt-example.c
    endif
    endif

    ifneq (,$(filter -DMQTT_SHADOW,$(CFLAGS)))
    TARGET              += shadow-example
    SRCS_shadow-example := device-shadow/shadow-example.c
    endif

    ifneq (,$(filter -DMQTT_ID2_AUTH,$(CFLAGS)))
    ifneq (,$(filter -DMQTT_ID2_ENV=daily,$(CFLAGS)))
    LDFLAGS     += -ltfs
    else
    LDFLAGS     += -ltfs_online
    endif
    LDFLAGS     += -liot_tls
    endif

endif

ifneq (,$(filter -DCOAP_COMM_ENABLED,$(CFLAGS)))
TARGET              += coap-example

SRCS_coap-example   := coap/coap-example.c
SRCS                += coap/coap-example.c

    ifneq (,$(filter -DOTA_ENABLED,$(CFLAGS)))
    ifneq (,$(filter -DOTA_SIGNAL_CHANNEL=2,$(CFLAGS)))
    TARGET                += ota_coap-example
    SRCS_ota_coap-example := ota/ota_coap-example.c
    endif
    endif
endif

ifneq (,$(filter -DHTTP_COMM_ENABLED,$(CFLAGS)))
TARGET              += http-example
SRCS_http-example   := http/http-example.c
endif

DEPENDS             := src/platform
HDR_REFS            += src/sdk-impl src/utils src/log src/packages/LITE-utils
LDFLAGS             := -liot_sdk
LDFLAGS             += -liot_platform
ifneq (,$(filter -DIOTX_WITHOUT_ITLS,$(CFLAGS)))
LDFLAGS             += -Bstatic -liot_tls
endif 
CFLAGS              := $(filter-out -ansi,$(CFLAGS))

ifneq (,$(filter -D_PLATFORM_IS_WINDOWS_,$(CFLAGS)))
LDFLAGS             += -lws2_32
CFLAGS              := $(filter-out -DCOAP_COMM_ENABLED,$(CFLAGS))
endif

ifneq (,$(filter -DMQTT_COMM_ENABLED,$(CFLAGS)))
TARGET              += mqtt-example mqtt_rrpc-example mqtt_multi_thread-example
SRCS_mqtt-example   := mqtt/mqtt-example.c
SRCS_mqtt_rrpc-example := mqtt/mqtt_rrpc-example.c
SRCS_mqtt_multi_thread-example := mqtt/mqtt_multi_thread-example.c

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
    ifneq (,$(filter -DON_DAILY,$(CFLAGS)))
    LDFLAGS     += -ltfs
    else
    LDFLAGS     += -ltfs_online
    endif
    ifeq (,$(filter -DIOTX_WITHOUT_ITLS,$(CFLAGS)))
    LDFLAGS     += -litls
    endif
    ifeq (,$(filter -DIOTX_WITHOUT_TLS,$(CFLAGS)))
    LDFLAGS     += -liot_tls
    endif
    endif
    LDFLAGS     += -liot_sdk

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

ifneq (,$(filter -DSUBDEVICE_ENABLED,$(CFLAGS)))
TARGET                += subdev-example
SRCS_subdev-example   += subdev/subdev-example.c \
                         subdev/subdev_example_api.c
endif

ifneq (,$(filter -DCLOUD_CONN_ENABLED,$(CFLAGS)))   
TARGET                    += cloud_conn-example 
SRCS_cloud_conn-example   := cloud-conn/cloud_conn-example.c
endif

ifneq (,$(filter -DCMP_ENABLED,$(CFLAGS)))   
TARGET                    += cmp-example 
SRCS_cmp-example          := cmp/cmp-example.c
endif


ifneq (,$(filter -DDM_ENABLED,$(CFLAGS)))   
TARGET                    += linkkit-example 
SRCS_linkkit-example      := linkkit/src/linkkit_export.c \
                             linkkit/src/lite_queue.c \
                             linkkit/samples/linkkit_sample.c
endif


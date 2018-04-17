include $(CURDIR)/src/scripts/internal_make_funcs.mk

SWITCH_VARS := \
    FEATURE_MQTT_COMM_ENABLED \
    FEATURE_MQTT_SHADOW \
    FEATURE_MQTT_DIRECT \
    FEATURE_MQTT_DIRECT_NOTLS \
    FEATURE_COAP_COMM_ENABLED \
    FEATURE_COAP_DTLS_SUPPORT \
    FEATURE_OTA_ENABLED \
    FEATURE_OTA_SIGNAL_CHANNEL \
    FEATURE_OTA_FETCH_CHANNEL \
    FEATURE_MQTT_ID2_AUTH \
    FEATURE_MQTT_ID2_CRYPTO \
    FEATURE_MQTT_ID2_ENV \
    FEATURE_HTTP_COMM_ENABLED \
    FEATURE_SUBDEVICE_ENABLED \
    FEATURE_CLOUD_CONN_ENABLED \
    FEATURE_CMP_ENABLED \
    FEATURE_DM_ENABLED \
    FEATURE_SERVICE_OTA_ENABLED \
    FEATURE_SERVICE_COTA_ENABLED \
    FEATURE_SUPPORT_PRODUCT_SECRET \
    FEATURE_MQTT_DIRECT_NOITLS \

$(foreach v, \
    $(SWITCH_VARS), \
    $(if $(filter y,$($(v))), \
        $(eval CFLAGS += -D$(subst FEATURE_,,$(v)))) \
)

ifeq (y,$(strip $(FEATURE_OTA_ENABLED)))
ifneq (n,$(strip $(FEATURE_MQTT_DIRECT_NOTLS)))
# $(error FEATURE_OTA_ENABLED = y requires FEATURE_MQTT_DIRECT_NOTLS = n!)
endif
ifeq (MQTT,$(strip $(FEATURE_OTA_SIGNAL_CHANNEL)))
ifneq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))
$(error FEATURE_OTA_SIGNAL_CHANNEL = MQTT requires FEATURE_MQTT_COMM_ENABLED = y!)
endif
CFLAGS += -DOTA_SIGNAL_CHANNEL=1
else
ifeq (COAP,$(strip $(FEATURE_OTA_SIGNAL_CHANNEL)))
ifneq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))
$(error FEATURE_OTA_SIGNAL_CHANNEL = COAP requires FEATURE_COAP_COMM_ENABLED = y!)
endif
CFLAGS += -DOTA_SIGNAL_CHANNEL=2
else
ifeq (HTTP,$(strip $(FEATURE_OTA_SIGNAL_CHANNEL)))
CFLAGS += -DOTA_SIGNAL_CHANNEL=4
else
$(error FEATURE_OTA_SIGNAL_CHANNEL must be MQTT or COAP or HTTP!)
endif # HTTP
endif # COAP
endif # MQTT
endif # OTA Enabled

ifeq (y,$(strip $(FEATURE_SUBDEVICE_ENABLED)))
ifneq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))
$(error FEATURE_SUBDEVICE_ENABLED = y requires FEATURE_MQTT_COMM_ENABLED = y!)
endif
CFLAGS += -DSUBDEVICE_SUPPORT
ifeq (gateway,$(strip $(FEATURE_SUBDEVICE_STATUS)))
CFLAGS += -DGATEWAY_SUPPORT
endif # FEATURE_SUBDEVICE_STATUS
endif # FEATURE_SUBDEVICE_ENABLED

ifeq (y,$(strip $(FEATURE_CLOUD_CONN_ENABLED)))
ifneq (n,$(strip $(FEATURE_MQTT_DIRECT_NOTLS)))
$(error FEATURE_CLOUD_CONN_ENABLED = y requires FEATURE_MQTT_DIRECT_NOTLS = n!)
endif # FEATURE_MQTT_DIRECT_NOTLS
CFLAGS += -DCLOUD_CONN_SUPPORT
endif # FEATURE_CLOUD_CONN_ENABLED

ifeq (CLOUD_CONN,$(strip $(FEATURE_SUBDEVICE_CHANNEL)))
ifneq (y,$(strip $(FEATURE_CLOUD_CONN_ENABLED)))
$(error FEATURE_SUBDEVICE_CHANNEL = CLOUD_CONN requires FEATURE_CLOUD_CONN_ENABLED = y!)
endif
CFLAGS += -DSUBDEV_VIA_CLOUD_CONN
endif # FEATURE_SUBDEVICE_CHANNEL

ifeq (MQTT,$(strip $(FEATURE_SUBDEVICE_CHANNEL)))
ifneq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))
$(error FEATURE_SUBDEVICE_CHANNEL = MQTT requires FEATURE_MQTT_COMM_ENABLED = y!)
endif
CFLAGS += -DSUBDEV_VIA_MQTT
endif # FEATURE_SUBDEVICE_CHANNEL

ifeq (y,$(strip $(FEATURE_CMP_ENABLED)))
ifneq (y,$(strip $(FEATURE_OTA_ENABLED)))
$(error FEATURE_CMP_ENABLED = y requires FEATURE_OTA_ENABLED = y!)
endif
ifeq (y,$(strip $(FEATURE_CMP_VIA_MQTT_DIRECT)))
ifneq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))
$(error FEATURE_CMP_VIA_MQTT_DIRECT = y requires FEATURE_MQTT_COMM_ENABLED = y!)
endif
CFLAGS += -DCMP_VIA_MQTT_DIRECT
endif #FEATURE_CMP_VIA_MQTT_DIRECT
ifeq (n,$(strip $(FEATURE_CMP_VIA_MQTT_DIRECT)))
ifneq (y,$(strip $(FEATURE_CLOUD_CONN_ENABLED)))
$(error FEATURE_CMP_VIA_MQTT_DIRECT = n requires FEATURE_CLOUD_CONN_ENABLED = y!)
endif #FEATURE_CLOUD_CONN_ENABLED
CFLAGS += -DCMP_VIA_CLOUN_CONN
ifeq (MQTT,$(strip $(FEATURE_CMP_VIA_CLOUD_CONN)))
ifneq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))
$(error FEATURE_CMP_VIA_CLOUD_CONN = MQTT requires FEATURE_MQTT_COMM_ENABLED = y!)
endif #FEATURE_MQTT_COMM_ENABLED
CFLAGS += -DCMP_VIA_CLOUD_CONN_MQTT
endif #FEATURE_CMP_VIA_CLOUD_CONN
ifeq (COAP,$(strip $(FEATURE_CMP_VIA_CLOUD_CONN)))
ifneq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))
$(error FEATURE_CMP_VIA_CLOUD_CONN = COAP requires FEATURE_COAP_COMM_ENABLED = y!)
endif #FEATURE_COAP_COMM_ENABLED
CFLAGS += -DCMP_VIA_CLOUD_CONN_COAP
endif #FEATURE_CMP_VIA_CLOUD_CONN
ifeq (HTTP,$(strip $(FEATURE_CMP_VIA_CLOUD_CONN)))
ifneq (y,$(strip $(FEATURE_HTTP_COMM_ENABLED)))
$(error FEATURE_CMP_VIA_CLOUD_CONN = HTTP requires FEATURE_HTTP_COMM_ENABLED = y!)
endif #FEATURE_HTTP_COMM_ENABLED
CFLAGS += -DCMP_VIA_CLOUD_CONN_HTTP
endif #FEATURE_CMP_VIA_CLOUD_CONN
endif #FEATURE_CMP_VIA_MQTT_DIRECT = n
endif #FEATURE_CMP_ENABLED

include build-rules/settings.mk
sinclude $(CONFIG_TPL)

ifeq (,$(filter reconfig distclean,$(MAKECMDGOALS)))
ifneq (HTTP,$(strip $(FEATURE_OTA_FETCH_CHANNEL)))
$(error FEATURE_OTA_FETCH_CHANNEL must be HTTP!)
endif

ifneq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))

    ifneq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))
    $(error Either CoAP or MQTT required to be y!)
    endif

$(foreach V,SHADOW DIRECT DIRECT_NOTLS, \
    $(if $(filter y,$(strip $(FEATURE_MQTT_$(V)))), \
        $(error FEATURE_MQTT_$(V) = y requires FEATURE_MQTT_COMM_ENABLED = y!) \
    ) \
)
endif

ifeq (y,$(strip $(FEATURE_MQTT_DIRECT)))

    ifneq (y,$(strip $(FEATURE_MQTT_DIRECT_NOTLS)))
        ifneq (y,$(strip $(FEATURE_MQTT_DIRECT_NOITLS)))
            $(error FEATURE_MQTT_DIRECT_NOITLS or FEATURE_MQTT_DIRECT_NOTLS must be selected one or more)
        endif
    endif
    ifeq (y,$(strip $(FEATURE_MQTT_DIRECT_NOTLS)))
    CFLAGS  += -DIOTX_WITHOUT_TLS
    endif
    ifeq (y,$(strip $(FEATURE_MQTT_DIRECT_NOITLS)))
    CFLAGS  += -DIOTX_WITHOUT_ITLS
    endif   
    ifeq (y,$(strip $(FEATURE_MQTT_ID2_CRYPTO)))
    $(error FEATURE_MQTT_ID2_CRYPTO + FEATURE_MQTT_DIRECT is not supported!)
    endif

else    # ifeq (y,$(strip $(FEATURE_MQTT_DIRECT)))

    ifeq (y,$(strip $(FEATURE_MQTT_DIRECT_NOTLS)))
    $(error FEATURE_MQTT_DIRECT_NOTLS = y requires FEATURE_MQTT_DIRECT = y!)
    endif

endif   # ifeq (y,$(strip $(FEATURE_MQTT_DIRECT)))

ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))
    ifneq (y,$(strip $(FEATURE_MQTT_DIRECT_NOTLS)))
        ifneq (y,$(strip $(FEATURE_MQTT_DIRECT_NOITLS)))
            $(error FEATURE_MQTT_DIRECT_NOITLS or FEATURE_MQTT_DIRECT_NOTLS must be selected one or more)
        endif
    endif
    ifeq (y,$(strip $(FEATURE_MQTT_DIRECT_NOITLS)))
    $(error FEATURE_MQTT_ID2_AUTH requires FEATURE_MQTT_DIRECT_NOITLS = n!)
    endif
    
    ifeq (y,$(strip $(FEATURE_COAP_DTLS_SUPPORT)))
    $(error FEATURE_COAP_DTLS_SUPPORT = y requires FEATURE_MQTT_ID2_AUTH = n!)
    endif
    # ifneq (gcc,$(strip $(CC)))
    # $(error FEATURE_MQTT_ID2_AUTH requires $(CC) equal gcc!)
    # endif

else    # ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))
    ifeq (n,$(strip $(FEATURE_MQTT_DIRECT_NOITLS)))
    $(error FEATURE_MQTT_ID2_AUTH = n requires FEATURE_MQTT_DIRECT_NOITLS = y!)
    endif
        
    ifeq (y,$(strip $(FEATURE_MQTT_ID2_CRYPTO)))
    $(error FEATURE_MQTT_ID2_CRYPTO = y requires FEATURE_MQTT_ID2_AUTH = y!)
    endif

endif   # ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))

ifeq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))
else    # ifeq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))

    ifeq (y,$(strip $(FEATURE_COAP_DTLS_SUPPORT)))
    $(error FEATURE_COAP_DTLS_SUPPORT = y requires FEATURE_COAP_COMM_ENABLED = y!)
    endif

endif   # ifeq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))

ifeq (pre,$(strip $(FEATURE_MQTT_ID2_ENV)))
CFLAGS  += -DTEST_ID2_PRE
else
ifeq (daily,$(strip $(FEATURE_MQTT_ID2_ENV)))
CFLAGS  += -DTEST_ID2_DAILY
endif
endif
endif

ifeq (y,$(strip $(FEATURE_DM_ENABLED)))    
    ifneq (y,$(strip $(FEATURE_CMP_ENABLED)))
    $(error FEATURE_DM_ENABLED = y requires FEATURE_CMP_ENABLED = y!)
    endif
endif

ifeq (y,$(strip $(FEATURE_SERVICE_OTA_ENABLED)))    
    ifneq (y,$(strip $(FEATURE_CMP_ENABLED)))
    $(error FEATURE_SERVICE_OTA_ENABLED = y requires FEATURE_CMP_ENABLED = y!)
    endif
    CFLAGS  += -DSERVICE_OTA_ENABLED
    
    ifeq (y,$(strip $(FEATURE_SERVICE_COTA_ENABLED)))    
    ifneq (y,$(strip $(FEATURE_SERVICE_OTA_ENABLED)))
    $(error FEATURE_SERVICE_COTA_ENABLED = y requires FEATURE_SERVICE_OTA_ENABLED = y!)
    endif
    CFLAGS  += -DSERVICE_COTA_ENABLED
endif
endif

ifeq (y,$(strip $(FEATURE_SUPPORT_PRODUCT_SECRET)))
    CFLAGS  += -DSUPPORT_PRODUCT_SECRET
endif

SUBDIRS += src/tls
SUBDIRS += src/platform
SUBDIRS += sample
SUBDIRS += src/sdk-tests


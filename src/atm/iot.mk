LIBA_TARGET := libiot_at.a

HDR_REFS        := src/infra

ifneq (,$(filter -DATM_ENABLED, $(CFLAGS)))
LIB_SRCS_PATTERN += at_api.c    

ifneq (,$(filter -DAT_TCP_ENABLED, $(CFLAGS)))
LIB_SRCS_PATTERN += at_conn_mbox.c at_conn_mgmt.c

ifneq (,$(filter -DAT_SSL_ENABLED, $(CFLAGS)))
LIB_SRCS_PATTERN += at_ssl.c
else
LIB_SRCS_PATTERN += at_tcp.c
endif
endif

ifneq (,$(filter -DAT_MQTT_ENABLED, $(CFLAGS)))
LIB_SRCS_PATTERN += at_mqtt.c
endif

ifneq (,$(filter -DAT_PARSER_ENABLED, $(CFLAGS)))
LIB_SRCS_PATTERN += at_parser.c
endif
endif

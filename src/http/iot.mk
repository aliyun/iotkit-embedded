LIBA_TARGET     := libiot_http.a

HDR_REFS        += src/infra

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

LIB_SRCS_EXCLUDE        := examples/http_example.c
SRCS_http-example   	+= examples/http_example.c

$(call Append_Conditional, TARGET, http-example, COMM_HTTP_ENABLED, BUILD_AOS NO_EXECUTABLES)

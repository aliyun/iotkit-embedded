LIBA_TARGET     := libiot_dynreg.a

HDR_REFS        := src/infra

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

LIB_SRCS_EXCLUDE      := examples/dynreg_example.c
SRCS_dynreg-example   += examples/dynreg_example.c

$(call Append_Conditional, TARGET, dynreg-example, DYNAMIC_REGISTER, BUILD_AOS NO_EXECUTABLES)


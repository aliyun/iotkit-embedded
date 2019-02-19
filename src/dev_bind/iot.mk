LIBA_TARGET         := libiot_dev_bind.a
LIB_SRCS_PATTERN    := *.c
$(call Append_Conditional, LIB_SRCS_PATTERN, dev_bind_impl/*.c, DEV_BIND_ENABLED)


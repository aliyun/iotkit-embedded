#CFLAGS              := $(filter-out -Werror,$(CFLAGS))

TARGET              := linkkit-sample
SRCS_linkkit-sample := linkkit_sample.c

LDFLAGS             += \
    -lilop-sdk \
    -lilop-hal \
    -lilop-tls \

HDR_REFS            += layers/dm/include service/fota sdk-encap layers/linkkit/include

DEPENDS             += \
    service/fota \
    layers/dm \
    layers/linkkit


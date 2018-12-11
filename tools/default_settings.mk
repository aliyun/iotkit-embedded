DEFAULT_BLD             ?= $(CONFIG_DIR)/config.ubuntu.x86
POST_ENV_HOOK           := Post_Env
POST_FINAL_OUT_HOOK     := Post_Distro
SUBDIRS                 := directory-not-exist-actually

ifeq (Darwin,$(shell uname))
POST_FINAL_OUT_HOOK     :=
endif

CONFIG_LIB_EXPORT       ?= static

# Default CFLAGS setting
#
CFLAGS  += -DFORCE_SSL_VERIFY
CFLAGS  += -I$(TOP_DIR)/include
CFLAGS  += -I$(TOP_DIR)/include/imports
CFLAGS  += -I$(TOP_DIR)/include/exports

# For one makefile Generation
#
EXTRA_INSTALL_HDRS  := \
    $(EXPORT_DIR)/iot_import.h \
    $(EXPORT_DIR)/iot_export.h \
    $(EXPORT_DIR)/imports \
    $(EXPORT_DIR)/exports \

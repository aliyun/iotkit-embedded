include $(RULE_DIR)/funcs.mk

TOPDIR_NAME     := $(shell $(SHELL_DBG) basename $(TOP_DIR)|grep -m 1 -o \[-_a-zA-Z\]*[a-zA-Z])
LIBOBJ_TMPDIR   := $(OUTPUT_DIR)/lib$(TOPDIR_NAME).objs

SYSROOT_BIN     := $(OUTPUT_DIR)${bindir}
SYSROOT_INC     := $(OUTPUT_DIR)${incdir}
SYSROOT_LIB     := $(OUTPUT_DIR)${libdir}
CONFIG_VENDOR   := $(shell grep -m 1 "VENDOR *:" $(CONFIG_TPL) 2>/dev/null|awk '{ print $$NF }')
IMPORT_VDRDIR   := $(IMPORT_DIR)/$(CONFIG_VENDOR)

HOST_ARCH_BITS  := $(shell [ "$$(uname -m)" = "x86_64" ] && echo -n "64" || echo -n "32")
PREBUILT_LIBDIR := $(if $(strip $(filter 64,$(HOST_ARCH_BITS)) $(CROSS_PREFIX)),libs,32bit-libs)
PREBUILT_BINDIR := $(if $(strip $(filter 64,$(HOST_ARCH_BITS)) $(CROSS_PREFIX)),bin,32bit-bin)

ifeq ($(shell uname), Linux)
RPATH_CFLAGS    += -Wl,-rpath='$$''ORIGIN/../lib'
endif

# CFLAGS contains '$' causes ./configure failure in ORIGIN packages
#
# Instead of:
#
# ifeq ($(shell uname), Linux)
# CFLAGS += -Wl,-rpath='$$''ORIGIN/../lib'
# CFLAGS := $(sort $(CFLAGS))
# endif
#
# Write @ package's makefile:
#
# ./configure \
#     CFLAGS='$(CFLAGS) -Wl,-rpath='\''$$$$'\''ORIGIN/../lib' \
#     --prefix=...
#
# To pass in RPATH related link flags

COMP_LIB_NAME   := $(subst lib,,$(subst .so,,$(subst .a,,$(COMP_LIB))))
RECURSIVE_MAKE  := $(MAKE) -s -C $(TOP_DIR) -f $(TOP_MAKEFILE)
ALL_SUB_DIRS    := $(shell find -L $(TOP_DIR) ! -path "$(OUTPUT_DIR)/*" -name "$(MAKE_SEGMENT)" \
                            | sed 's:$(TOP_DIR)[/]*::;s:[/]*$(MAKE_SEGMENT)::')

SHOW_ENV_VARS   := \
    MODULE_NAME PKG_NAME PKG_RPATH PKG_SOURCE PKG_SWITCH_V PKG_SWITCH \
    HOST_ARCH_BITS PREBUILT_LIBDIR RPATH_CFLAGS \
    CROSS_PREFIX DEPENDS CFLAGS CCLD LDFLAGS \
    CC LD AR STRIP OBJCOPY COMP_LIB_COMPONENTS \
    MAKE_ENV_VARS MAKE_FN_VARS BUILD_CONFIG \
    LIBA_TARGET LIBSO_TARGET TARGET KMOD_TARGET \
    SRCS OBJS LIB_SRCS LIB_OBJS LIBHDR_DIR LIB_HEADERS \
    INTERNAL_INCLUDES IMPORT_DIR EXTERNAL_INCLUDES \
    CONFIG_LIB_EXPORT OBJCOPY_FLAGS \

ifndef CONFIG_LIB_EXPORT
ifeq (y,$(strip $(CONFIG_EMB_GATEWAY_SDK)))
CONFIG_LIB_EXPORT := dynamic
else
ifeq (y,$(strip $(CONFIG_EMB_BASIC_SDK)))
CONFIG_LIB_EXPORT := static
else
ifeq (y,$(strip $(CONFIG_EMB_ROUTER_SDK)))
CONFIG_LIB_EXPORT := dynamic
endif # ROUTER
endif # BASIC
endif # GATEWAY
else
ifneq (static,$(CONFIG_LIB_EXPORT))
ifneq (dynamic,$(CONFIG_LIB_EXPORT))
$(error CONFIG_LIB_EXPORT($(CONFIG_LIB_EXPORT)) is neither 'static' or 'dynamic')
endif # dynamic
endif # static
endif # CONFIG_LIB_EXPORT

ifneq (,$(shell ls $(STAMP_BLD_VAR) 2>/dev/null))
ifeq (,$(filter reconfig distclean,$(MAKECMDGOALS)))
ifeq (0,$(shell sed '/[-_/a-zA-Z0-9]* = ..*/d' $(STAMP_BLD_VAR)|wc -l))
include $(STAMP_BLD_VAR)
endif
endif
endif

.PHONY: all clean

ifdef SUBDIRS
include $(RULE_DIR)/_rules-top.mk
include $(RULE_DIR)/_rules-prefix.mk

CROSS_CANDIDATES := CC CXX AR LD STRIP OBJCOPY
export CC := $(CROSS_PREFIX)gcc
export CXX := $(CROSS_PREFIX)g++
export AR := $(CROSS_PREFIX)ar
export LD := $(CROSS_PREFIX)ld
export STRIP := $(CROSS_PREFIX)strip
export OBJCOPY := $(CROSS_PREFIX)objcopy

include $(RULE_DIR)/_rules-dist.mk
include $(RULE_DIR)/_rules-complib.mk
include $(RULE_DIR)/_rules-submods.mk

env:
	$(Q)echo ""
	$(Q)printf -- "-----------------------------------------------------------------\n"
	$(Q)$(foreach var,$(SHOW_ENV_VARS),$(call Dump_Var,$(var)))
	$(Q)printf -- "-----------------------------------------------------------------\n"
	$(Q)echo ""

else    # ifdef SUBDIRS

PKG_RPATH   := $(shell echo $(CURDIR)|sed 's:$(OUTPUT_DIR)/*::g')
PKG_NAME    ?= $(shell basename $(CURDIR))
PKG_SOURCE  ?= $(shell find $(PACKAGE_DIR) -name "$(PKG_NAME)*" | head -1)

DEPENDS     += $(DEPENDS_$(MODULE_NAME))
DEPENDS     := $(sort $(strip $(DEPENDS)))

ifdef CONFIG_SWITCH
PKG_SWITCH_V = $(CONFIG_SWITCH)
else
PKG_SWITCH_V := CONFIG_$(PKG_RPATH)
endif
PKG_SWITCH = $($(PKG_SWITCH_V))

ifdef ORIGIN

include $(RULE_DIR)/_rules-origin.mk

else    # ifdef ORIGIN

include $(RULE_DIR)/_rules-flat.mk

endif   # ifdef ORIGIN

include $(RULE_DIR)/_rules-modinfo.mk

endif   # ifdef SUBDIRS

include $(RULE_DIR)/_rules-coverage.mk
sinclude $(STAMP_POST_RULE)

ifneq ($(TOP_DIR),$(CURDIR))
INTERNAL_INCLUDES += -I$(SYSROOT_INC)
INTERNAL_INCLUDES += $(foreach d, $(shell find $(SYSROOT_INC) -type d), -I$(d))
INTERNAL_INCLUDES += -I$(TOP_DIR)
INTERNAL_INCLUDES += -I$(IMPORT_DIR)
INTERNAL_INCLUDES += -I$(IMPORT_DIR)/include
INTERNAL_INCLUDES += \
$(foreach d, \
    $(shell [ -d $(IMPORT_DIR)/$(CONFIG_VENDOR) ] && find -L $(IMPORT_DIR)/$(CONFIG_VENDOR)/include -type d), \
    -I$(d) \
)
INTERNAL_INCLUDES += $(foreach mod, $(MODULE_NAME) $(HDR_REFS), \
    $(foreach d, \
        $(shell find $(TOP_DIR)/$(mod)/ -type d -a -name "[^.]*"), \
        -I$(d) \
    ) \
)
INTERNAL_INCLUDES += \
    $(foreach d, \
        $(shell find $(OUTPUT_DIR)/$(MODULE_NAME) -type d -a -name "[^.]*"), \
        -I$(d) \
    )
INTERNAL_INCLUDES := $(strip $(sort $(INTERNAL_INCLUDES)))

EXTERNAL_INCLUDES += $(foreach mod, $(DEPENDS), \
    $(foreach d, \
        $(shell $(SHELL_DBG) find \
            $(SYSROOT_INC)/$(mod)/ -maxdepth 2 -type d 2>/dev/null) \
        $(shell $(SHELL_DBG) find \
            $(IMPORT_VDRDIR)/include/$(mod)/ -maxdepth 2 -type d 2>/dev/null), \
        -I$(d) \
    ) \
)
EXTERNAL_INCLUDES := $(strip $(EXTERNAL_INCLUDES))
endif   # ifneq ($(TOP_DIR),$(CURDIR))

ifeq (dynamic,$(strip $(CONFIG_LIB_EXPORT)))
CFLAGS  += -fPIC
endif

CFLAGS  := $(sort $(strip $(CFLAGS)))

LDFLAGS += -L$(SYSROOT_LIB)
LDFLAGS += -L$(IMPORT_VDRDIR)/$(PREBUILT_LIBDIR)

LDFLAGS += $(foreach d,$(DEPENDS_$(MODULE_NAME)),$(REF_LDFLAGS_$(d)))

WATCHED_VARS = \
    TARGET \
    CFLAGS \
    CC \
    LDFLAGS \
    CURDIR \
    INTERNAL_INCLUDES \
    DEPENDS \
    MAKECMDGOALS \
    EXTERNAL_INCLUDES \
    LIBA_TARGET \
    LIBSO_TARGET \

ALL_TARGETS := $(TARGET) $(LIBSO_TARGET) $(LIBA_TARGET) $(firstword $(KMOD_TARGET))

ifneq (,$(strip $(PKG_SWITCH)))
ifneq (,$(strip $(OVERRIDE_BUILD)))

$(LIBA_TARGET) $(LIBSO_TARGET) all:
	$(Q)echo -ne "\r                                              \r"
	$(Q)$(MAKE) CFLAGS='$(CFLAGS) -I$(SYSROOT_INC)' build

#	$(Q)$(MAKE) install \
#	    INS_LIBDIR=$(SYSROOT_LIB) \
#	    INS_INCDIR=$(SYSROOT_INC)/$(LIBHDR_DIR)

ifneq (,$(strip $(PKG_SOURCE)))

	$(Q) \
	SRCDIR=$$(basename $(PKG_SOURCE)); \
	if [ -d $${SRCDIR} ]; then \
	    rm -rf $(SYSROOT_INC)/$(MODULE_NAME)/$(LIBHDR_DIR).cache; \
	    mkdir -p $(SYSROOT_INC)/$(MODULE_NAME)/$(LIBHDR_DIR).cache; \
	    $(MAKE) -C $${SRCDIR} install \
	        CFLAGS='$(CFLAGS) -I$(SYSROOT_INC)' \
	        INS_LIBS=$(LIBA_TARGET) \
	        INS_LIBDIR=$(SYSROOT_LIB) \
	        INS_INCDIR=$(SYSROOT_INC)/$(MODULE_NAME)/$(LIBHDR_DIR).cache \
	    ; \
	    for iter in \
	        $$(find $(SYSROOT_INC)/$(MODULE_NAME)/$(LIBHDR_DIR).cache -type f); do \
	        if  [ ! -e $(SYSROOT_INC)/$(LIBHDR_DIR)/$$(basename $${iter}) ] || \
	            ! diff -q $${iter} $(SYSROOT_INC)/$(LIBHDR_DIR)/$$(basename $${iter}); then \
	            cp -f $${iter} $(SYSROOT_INC)/$(LIBHDR_DIR); \
	            printf "%-32s%s%s\n" "[CP] $(LIBHDR_DIR)/$$(basename $${iter})" \
	                   "<= $$(echo $${iter}|sed 's:$(TOP_DIR)/::g')"; \
	        fi; \
	    done; \
	    rm -rf $(SYSROOT_INC)/$(MODULE_NAME)/$(LIBHDR_DIR).cache; \
	fi

	$(Q)mkdir -p $(LIBOBJ_TMPDIR)/$(MODULE_NAME)
	$(Q)cp -f $(SYSROOT_LIB)/$(LIBA_TARGET) $(LIBOBJ_TMPDIR)/$(MODULE_NAME)
	$(Q)cd $(LIBOBJ_TMPDIR)/$(MODULE_NAME) && ar xf $(LIBA_TARGET)
	$(Q)rm -f $(LIBOBJ_TMPDIR)/$(MODULE_NAME)/$(LIBA_TARGET)

endif   # PKG_SOURCE is directory and has its own 'build:'

else    # PKG_SWITCH is on without OVERRIDE_BUILD
.PHONY: before-build

all: before-build $(ALL_TARGETS)

before-build:
ifdef PREP_BUILD_HOOK
	$(Q)$(RECURSIVE_MAKE) pre-sub-build target-$(MODULE_NAME)
endif
	@$(call $(PREP_BUILD_HOOK))

endif   # End PKG_SWITCH is on Section

else    # PKG_SWITCH = n
all:
	$(Q)true
endif

clean:
	$(Q)rm -f $(strip $(ALL_TARGETS) $(OBJS) $(LIB_OBJS) $(OBJS:.o=.d) $(LIB_OBJS:.o=.d)) *.o.e *.d *.o *.a *.so *.log *.gc*

ifneq (,$(strip $(OVERRIDE_BUILD)))
ifneq (,$(strip $(PKG_SOURCE)))
	$(Q) \
	SRCDIR=$$(basename $(PKG_SOURCE)); \
	if [ -d $${SRCDIR} ]; then \
	    $(MAKE) -C $${SRCDIR} clean \
	        INS_LIBDIR=$(SYSROOT_LIB) \
	        INS_INCDIR=$(SYSROOT_INC)/$(LIBHDR_DIR) \
        ; \
	fi
endif
endif

%.o: %.c $(HD_MAKEFILE)
	$(call Brief_Log,"CC")
	$(call Inspect_Env,$(WATCHED_VARS))
	$(Q) \
	set -o pipefail; \
	$(CC) -I$(CURDIR) \
	    $(INTERNAL_INCLUDES) \
	    $(EXTERNAL_INCLUDES) \
	    $(CFLAGS) \
	    -c -o $@ $<
ifneq (,$(OBJCOPY_FLAGS))
	$(Q)$(OBJCOPY) $(OBJCOPY_FLAGS) $@
endif

NODEP_LIST = \
    $(SYSROOT_INC)/git_version.h \
    $(SYSROOT_INC)/platform.h \
    $(SYSROOT_INC)/product.h \
    $(SYSROOT_INC)/product_config.h \

%.d: %.c
	$(Q) \
	$(CC) -MM -I$(CURDIR) \
	    $(INTERNAL_INCLUDES) \
	    $(EXTERNAL_INCLUDES) \
	    $(CFLAGS) \
	$< > $@.$$$$; \
	sed -i 's!$(shell basename $*)\.o[ :]!$*.o:!1' $@.$$$$; \
	mv $@.$$$$ $@;

%.o: %.cpp
	$(call Brief_Log,"CC")
	$(call Inspect_Env,$(WATCHED_VARS))
	$(Q)$(CXX) -I$(CURDIR) \
	    $(INTERNAL_INCLUDES) \
	    $(EXTERNAL_INCLUDES) \
	    $(CFLAGS) \
	    -c -o $@ $<

%.d: %.cpp
	$(Q) \
	$(CXX) -MM -I$(CURDIR) \
	    $(INTERNAL_INCLUDES) \
	    $(EXTERNAL_INCLUDES) \
	    $(CFLAGS) \
	$< > $@.$$$$; \
	$(foreach D,$(NODEP_LIST),sed -i 's:$(D)::g' $@.$$$$;) \
	sed 's,\($*\)\.o[ :]*,\1.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$;

ifeq (,$(strip $(OVERRIDE_BUILD)))
include $(RULE_DIR)/_rules-libs.mk
include $(RULE_DIR)/_rules-prog.mk
include $(RULE_DIR)/_rules-kmod.mk
endif

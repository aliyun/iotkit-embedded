final-out: NO_DIST_FILES = \
    include/product_config.h \
    include/autoconf.h \
    include/git_version.h \
    include/libubox \
    include/alink-sdk \
    lib/libuboxreduce.so \
    lib/lib{lua,form,history,menu,ncurses,panel,readline}* \
    lib/terminfo \
    lib/libalink.a \
    lib/libjccmsg-0.9.28.so \
    lib/libcommon.a \
    lib/libnvram-0.9.28.so \
    lib/librouter_api.a \

final-out: EXTRA_DIST_FILES = \
    $(IMPORT_VDRDIR)/demo/alink_sample.c \
    $(IMPORT_VDRDIR)/platform/*.[ch] \
    $(IMPORT_VDRDIR)/product/product.c \

ifdef COMP_LIB
final-out: sub-mods
	$(TOP_Q) \
	if [ -f $(STAMP_PRJ_CFG) ]; then true; else \
	    rm -rf $(DIST_DIR); \
	    mkdir -p $(DIST_DIR) $(FINAL_DIR); \
	    if  [ ! -f $(SYSROOT_LIB)/lib$(COMP_LIB_NAME).a ] && \
	        [ ! -f $(SYSROOT_LIB)/lib$(COMP_LIB_NAME).so ]; then \
	        $(call Build_CompLib, FORCE) \
	    fi; \
	    for i in bin lib include; do \
	        [ -d $(OUTPUT_DIR)/usr/$${i} ] && cp -rf $(OUTPUT_DIR)/usr/$${i} $(FINAL_DIR) || true; \
	    done; \
	fi
else
final-out: sub-mods
	$(TOP_Q) \
	if [ -f $(STAMP_PRJ_CFG) ]; then true; else \
	    rm -rf $(DIST_DIR); \
	    mkdir -p $(DIST_DIR) $(FINAL_DIR); \
	    for i in bin lib include; do \
	        [ -d $(OUTPUT_DIR)/usr/$${i} ] && cp -rf $(OUTPUT_DIR)/usr/$${i} $(FINAL_DIR) || true; \
	    done; \
	fi
endif

	$(TOP_Q) \
	for i in $(NO_DIST_FILES); do \
	    rm -rf $(FINAL_DIR)/$${i}; \
	done;
	$(TOP_Q)mkdir -p $(FINAL_DIR)/sample/{demo,product,platform}
	$(TOP_Q) \
	for i in $(EXTRA_DIST_FILES); do \
	    cp -f $${i} $(FINAL_DIR)/sample/$$(dirname $${i}|head -1|xargs basename) \
	    2>/dev/null || true; \
	done

ifneq (,$(strip $(CONFIG_EMB_BASIC_SDK)))
	$(TOP_Q)cp -f include/alink_export.h $(FINAL_DIR)/include
endif
ifneq (,$(strip $(CONFIG_EMB_GATEWAY_SDK)))
	$(TOP_Q)cp -f include/alink_export_gateway.h $(FINAL_DIR)/include
	$(TOP_Q)cp -f include/alink_export_zigbee.h $(FINAL_DIR)/include
endif
ifneq (,$(strip $(CONFIG_EMB_ASR_SDK)))
	$(TOP_Q)cp -f include/alink_export_asr.h $(FINAL_DIR)/include
	$(TOP_Q)cp $(IMPORT_VDRDIR)/demo/alink_audio_sample.c $(FINAL_DIR)/sample/demo
	$(TOP_Q)cp $(IMPORT_VDRDIR)/demo/{sample.*,aliasr-ca.pem} $(FINAL_DIR)/bin
endif
ifneq (,$(strip $(CONFIG_EMB_IOT_SDK)))
	$(TOP_Q) \
	if [ -f $(IMPORT_VDRDIR)/libs/libtfs.a ]; then \
	    cp -f $(IMPORT_VDRDIR)/libs/libtfs.a $(FINAL_DIR)/lib; \
	fi
	$(TOP_Q)cp -f $(EXPORT_DIR)/*.h $(FINAL_DIR)/include
	$(TOP_Q)cp -f $(EXPORT_DIR)/*.c $(FINAL_DIR)/sample/demo
endif

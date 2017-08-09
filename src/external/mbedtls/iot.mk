ORIGIN  := 1

PKG_SOURCE  := $(PACKAGE_DIR)/mbedtls-2.5.0.tar.bz2
REF_LDFLAGS := -lmbedtls -lmbedx509 -lmbedcrypto

config:
	$(Q)true

build:
	$(Q)cd mbedtls-* && make -j8 lib

install:
	$(Q)cd mbedtls-* && \
	    cp -rf include/mbedtls $(SYSROOT_INC) && \
	    cp -f library/*.a $(SYSROOT_LIB)

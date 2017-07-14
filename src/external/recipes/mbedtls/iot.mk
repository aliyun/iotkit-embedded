ORIGIN  := 1

REF_LDFLAGS := -lmbedtls -lmbedcrypto -lmbedx509

config:
	$(Q)true

build:
	$(Q)cd mbedtls-* && make -j8 lib

install:
	$(Q)cd mbedtls-* && \
	    cp -rf include/mbedtls $(SYSROOT_INC) && \
	    cp -f library/*.a $(SYSROOT_LIB)

ORIGIN  := 1

config:
	true

build:
	cd mbedtls-* && make -j8 lib

install:
	cd mbedtls-* && \
	    cp -rf include/mbedtls $(SYSROOT_INC) && \
	    cp -f library/*.a $(SYSROOT_LIB)


LIB_PLATFORM = $(PLATFORM_LIB_NAME).a
BUILD_OUT_PATH = ./ssl/build/

INCLUD_PATH = -I./
INCLUD_PATH += -I./os/
INCLUD_PATH += -I./network/
INCLUD_PATH += -I./ssl/


#path of include files of mbedtls
ifeq ($(SSL), mbedtls)
INCLUD_PATH += -I./ssl/mbedtls/mbedtls/include/
INCLUD_PATH += -I./ssl/mbedtls/mbedtls/include/mbedtls/
INCLUD_PATH += -I./ssl/mbedtls/mbedtls/configs/$(OS)/
endif


CFLAGS =  -Wall -g ${INCLUD_PATH} 
ifeq ($(BUILD_TYPE), debug)
CFLAGS += -DALIOT_DEBUG
endif

#cacualte source files
SRC_FILE = $(wildcard ./os/$(OS)/*.c)
SRC_FILE += $(wildcard ./network/$(NETWORK)/*.c)
ifneq ($(SSL), )
SRC_FILE += $(wildcard ./ssl/$(SSL)/*.c)
endif

$(info platform_src_files = $(SRC_FILE))


OBJ = $(patsubst %.c,%.o, ${SRC_FILE})
CFILE = $(patsubst %.o,%.c, ${OBJ})


.PHONY:clean all

all: ${LIB_PLATFORM}
	$(info platform library name is: $(LIB_PLATFORM))

${LIB_PLATFORM}: ${OBJ} lib_ssl
ifeq ($(SSL), mbedtls)
	$(AR) $(ARFLAGS) $@ $(OBJ) ${BUILD_OUT_PATH}*.o
else
	$(AR) $(ARFLAGS) $@ $(OBJ)        
endif
	-rm -rf ${BUILD_OUT_PATH}*.o
	cp ${LIB_PLATFORM} ../../libs/

lib_ssl:
	mkdir -p ${BUILD_OUT_PATH}
ifeq ($(SSL), mbedtls)
	make -C ./ssl/mbedtls/mbedtls lib -e CC=$(CC) AR=$(AR) OS=$(OS)
	cp -RP ./ssl/mbedtls/mbedtls/library/libmbedtls.*    $(BUILD_OUT_PATH)
	cp -RP ./ssl/mbedtls/mbedtls/library/libmbedx509.*   $(BUILD_OUT_PATH)
	cp -RP ./ssl/mbedtls/mbedtls/library/libmbedcrypto.* $(BUILD_OUT_PATH)
	cd $(BUILD_OUT_PATH) && $(AR) x libmbedtls.a
	cd $(BUILD_OUT_PATH) && $(AR) x libmbedx509.a
	cd $(BUILD_OUT_PATH) && $(AR) x libmbedcrypto.a
endif
	

${OBJ}:%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-rm -rf ${BUILD_OUT_PATH}
	-rm ${OBJ} ${LIB_PLATFORM}
	-make -C ./ssl/mbedtls/mbedtls clean

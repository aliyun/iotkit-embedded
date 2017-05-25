include ./../make.settings

#path of include files
INCLUD_PATH = -I./auth/
INCLUD_PATH += -I./mqtt/
INCLUD_PATH += -I./platform/os/$(PLATFORM)/
INCLUD_PATH += -I./platform/ssl/$(SSL_LIB_NAME)/
INCLUD_PATH += -I./system/
INCLUD_PATH += -I./utility/

#path of include files of mbedtls
ifeq ($(MBEDTLS_LIB_ENABLE), y)
INCLUD_PATH += -I./libs/mbedtls/include/
INCLUD_PATH += -I./libs/mbedtls/include/mbedtls/
INCLUD_PATH += -I./libs/mbedtls/configs/$(PLATFORM)/
endif


#cacualte source files
DIRS := $(shell find ./ \( -path "./libs" -o -path "./platform" \) -prune -o -type d)
$(info Src_Dir = $(DIRS))

SRC_FILE = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c))
$(info src_files = $(SRC_FILE))

SRC_FILE += $(wildcard ./platform/os/$(PLATFORM)/*.c)
SRC_FILE += $(wildcard ./platform/ssl/$(SSL_LIB_NAME)/*.c)


#path of src files of tls interface
ifeq ($(MBEDTLS_LIB_ENABLE), y)
SRC_FILE += $(wildcard ${SSL_PORTING_SRC}*.c)
endif


OBJ = $(patsubst %.c,%.o, ${SRC_FILE})
CFILE = $(patsubst %.o,%.c, ${OBJ})


LIB_STATIC = ${ALIIOT_SDK_LIB_NAME}.a

CC = $(PLATFORM_CC)
AR = $(PLATFORM_AR)

ifeq ($(CHANNEL_ENCRYPT), SSL)
CFLAGS = -DALIOT_CHANNEL_ENCRYPT_SSL
endif
CFLAGS +=  -Wall -g ${INCLUD_PATH} 
ARFLAGS = rcs

BUILD_OUT_PATH = ./Build/
TLS_LIB_PATH = ./libs/mbedtls/
EMBED_TLS_CREATE_PATH = ./libs/mbedtls/library/


.PHONY:clean libs demo

all: ${LIB_STATIC}  


${LIB_STATIC}: libs ${OBJ}
${LIB_STATIC}: ${OBJ}
	$(AR) $(ARFLAGS) $@ $(OBJ) ${BUILD_OUT_PATH}*.o 
	-rm -rf ${BUILD_OUT_PATH}*.o
	
libs:
	mkdir -p ${BUILD_OUT_PATH}
ifeq ($(MBEDTLS_LIB_ENABLE), y)
	make -C $(TLS_LIB_PATH) lib -e CC=$(PLATFORM_CC) AR=$(PLATFORM_AR)
	cp -RP $(EMBED_TLS_CREATE_PATH)libmbedtls.*    $(BUILD_OUT_PATH)
	cp -RP $(EMBED_TLS_CREATE_PATH)libmbedx509.*   $(BUILD_OUT_PATH)
	cp -RP $(EMBED_TLS_CREATE_PATH)libmbedcrypto.* $(BUILD_OUT_PATH)
	cd $(BUILD_OUT_PATH) && $(AR) x libmbedtls.a
	cd $(BUILD_OUT_PATH) && $(AR) x libmbedx509.a
	cd $(BUILD_OUT_PATH) && $(AR) x libmbedcrypto.a
endif
	

${OBJ}:%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-rm -rf ${BUILD_OUT_PATH}
	-rm ${OBJ} ${LIB_STATIC}
	-make -C $(TLS_LIB_PATH) clean
	

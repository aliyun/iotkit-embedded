
include ./../../make.settings

CC=$(PLATFORM_CC)
CFLAGS += -std=gnu99 -Wall -g
LDFLAGS = -lm -pthread

ifeq ($(PLATFORM), windows)
LDFLAGS += -lws2_32
endif

INCLUD_PATH = -I./../../src/auth/
INCLUD_PATH += -I./../../src/mqtt/
INCLUD_PATH += -I./../../src/system/
INCLUD_PATH += -I./../../src/utility/
INCLUD_PATH += -I./../../src/shadow/
INCLUD_PATH += -I./../../src/platform/os/$(PLATFORM)/
INCLUD_PATH += -I./../../src/platform/ssl/$(SSL_LIB_NAME)/

#path of include files of mbedtls
ifeq ($(MBEDTLS_LIB_ENABLE), y)
INCLUD_PATH += -I./../../src/libs/mbedtls/include/
INCLUD_PATH += -I./../../src/libs/mbedtls/include/mbedtls/
INCLUD_PATH += -I./../../src/libs/mbedtls/configs/$(PLATFORM)/
endif

TARGET = deviceshadow

all:
	$(CC) $(CFLAGS) $(INCLUD_PATH) -o $(TARGET) deviceshadow.c ../../src/libaliot_sdk.a $(LDFLAGS) 
	
clean:
	 rm -rf $(TARGET)

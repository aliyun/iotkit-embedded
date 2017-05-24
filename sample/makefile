include ../make.settings
CC=$(PLATFORM_CC)
CFLAGS += -std=gnu99 -Wall -g

INCLUD_PATH = -I./../src/auth/
INCLUD_PATH += -I./../src/mqtt/
INCLUD_PATH += -I./../src/system/
INCLUD_PATH += -I./../src/utility/
INCLUD_PATH += -I./../src/shadow/
INCLUD_PATH += -I./../src/platform/os/$(PLATFORM)/
INCLUD_PATH += -I./../src/platform/ssl/$(SSL_LIB_NAME)/

#path of include files of mbedtls
ifeq ($(MBEDTLS_LIB_ENABLE), y)
INCLUD_PATH += -I./../src/libs/mbedtls/include/
INCLUD_PATH += -I./../src/libs/mbedtls/include/mbedtls/
INCLUD_PATH += -I./../src/libs/mbedtls/configs/$(PLATFORM)/
endif

TARGET = demo

all:
	$(CC) $(CFLAGS) $(INCLUD_PATH) -o $(TARGET) demo.c ../src/libaliot_sdk.a -lm -lpthread -lws2_32
	
clean:
	 rm -rf $(TARGET)

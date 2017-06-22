
include ./../../make.settings

LIB_ALIOT_SDK = $(ALIOT_PLATFORM_LIB_NAME).a
LIB_ALIOT_PLATFORM = $(ALIOT_SDK_LIB_NAME).a


CC=$(PLATFORM_CC)
CFLAGS += -std=gnu99 -Wall -g
LDFLAGS = -lm -pthread

ifeq ($(PLATFORM_OS), windows)
LDFLAGS += -lws2_32
endif

INCLUD_PATH = -I./../../src/auth/
INCLUD_PATH += -I./../../src/mqtt/
INCLUD_PATH += -I./../../src/system/
INCLUD_PATH += -I./../../src/utility/
INCLUD_PATH += -I./../../src/shadow/
INCLUD_PATH += -I./../../src/platform/


TARGET = deviceshadow

all:
	$(CC) $(CFLAGS) $(INCLUD_PATH) -o $(TARGET) deviceshadow.c ../../build_out/$(LIB_ALIOT_PLATFORM) ../../build_out/$(LIB_ALIOT_SDK) $(LDFLAGS) 
	
clean:
	 rm -rf $(TARGET)


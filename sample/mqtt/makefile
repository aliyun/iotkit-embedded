
include ./../../make.settings

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

TARGET = mqtt

all:
	$(CC) $(CFLAGS) $(INCLUD_PATH) -o $(TARGET) mqtt.c ./../../libs/libaliot_sdk.a ./../../libs/libplatform.a $(LDFLAGS) 
	
clean:
	 rm -rf $(TARGET)

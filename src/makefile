include ./../make.settings

#path of include files
INCLUD_PATH = -I./auth/
INCLUD_PATH += -I./mqtt/
INCLUD_PATH += -I./system/
INCLUD_PATH += -I./utility/
INCLUD_PATH += -I./platform/


#cacualte source files
DIRS := $(shell find ./ \( -path "./libs" -o -path "./platform" \) -prune -o -type d)
$(info Src_Dir = $(DIRS))

SRC_FILE = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c))
$(info src_files = $(SRC_FILE))


OBJ = $(patsubst %.c,%.o, ${SRC_FILE})
CFILE = $(patsubst %.o,%.c, ${OBJ})

LIB_STATIC = ${ALIIOT_SDK_LIB_NAME}.a

CC = $(PLATFORM_CC)
AR = $(PLATFORM_AR)

ifeq ($(MQTT_CHANNEL_ENCRYPT_SSL), yes)
CFLAGS = -DALIOT_MQTT_CHANNEL_ENCRYPT_SSL
endif
CFLAGS +=  -Wall -g ${INCLUD_PATH} 
ARFLAGS = rcs


.PHONY:clean all

all: ${LIB_STATIC}

${LIB_STATIC}: ${OBJ}
	$(AR) $(ARFLAGS) $@ $(OBJ)
	cp ${LIB_STATIC} ../libs/

${OBJ}:%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-rm ${OBJ} ${LIB_STATIC}

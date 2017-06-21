
#MQTT_TCP=yes


#path of include files
INCLUD_PATH = -I./auth/
INCLUD_PATH += -I./mqtt/
INCLUD_PATH += -I./system/
INCLUD_PATH += -I./utility/
INCLUD_PATH += -I./platform/


#cacualte source files
DIRS := $(shell find .  \( -path ./platform \) -prune -o -type d -print )
$(info Src_Dir = $(DIRS))

SRC_FILE = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c))
$(info src_files = $(SRC_FILE))


OBJ = $(patsubst %.c,%.o, ${SRC_FILE})
CFILE = $(patsubst %.o,%.c, ${OBJ})

LIB_STATIC = ${SDK_LIB_NAME}.a


CFLAGS = -Wall -g ${INCLUD_PATH}

ifeq ($(MQTT_TCP), yes)
CFLAGS += -DALIOT_MQTT_TCP
endif

ifeq ($(BUILD_TYPE), debug)
CFLAGS += -DALIOT_DEBUG
endif

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

include ./make.settings

.PHONY: libaliot libplatform sample all clean 

prefix ?= .
$(info prefix is: '$(prefix)')

all: libsdk libplatform libext sample
	$(info make all)

libsdk:
	mkdir -p ./build_out 
	$(info make aliot library)
	make -C ./src/ \
    	CC=$(PLATFORM_CC) \
    	AR=$(PLATFORM_AR) \
    	OS=$(PLATFORM_OS) \
    	NETWORK=$(PLATFORM_NETWORK) \
    	SSL=$(PLATFORM_SSL) \
    	SDK_LIB_NAME=$(ALIOT_SDK_LIB_NAME) \
    	BUILD_TYPE=$(BUILD_TYPE) \
    	DIRECT_MQTT=$(ALIOT_DIRECT_MQTT) \


	
libplatform:
	mkdir -p ./build_out
	$(info make platform library)
	make -C ./src/platform/ \
		CC=$(PLATFORM_CC) \
		AR=$(PLATFORM_AR) \
		OS=$(PLATFORM_OS) \
		NETWORK=$(PLATFORM_NETWORK) \
		SSL=$(PLATFORM_SSL) \
		PLATFORM_LIB_NAME=$(ALIOT_PLATFORM_LIB_NAME) \
		BUILD_TYPE=$(BUILD_TYPE) \
		DIRECT_MQTT=$(ALIOT_DIRECT_MQTT) \


libext:
	$(info make external library)
	make -C ./libs/ CC=$(PLATFORM_CC) AR=$(PLATFORM_AR)
	
sample:

	$(info make mqtt sample)
	make -C ./sample/mqtt/ \
		CC=$(PLATFORM_CC) \
		AR=$(PLATFORM_AR) \
		OS=$(PLATFORM_OS) \
		NETWORK=$(PLATFORM_NETWORK) \
		SSL=$(PLATFORM_SSL) \
		PLATFORM_LIB_NAME=$(ALIOT_PLATFORM_LIB_NAME) \
		BUILD_TYPE=$(BUILD_TYPE) \
		DIRECT_MQTT=$(ALIOT_DIRECT_MQTT) \

	$(info make device shadow sample)
	make -C ./sample/deviceshadow/ \
		CC=$(PLATFORM_CC) \
		AR=$(PLATFORM_AR) \
		OS=$(PLATFORM_OS) \
		NETWORK=$(PLATFORM_NETWORK) \
		SSL=$(PLATFORM_SSL) \
		PLATFORM_LIB_NAME=$(ALIOT_PLATFORM_LIB_NAME) \
		BUILD_TYPE=$(BUILD_TYPE) \
		DIRECT_MQTT=$(ALIOT_DIRECT_MQTT) \


install:
	mkdir -p $(prefix)/lib
	mkdir -p $(prefix)/include
	mkdir -p $(prefix)/include/utility
	mkdir -p $(prefix)/include/platform
	mkdir -p $(prefix)/include/platform/os
	mkdir -p $(prefix)/include/platform/network
	mkdir -p $(prefix)/include/platform/ssl
	mkdir -p $(prefix)/include/mqtt
	mkdir -p $(prefix)/include/shadow
	mkdir -p $(prefix)/include/auth
	mkdir -p $(prefix)/include/system
	find ./src/utility -name '*.h' -exec cp -f {} $(prefix)/include/utility/ \;
	find ./src/platform -maxdepth 1  -name '*.h' -exec cp -f {} $(prefix)/include/platform/ \;
	find ./src/platform/os -maxdepth 1  -name '*.h' -exec cp -f {} $(prefix)/include/platform/os/ \;
	find ./src/platform/network -maxdepth 1  -name '*.h' -exec cp -f {} $(prefix)/include/platform/network/ \;
	find ./src/platform/ssl -maxdepth 1  -name '*.h' -exec cp -f {} $(prefix)/include/platform/ssl/ \;
	find ./src/auth -name '*.h' -exec cp -f {} $(prefix)/include/auth/ \;
	find ./src/system -name '*.h' -exec cp -f {} $(prefix)/include/system/ \;
	cp -f ./src/mqtt/aliot_mqtt_client.h  $(prefix)/include/mqtt/
	cp -f ./src/shadow/aliot_shadow.h $(prefix)/include/shadow/
	cp -rvf ./build_out/ $(prefix)/lib
	find ./build_out -name '*.a' -exec cp -f {} $(prefix)/lib/ \;

clean:
	rm -fr ./build_out/
	$(info make clean aliot library)
	make -C ./src/ clean 
	$(info make clean platform library)
	make -C ./src/platform clean OS=$(PLATFORM_OS) NETWORK=$(PLATFORM_NETWORK) SSL=$(PLATFORM_SSL) PLATFORM_LIB_NAME=$(ALIOT_PLATFORM_LIB_NAME)
	$(info make clean external library)
	make -C ./libs/ clean 
	$(info make clean mqtt sample)
	make -C ./sample/mqtt clean
	$(info make clean device shadow sample)
	make -C ./sample/deviceshadow clean

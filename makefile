include ./make.settings

.PHONY: libaliot libplatform sample all clean 

all: libaliot libplatform sample
	$(info make all)

libaliot:
	mkdir -p ./libs 
	$(info make aliot library)
	make -C ./src/ \
    	-e \
    	CC=$(PLATFORM_CC) \
    	AR=$(PLATFORM_AR) \
    	OS=$(PLATFORM_OS) \
    	NETWORK=$(PLATFORM_NETWORK) \
    	SSL=$(PLATFORM_SSL) \
    	SDK_LIB_NAME=$(ALIOT_SDK_LIB_NAME) \
    	BUILD_TYPE=$(BUILD_TYPE)
	
libplatform:
	mkdir -p ./libs
	$(info make platform library)
	make -C ./src/platform/ \
		-e \
		CC=$(PLATFORM_CC) \
		AR=$(PLATFORM_AR) \
		OS=$(PLATFORM_OS) \
		NETWORK=$(PLATFORM_NETWORK) \
		SSL=$(PLATFORM_SSL) \
		PLATFORM_LIB_NAME=$(ALIOT_PLATFORM_LIB_NAME) \
		BUILD_TYPE=$(BUILD_TYPE)
	
sample:
	$(info make mqtt sample)
	make -C ./sample/mqtt/
	$(info make device shadow sample)
	make -C ./sample/deviceshadow/

clean:
	-rm -fr ./libs/
	$(info make clean aliot library)	
	make -C ./src/ clean 
	$(info make clean platform library)	
	make -C ./src/platform clean -e OS=$(PLATFORM_OS) NETWORK=$(PLATFORM_NETWORK) SSL=$(SSL) PLATFORM_LIB_NAME=$(ALIOT_PLATFORM_LIB_NAME)

	$(info make clean mqtt sample)
	make -C ./sample/mqtt clean
	$(info make clean device shadow sample)
	make -C ./sample/deviceshadow clean

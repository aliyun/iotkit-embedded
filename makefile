include ./make.settings

#If disable channel encrypt by SSL, it will NOT build SSL/TLS library.
#ifeq ($(CHANNEL_ENCRYPT_SSL), yes)
SSL=$(PLATFORM_SSL)
#else 
#SSL=
#endif

.PHONY: libaliot libplatform sample all clean 

all: libaliot libplatform sample
	$(info make all)

libaliot:
	$(info make aliot library)
	make -C ./src/
	
libplatform:
	$(info make platform library)
	make -C ./src/platform/ -e CC=$(PLATFORM_CC) AR=$(PLATFORM_AR) OS=$(PLATFORM_OS) NETWORK=$(PLATFORM_NETWORK) SSL=$(SSL)
	
sample:
	$(info make mqtt sample)
	make -C ./sample/mqtt/
	$(info make device shadow sample)
	make -C ./sample/deviceshadow/

clean:
	-rm -fr ./libs/libaliot_sdk.a
	-rm -fr ./libs/libplatform.a
	$(info make clean aliot library)	
	make -C ./src/ clean
	$(info make clean platform library)	
	make -C ./src/platform clean
	$(info make clean mqtt sample)
	make -C ./sample/mqtt clean
	$(info make clean device shadow sample)
	make -C ./sample/deviceshadow clean

	

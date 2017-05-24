include ./make.settings

.PHONY: all clean

all:
	$(info make aliot-sdk)
	make -C ./src/
	$(info make mqtt sample)
	make -C ./sample/mqtt/
	$(info make device shadow sample)
	make -C ./sample/deviceshadow/


clean:
	$(info make clean aliot-sdk)	
	make -C ./src/ clean
	$(info make clean mqtt sample)
	make -C ./sample/mqtt clean
	$(info make clean device shadow sample)
	make -C ./sample/deviceshadow clean

	

include ./make.settings

.PHONY: all clean

all:
	$(info make aliot-sdk)
	make -C ./src/
	$(info make sample)
	make -C ./sample/


clean:
	$(info make clean aliot-sdk)	
	make -C ./src/ clean
	$(info make clean sample)
	make -C ./sample/ clean
	

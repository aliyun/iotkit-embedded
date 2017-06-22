

.PHONY:clean all


all: gtest
	

clean: 
	make -C ./googletest/googletest/make clean
	-rm -vf *.a
	
	
gtest:
	make -C ./googletest/googletest/make gtest_main.a
	cp -vf ./googletest/googletest/make/gtest_main.a ./
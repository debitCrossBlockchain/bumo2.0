OS_NAME = $(shell uname -s)
#darwin or  linux
LC_OS_NAME = $(shell echo $(OS_NAME) | tr '[A-Z]' '[a-z]')

MAKE_PLATFORM = pwd
ifeq ($(LC_OS_NAME), darwin)
	CUR_OS = mac
	MAKE_PLATFORM=cat src/3rd/v8_target/mac-v8.tar.gz.* | tar -zxv -d src/3rd/v8_target/
else
	CUR_OS = linux
endif

.PHONY:all
all:
	$(MAKE_PLATFORM); cd build/$(CUR_OS); \
	cmake -DSVNVERSION=$(bumo_version) -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_VERBOSE_MAKEFILE=ON ../../src; \
	make -j 4

.PHONY:clean_all clean clean_build clean_3rd
clean_all:clean clean_build clean_3rd

clean:
	if [ -d ./lib ]; then rm -rf bin/* && cd lib/ && ls | grep -v http |grep -v ed25519 | xargs rm -rf; fi

clean_3rd:
	cd src/3rd && make clean_3rd && cd ../../

clean_build:
	cd build/$(CUR_OS)/; ls |grep -v bumo | xargs rm -rf

.PHONY:install uninstall
install:
	cd build/$(CUR_OS) && make install && make soft_link -f MakeSupplement

uninstall:
	cd build/$(CUR_OS) && make uninstall -f MakeSupplement


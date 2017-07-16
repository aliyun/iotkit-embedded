LIBA_TARGET := libutils.a

PKG_SOURCE  := src/packages/LITE-utils
# PKG_UPDATE  := 'git@gitlab.alibaba-inc.com:iot-middleware/LITE-utils.git'

build:
	$(Q)$(MAKE) -C LITE-utils library

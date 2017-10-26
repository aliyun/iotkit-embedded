CONFIG_ENV_CFLAGS += -Os -Wall -Werror

CONFIG_LIB_EXPORT := static

PLATFORM_CC         := i686-w64-mingw32-gcc
PLATFORM_AR         := i686-w64-mingw32-ar
PLATFORM_OS         := win7

OVERRIDE_LD         := i686-w64-mingw32-ld
OVERRIDE_STRIP      := i686-w64-mingw32-strip
OVERRIDE_OBJCOPY    := i686-w64-mingw32-objcopy

CONFIG_src/coap     :=

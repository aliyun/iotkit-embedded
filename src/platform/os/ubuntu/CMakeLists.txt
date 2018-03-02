file(GLOB C_SOURCES "*.c")
add_library(platform_os_linux OBJECT ${C_SOURCES})

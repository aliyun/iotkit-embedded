file(GLOB C_SOURCES "*.c")
add_library(platform_ssl_mbedtls OBJECT ${C_SOURCES})

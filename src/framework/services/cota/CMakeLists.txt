file(GLOB C_SOURCES "config_ota.c")
add_library(cota STATIC ${C_SOURCES})

target_link_libraries(cota iot_sdk)
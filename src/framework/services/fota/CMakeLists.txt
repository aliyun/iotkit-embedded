file(GLOB C_SOURCES "service_ota.c")
add_library(fota STATIC ${C_SOURCES})

target_link_libraries(fota iot_sdk)


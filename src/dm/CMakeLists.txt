file(GLOB C_SOURCES "src/*.c")
add_library(dm STATIC ${C_SOURCES})

target_link_libraries(dm iot_sdk)

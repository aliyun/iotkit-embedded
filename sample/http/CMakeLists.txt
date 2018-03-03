file(GLOB C_SOURCES "*.c")
add_executable(http-example ${C_SOURCES})
target_link_libraries(http-example iot_sdk)

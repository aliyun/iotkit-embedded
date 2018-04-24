include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl)

file(GLOB C_SOURCES "*.c")
add_executable(coap-example ${C_SOURCES})
target_link_libraries(coap-example iot_sdk)

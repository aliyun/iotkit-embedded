include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl)

add_executable(ota_mqtt-example ota_mqtt-example.c)
target_link_libraries(ota_mqtt-example iot_sdk)

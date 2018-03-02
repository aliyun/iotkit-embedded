include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl)

add_executable(mqtt-example mqtt-example.c)
target_link_libraries(mqtt-example iot_sdk)

add_executable(mqtt_rrpc-example mqtt_rrpc-example.c)
target_link_libraries(mqtt_rrpc-example iot_sdk)

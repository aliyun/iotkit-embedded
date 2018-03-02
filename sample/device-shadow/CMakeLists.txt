include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl)

add_executable(shadow-example shadow-example.c)
target_link_libraries(shadow-example iot_sdk)

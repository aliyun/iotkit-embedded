include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/LITE-utils)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/LITE-log)

add_executable(subdev-example subdev_example_api.c subdev-example.c)
target_link_libraries(subdev-example iot_sdk)

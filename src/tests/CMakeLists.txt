include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl)
include_directories(${PROJECT_SOURCE_DIR}/src/)
include_directories(${PROJECT_SOURCE_DIR}/src/tfs)

add_executable(sdk-testsuites sdk-testsuites.c)
target_link_libraries(sdk-testsuites iot_sdk)

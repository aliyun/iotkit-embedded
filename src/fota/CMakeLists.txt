include_directories(${PROJECT_SOURCE_DIR}/src/dm/include)
include_directories(${PROJECT_SOURCE_DIR}/src/fota/include)

file(GLOB C_SOURCES "src/*.c")
add_library(fota STATIC ${C_SOURCES})

target_link_libraries(fota iot_sdk)


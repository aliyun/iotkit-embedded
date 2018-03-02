include_directories(${PROJECT_SOURCE_DIR}/src/packages/LITE-utils)

file(GLOB C_SOURCES "*.c")
add_library(utils_misc OBJECT ${C_SOURCES})


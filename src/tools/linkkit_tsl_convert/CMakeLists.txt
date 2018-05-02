include_directories(${PROJECT_SOURCE_DIR}/tool/linkkit_tsl_convert/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/tool/linkkit_tsl_convert/src SRC_TSL_FORMAT)

add_library(tsl_format STATIC ${SRC_TSL_FORMAT})

add_executable(linkkit_tsl_convert linkkit_tsl_convert.c)
target_link_libraries(linkkit_tsl_convert tsl_format)

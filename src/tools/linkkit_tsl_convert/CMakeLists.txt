INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/.O/usr/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/include/itls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/include/tfs)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/src)

ADD_EXECUTABLE (linkkit_tsl_convert
    ${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/linkkit_tsl_convert.c
    ${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/src/tsl_opt_impl.c
    ${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/src/tsl_file.c
    ${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/src/cJSON.c
    ${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/src/tsl_format_export.c
)

TARGET_LINK_LIBRARIES (linkkit_tsl_convert pthread)


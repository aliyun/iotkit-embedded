include_directories(${PROJECT_SOURCE_DIR}/sample/linkkit/include)

set(LINKKIT_C_SOURCES src/linkkit_export.c src/lite_queue.c)
add_library(linkkit STATIC ${LINKKIT_C_SOURCES})
target_link_libraries(linkkit dm)
if(FEATURE_SERVICE_OTA_ENABLED)
    target_link_libraries(linkkit fota cota)
endif(FEATURE_SERVICE_OTA_ENABLED)

set(LINKKIT_SAMPLE_C_SOURCES samples/linkkit_sample.c )
add_executable(linkkit-example ${LINKKIT_SAMPLE_C_SOURCES})
target_link_libraries(linkkit-example linkkit)

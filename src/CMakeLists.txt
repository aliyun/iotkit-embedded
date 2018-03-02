include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/mbedtls-in-iotkit/include)
include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl/imports)
include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl/exports)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/LITE-log)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/LITE-utils)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/Link-MQTT)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/Link-OTA)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/iot-coap-c)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/iotkit-system)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/Link-CMP)
include_directories(${PROJECT_SOURCE_DIR}/src/utils/digest)
include_directories(${PROJECT_SOURCE_DIR}/src/utils/misc)
include_directories(${PROJECT_SOURCE_DIR}/src/guider)
include_directories(${PROJECT_SOURCE_DIR}/src/tfs)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/iot-coap-c)
include_directories(${PROJECT_SOURCE_DIR}/src/import/linux/include)

add_subdirectory(platform)
add_subdirectory(utils)
add_subdirectory(shadow)
add_subdirectory(coap)
add_subdirectory(packages)
add_subdirectory(http)
add_subdirectory(subdev)
#add_subdirectory(sdk-tests)

set(iot_sdk_c_sources $<TARGET_OBJECTS:iotkit_packages>
                      $<TARGET_OBJECTS:coap>
                      $<TARGET_OBJECTS:iot_http>
                      #$<TARGET_OBJECTS:platform_os_win7>
                      $<TARGET_OBJECTS:platform_ssl_mbedtls>
                      $<TARGET_OBJECTS:iot_shadow>
                      $<TARGET_OBJECTS:utils_digest>
                      $<TARGET_OBJECTS:utils_misc>
                      $<TARGET_OBJECTS:iot_subdev>
   )
if(WIN32)
    set(iot_sdk_c_sources ${iot_sdk_c_sources} $<TARGET_OBJECTS:platform_os_win7>)
else(WIN32)
    set(iot_sdk_c_sources ${iot_sdk_c_sources} $<TARGET_OBJECTS:platform_os_linux>)
endif(WIN32)

add_library(iot_sdk STATIC ${iot_sdk_c_sources})
if(WIN32)
    target_link_libraries(iot_sdk ws2_32)
endif(WIN32)

add_subdirectory(platform)
add_subdirectory(utils)
add_subdirectory(shadow)
add_subdirectory(coap)
add_subdirectory(packages)
add_subdirectory(http)
if(FEATURE_SUBDEVICE_ENABLED)
    add_subdirectory(subdev)
endif(FEATURE_SUBDEVICE_ENABLED)
if(FEATURE_CMP_ENABLED)
    if(FEATURE_SERVICE_OTA_ENABLED)
        add_subdirectory(fota)
        add_subdirectory(cota)
    endif(FEATURE_SERVICE_OTA_ENABLED)
    if(FEATURE_DM_ENABLED)
        add_subdirectory(dm)
    endif(FEATURE_DM_ENABLED)
endif(FEATURE_CMP_ENABLED)
#add_subdirectory(sdk-tests)

set(iot_sdk_c_sources $<TARGET_OBJECTS:iotkit_packages>
                      $<TARGET_OBJECTS:coap>
                      $<TARGET_OBJECTS:iot_http>
                      $<TARGET_OBJECTS:platform_ssl_mbedtls>
                      $<TARGET_OBJECTS:iot_shadow>
                      $<TARGET_OBJECTS:utils_digest>
                      $<TARGET_OBJECTS:utils_misc>)

if(FEATURE_SUBDEVICE_ENABLED)
    set(iot_sdk_c_sources ${iot_sdk_c_sources} $<TARGET_OBJECTS:iot_subdev>)
endif(FEATURE_SUBDEVICE_ENABLED)

if(WIN32)
    set(iot_sdk_c_sources ${iot_sdk_c_sources} $<TARGET_OBJECTS:platform_os_win7>)
else(WIN32)
    set(iot_sdk_c_sources ${iot_sdk_c_sources} $<TARGET_OBJECTS:platform_os_linux>)
endif(WIN32)

add_library(iot_sdk STATIC ${iot_sdk_c_sources})
if(WIN32)
    target_link_libraries(iot_sdk ws2_32)
endif(WIN32)

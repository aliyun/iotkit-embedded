if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    MESSAGE (FATAL_ERROR "in-tree build not allowded")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (iotkit-embedded-V2.10 C)

SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET (CMAKE_C_FLAGS "-DALCS_ENABLED -DCMP_ENABLED -DCMP_VIA_MQTT_DIRECT -DCM_VIA_CLOUD_CONN -DCM_VIA_CLOUD_CONN_MQTT -DCONFIG_GUIDER_AUTH_TIMEOUT=500 -DCONFIG_HTTP_AUTH_TIMEOUT=500 -DCONFIG_MID_HTTP_TIMEOUT=500 -DDM_ENABLED -DFORCE_SSL_VERIFY -DHTTP_COMM_ENABLED -DIOTX_WITHOUT_ITLS -DMQTT_COMM_ENABLED -DMQTT_DIRECT -DMQTT_DIRECT_NOITLS -DMQTT_SHADOW -DOTA_ENABLED -DOTA_SIGNAL_CHANNEL=1 -DSERVICE_OTA_ENABLED -DSUBDEV_VIA_MQTT -DUSING_UTILS_JSON -D_PLATFORM_IS_HOST_ -Os -Wall")

IF (WIN32)
    SET (CMAKE_EXE_LINKER_FLAGS "-L${PROJECT_SOURCE_DIR}/src/import/win7/libs")
ENDIF (WIN32)

MESSAGE ("---------------------------------------------------------------------")
MESSAGE ("Project Name              : " ${PROJECT_NAME})
MESSAGE ("Source Dir                : " ${PROJECT_SOURCE_DIR})
MESSAGE ("Binary Dir                : " ${PROJECT_BINARY_DIR})
MESSAGE ("System Processor          : " ${CMAKE_SYSTEM_PROCESSOR})
MESSAGE ("System Platform           : " ${CMAKE_SYSTEM})
MESSAGE ("C Compiler                : " ${CMAKE_C_COMPILER})
MESSAGE ("Executable Dir            : " ${EXECUTABLE_OUTPUT_PATH})
MESSAGE ("Library Dir               : " ${LIBRARY_OUTPUT_PATH})
MESSAGE ("SDK Version               : " V2.10)
MESSAGE ("")

if(WIN32)
    MESSAGE ("Building on WINDOWS ...")
else(WIN32)
    MESSAGE ("Building on LINUX ...")
endif(WIN32)
MESSAGE ("---------------------------------------------------------------------")

FILE (REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/../iotx-sdk-c.pkgs)
FILE (MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/../iotx-sdk-c.pkgs)



ADD_SUBDIRECTORY (src/utils)
ADD_SUBDIRECTORY (src/system)
ADD_SUBDIRECTORY (src/framework/services/linkkit)
ADD_SUBDIRECTORY (src/framework/connectivity/mqtt)
ADD_SUBDIRECTORY (src/framework/services/ota)
ADD_SUBDIRECTORY (src/framework/services/shadow)
ADD_SUBDIRECTORY (src/framework/connectivity/http)
ADD_SUBDIRECTORY (src/framework/connectivity/alcs)
ADD_SUBDIRECTORY (src/framework/cm)
ADD_SUBDIRECTORY (src/framework/dm)
ADD_SUBDIRECTORY (src/framework/services/fota)
ADD_SUBDIRECTORY (src/framework/services/cota)

ADD_SUBDIRECTORY (src/platform)
ADD_SUBDIRECTORY (samples)
ADD_SUBDIRECTORY (src/tests)
ADD_SUBDIRECTORY (src/external/tls)
ADD_SUBDIRECTORY (src/tools/linkkit_tsl_convert)

ADD_LIBRARY (iot_sdk STATIC
    $<TARGET_OBJECTS:iot_utils>
    $<TARGET_OBJECTS:iot_system>
    $<TARGET_OBJECTS:iot_linkkit>
    $<TARGET_OBJECTS:iot_mqtt>
    $<TARGET_OBJECTS:iot_ota>
    $<TARGET_OBJECTS:iot_shadow>
    $<TARGET_OBJECTS:iot_http>
    $<TARGET_OBJECTS:iot_alcs>
    $<TARGET_OBJECTS:iot_cmp>
    $<TARGET_OBJECTS:iot_dm>
    $<TARGET_OBJECTS:iot_fota>
    $<TARGET_OBJECTS:iot_cota>
)
if(WIN32)
    TARGET_LINK_LIBRARIES (iot_sdk ws2_32)
endif(WIN32)


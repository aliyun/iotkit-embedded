########################################################################
# prevent in-tree builds
########################################################################
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "not allowded in-tree build")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

########################################################################
# project setup
########################################################################
cmake_minimum_required(VERSION 2.8)
project(iotx-sdk-c)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")
include(iotx-sdk-version)

########################################################################
# options
########################################################################
option(FEATURE_GIT_CLONE_BEFORE_BUILD     "git clone repos in packages folder when cmake configs"    ON)
option(FEATURE_MQTT_COMM_ENABLED          "MQTT communication enabled or not"                        ON)
option(FEATURE_MQTT_DIRECT                "MQTT direct connection enabled or not"                    ON)
option(FEATURE_MQTT_DIRECT_NOTLS          "MQTT direct connection w/o tls enabled or not"           OFF)
option(FEATURE_COAP_COMM_ENABLED          "coap communication enabled or not"                        ON)
option(FEATURE_HTTP_COMM_ENABLED          "HTTP communication enabled or not"                        ON)
option(FEATURE_MQTT_SHADOW                "MQTT shadow enabled or not"     ${FEATURE_MQTT_COMM_ENABLED})
option(FEATURE_COAP_DTLS_SUPPORT          "coap w/ dtls support or not"    ${FEATURE_COAP_COMM_ENABLED})
option(FEATURE_SUBDEVICE_ENABLED          "subdev enabled or not"                                   OFF)
option(FEATURE_CLOUD_CONN_ENABLED         "cloud connection enabled or not"                         OFF)
option(FEATURE_CMP_ENABLED                "cmp enabled or not"                                       ON)
option(FEATURE_DM_ENABLED                 "dm & linkkit enabled or not"                              ON)
option(FEATURE_SERVICE_OTA_ENABLED        "ota enabled or not"                                       ON)
option(FEATURE_SERVICE_COTA_ENABLED       "config ota enabled or not"                               OFF)
option(FEATURE_SUPPORT_PRODUCT_SECRET     "support via product_secret get device_secret"            OFF)
#option(FEATURE_OTA_FETCH_CHANNEL         "specify ota fetch channel"                                ON)HTTP
#option(FEATURE_OTA_SIGNAL_CHANNEL        "specify ota signal channel"                               ON)MQTT
set(GCC_ARCH native CACHE STRING "GCC compile for specific architecture.")

########################################################################
# Compiler specific setup
########################################################################
add_definitions(-DOTA_SIGNAL_CHANNEL=1)
add_definitions(-DFORCE_SSL_VERIFY)
add_definitions(-DUSING_UTILS_JSON)
add_definitions(-DLITE_THING_MODEL)

if(FEATURE_MQTT_DIRECT)
    add_definitions(-DMQTT_DIRECT)
endif(FEATURE_MQTT_DIRECT)

add_definitions(-DUSING_SHA1_IN_HMAC)

if(FEATURE_MQTT_COMM_ENABLED)
    add_definitions(-DMQTT_COMM_ENABLED)
endif(FEATURE_MQTT_COMM_ENABLED)

if(FEATURE_SUBDEVICE_ENABLED)
    add_definitions(-DSUBDEVICE_ENABLED)
endif(FEATURE_SUBDEVICE_ENABLED)

if(FEATURE_CMP_ENABLED)
    add_definitions(-DCMP_ENABLED)
endif(FEATURE_CMP_ENABLED)
add_definitions(-DCMP_SUPPORT_TOPIC_DISPATCH)

if(FEATURE_DM_ENABLED)
    add_definitions(-DDM_ENABLED)
    add_definitions(-DDEVICEINFO_ENABLED)
endif(FEATURE_DM_ENABLED)

if(FEATURE_SERVICE_OTA_ENABLED)
    add_definitions(-DSERVICE_OTA_ENABLED)
    if(FEATURE_SERVICE_COTA_ENABLED)
        add_definitions(-DSERVICE_COTA_ENABLED)
    endif(FEATURE_SERVICE_COTA_ENABLED)
endif(FEATURE_SERVICE_OTA_ENABLED)

if(FEATURE_SUPPORT_PRODUCT_SECRET)
    add_definitions(-DSUPPORT_PRODUCT_SECRET)
endif(FEATURE_SUPPORT_PRODUCT_SECRET)

add_definitions(-DIOTX_WITHOUT_ITLS)
add_definitions(-DIOTX_NET_INIT_WITH_PK_EXT)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=${GCC_ARCH} -Wall -Wno-comment -Wno-write-strings -Wno-format-extra-args -Winline -Wno-unused-result -Wno-format")
########################################################################
# add -fPIC property to all targets
########################################################################
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

########################################################################
# print project summary
########################################################################
message(STATUS "---------------------------------------------")
message(STATUS "project name：\t" ${PROJECT_NAME})
message(STATUS "source dir：\t" ${PROJECT_SOURCE_DIR})
message(STATUS "binary dir：\t" ${PROJECT_BINARY_DIR})
message(STATUS "system processor:\t" ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "c compiler:\t" ${CMAKE_C_COMPILER})
message(STATUS "system platform:\t" ${CMAKE_SYSTEM})
message(STATUS "c compiler options:\t" ${CMAKE_C_FLAGS})

if(WIN32)
    message(STATUS "windows compiling...")
    add_definitions(-D_PLATFORM_IS_WINDOWS_)
else(WIN32)
    message(STATUS "linux compiling...")
    add_definitions( -D_PLATFORM_IS_LINUX_)
endif(WIN32)
message(STATUS "iotx sdk version:\t" ${iotx_sdk_version})
message(STATUS "---------------------------------------------")

########################################################################
# git clone integrated repos
########################################################################
if(FEATURE_GIT_CLONE_BEFORE_BUILD)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/LITE-log)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/mbedtls-in-iotkit)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/iotkit-system)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/Link-MQTT)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/Link-OTA)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/Link-CMP)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/LITE-log.git ${PROJECT_SOURCE_DIR}/src/packages/LITE-log)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/mbedtls-in-iotkit.git ${PROJECT_SOURCE_DIR}/src/packages/mbedtls-in-iotkit)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/iotkit-system.git ${PROJECT_SOURCE_DIR}/src/packages/iotkit-system)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/Link-MQTT.git ${PROJECT_SOURCE_DIR}/src/packages/Link-MQTT)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/Link-OTA.git ${PROJECT_SOURCE_DIR}/src/packages/Link-OTA)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/Link-CMP.git ${PROJECT_SOURCE_DIR}/src/packages/Link-CMP)
endif(FEATURE_GIT_CLONE_BEFORE_BUILD)

include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl)
include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl/imports)
include_directories(${PROJECT_SOURCE_DIR}/src/sdk-impl/exports)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/mbedtls-in-iotkit/include)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/LITE-log)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/LITE-utils)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/Link-MQTT)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/Link-OTA)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/Link-CMP/inc)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/iot-coap-c)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/iotkit-system)
include_directories(${PROJECT_SOURCE_DIR}/src/packages/iot-coap-c)
include_directories(${PROJECT_SOURCE_DIR}/src/utils/digest)
include_directories(${PROJECT_SOURCE_DIR}/src/utils/misc)
include_directories(${PROJECT_SOURCE_DIR}/src/tfs)
if(FEATURE_SUBDEVICE_ENABLED)
include_directories(${PROJECT_SOURCE_DIR}/src/subdev)
endif(FEATURE_SUBDEVICE_ENABLED)
if(FEATURE_DM_ENABLED)
include_directories(${PROJECT_SOURCE_DIR}/src/dm/include)
endif(FEATURE_DM_ENABLED)
include_directories(${PROJECT_SOURCE_DIR}/src/import/linux/include)

########################################################################
# Add the subdirectories
########################################################################
add_subdirectory(src)
add_subdirectory(sample)

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
option(FEATURE_MQTT_DIRECT_NOTLS          "MQTT direct connection w/o tls enabled or not"            ON)
option(FEATURE_COAP_COMM_ENABLED          "coap communication enabled or not"                        ON)
option(FEATURE_HTTP_COMM_ENABLED          "HTTP communication enabled or not"                        ON)
option(FEATURE_MQTT_SHADOW                "MQTT shadow enabled or not"     ${FEATURE_MQTT_COMM_ENABLED})
option(FEATURE_COAP_DTLS_SUPPORT          "coap w/ dtls support or not"    ${FEATURE_COAP_COMM_ENABLED})
option(FEATURE_OTA_ENABLED                "ota enabled or not"                                       ON)
option(FEATURE_SUBDEVICE_ENABLED          "subdev enabled or not"                                    ON)
#option(FEATURE_OTA_FETCH_CHANNEL         "specify ota fetch channel"                                ON)HTTP
#option(FEATURE_OTA_SIGNAL_CHANNEL        "specify ota signal channel"                               ON)MQTT
set(GCC_ARCH native CACHE STRING "GCC compile for specific architecture.")

########################################################################
# Compiler specific setup
########################################################################
add_definitions(-DOTA_SIGNAL_CHANNEL=1)
add_definitions(-DFORCE_SSL_VERIFY)

if(FEATURE_MQTT_DIRECT)
    add_definitions(-DMQTT_DIRECT)
endif(FEATURE_MQTT_DIRECT)

if(FEATURE_MQTT_COMM_ENABLED)
    add_definitions(-DMQTT_COMM_ENABLED)
endif(FEATURE_MQTT_COMM_ENABLED)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=${GCC_ARCH} -Wall -Wno-comment -Wno-write-strings -Wno-format-extra-args -Winline -Wno-unused-result -Wno-format")
########################################################################
# add -fPIC property to all targets
########################################################################
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

########################################################################
# print project summary
########################################################################
message(STATUS "---------------------------------------------")
message(STATUS "project name：" ${PROJECT_NAME})
message(STATUS "source dir：" ${PROJECT_SOURCE_DIR})
message(STATUS "binary dir：" ${PROJECT_BINARY_DIR})
message(STATUS "system processor: " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "c compiler: " ${CMAKE_C_COMPILER})
message(STATUS "system platform: " ${CMAKE_SYSTEM})
message(STATUS "c compiler options: " ${CMAKE_C_FLAGS})
if(WIN32)
    message(STATUS "windows compiling...")
    add_definitions(-D_PLATFORM_IS_WINDOWS_)
else(WIN32)
    message(STATUS "linux compiling...")
    add_definitions( -D_PLATFORM_IS_LINUX_)
endif(WIN32)
message(STATUS "iotx sdk version: " ${iotx_sdk_version})
message(STATUS "---------------------------------------------")

########################################################################
# git clone integrated repos
########################################################################
if(FEATURE_GIT_CLONE_BEFORE_BUILD)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/LITE-log)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/LITE-utils)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/mbedtls-in-iotkit)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/iotkit-system)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/Link-MQTT)
file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/src/packages/Link-OTA)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/LITE-log.git ${PROJECT_SOURCE_DIR}/src/packages/LITE-log)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/LITE-utils.git ${PROJECT_SOURCE_DIR}/src/packages/LITE-utils)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/mbedtls-in-iotkit.git ${PROJECT_SOURCE_DIR}/src/packages/mbedtls-in-iotkit)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/iotkit-system.git ${PROJECT_SOURCE_DIR}/src/packages/iotkit-system)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/Link-MQTT.git ${PROJECT_SOURCE_DIR}/src/packages/Link-MQTT)
execute_process(COMMAND git clone ${PROJECT_SOURCE_DIR}/src/packages/Link-OTA.git ${PROJECT_SOURCE_DIR}/src/packages/Link-OTA)
endif(FEATURE_GIT_CLONE_BEFORE_BUILD)

########################################################################
# Add the subdirectories
########################################################################
add_subdirectory(src)
add_subdirectory(sample)

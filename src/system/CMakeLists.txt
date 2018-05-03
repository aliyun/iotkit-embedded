INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/.O/usr/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/board)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit/include/mbedtls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit/library)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/cm)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/cm/inc)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/cm/src)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/connectivity)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/connectivity/coap)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/connectivity/http)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/connectivity/iot-coap-c)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/connectivity/mqtt)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/connectivity/mqtt/Link-MQTT)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/connectivity/mqtt/Link-MQTT/MQTTPacket)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/dm)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/dm/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/dm/include/interface)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/dm/src)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/services)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/services/cota)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/services/fota)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/services/linkkit)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/services/linkkit/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/services/linkkit/src)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/services/ota)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/services/ota/Link-OTA)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/services/ota/Link-OTA/src)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/services/shadow)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/services/subdev)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/mdm9206)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/mdm9206/example)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/mdm9206/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/include/itls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/include/tfs)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/libs)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/win7)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/win7/32bit-libs)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/win7/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/win7/include/itls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/win7/include/tfs)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/packages)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/os)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/os/ubuntu)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/os/win7)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/ssl)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/ssl/mbedtls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/ssl/openssl)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/sdk-impl)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/sdk-impl/exports)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/sdk-impl/imports)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/system)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/system/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/system/src)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/system/tfs)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/system/tfs/platform)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tests)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tools)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/src)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils/digest)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils/log)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils/misc)

ADD_LIBRARY (iot_system OBJECT
    ${PROJECT_SOURCE_DIR}/src/system/src/id2_guider.c
    ${PROJECT_SOURCE_DIR}/src/system/src/class_interface.c
    ${PROJECT_SOURCE_DIR}/src/system/src/sdk-impl.c
    ${PROJECT_SOURCE_DIR}/src/system/src/guider.c
    ${PROJECT_SOURCE_DIR}/src/system/src/report.c
    ${PROJECT_SOURCE_DIR}/src/system/src/device.c
    ${PROJECT_SOURCE_DIR}/src/system/src/ca.c
    ${PROJECT_SOURCE_DIR}/src/system/tfs/tfs_demo.c
    ${PROJECT_SOURCE_DIR}/src/system/tfs/id2_crypto.c
    ${PROJECT_SOURCE_DIR}/src/system/tfs/tfs_uttest.c
    ${PROJECT_SOURCE_DIR}/src/system/tfs/platform/pal_platform_base64.c
    ${PROJECT_SOURCE_DIR}/src/system/tfs/platform/pal_platform_random.c
    ${PROJECT_SOURCE_DIR}/src/system/tfs/platform/pal_platform_log.c
    ${PROJECT_SOURCE_DIR}/src/system/tfs/platform/pal_platform_memory.c
    ${PROJECT_SOURCE_DIR}/src/system/tfs/platform/pal_platform_md5.c
    ${EXTRA_C_SOURCES}
)


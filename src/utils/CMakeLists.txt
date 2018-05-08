INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/board)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/cmp)
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
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs)
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
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/common)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/common/itls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/common/tfs)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/mdm9206)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/mdm9206/example)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/mdm9206/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/libs)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/win7)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/win7/32bit-libs)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/win7/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/win7/libs)
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
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/system/src)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/system/tfs)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/system/tfs/platform)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tests)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tools)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/tools/linkkit_tsl_convert/src)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils/digest)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils/log)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils/misc)

ADD_LIBRARY (iot_utils OBJECT
    ${PROJECT_SOURCE_DIR}/src/utils/json_token.c
    ${PROJECT_SOURCE_DIR}/src/utils/json_parser.c
    ${PROJECT_SOURCE_DIR}/src/utils/string_utils.c
    ${PROJECT_SOURCE_DIR}/src/utils/mem_stats.c
    ${PROJECT_SOURCE_DIR}/src/utils/lite-cjson.c
    ${PROJECT_SOURCE_DIR}/src/utils/lite-utils_testsuites.c
    ${PROJECT_SOURCE_DIR}/src/utils/log/lite-log.c
    ${PROJECT_SOURCE_DIR}/src/utils/misc/utils_list.c
    ${PROJECT_SOURCE_DIR}/src/utils/misc/utils_epoch_time.c
    ${PROJECT_SOURCE_DIR}/src/utils/misc/linked_list.c
    ${PROJECT_SOURCE_DIR}/src/utils/misc/utils_httpc.c
    ${PROJECT_SOURCE_DIR}/src/utils/misc/utils_timer.c
    ${PROJECT_SOURCE_DIR}/src/utils/misc/lite_queue.c
    ${PROJECT_SOURCE_DIR}/src/utils/misc/utils_net.c
    ${PROJECT_SOURCE_DIR}/src/utils/digest/utils_hmac.c
    ${PROJECT_SOURCE_DIR}/src/utils/digest/utils_md5.c
    ${PROJECT_SOURCE_DIR}/src/utils/digest/utils_sha256.c
    ${PROJECT_SOURCE_DIR}/src/utils/digest/utils_base64.c
    ${PROJECT_SOURCE_DIR}/src/utils/digest/utils_sha1.c
    ${EXTRA_C_SOURCES}
)


include_directories(${PROJECT_SOURCE_DIR}/import/ubuntu/include)

add_definitions(-DCMP_VIA_MQTT_DIRECT)

file(GLOB PACKAGES_CONNECTIVITY_SOURCES "Link-OTA/src/ota.c" "Link-MQTT/*.c" "Link-MQTT/MQTTPacket/*.c" "iotkit-system/src/*.c" "LITE-log/*.c" "LITE-utils/*.c" "mbedtls-in-iotkit/library/*.c" "iot-coap-c/*.c")

if(FEATURE_CMP_ENABLED)
    file(GLOB PACKAGES_CONNECTIVITY_SOURCES ${PACKAGES_CONNECTIVITY_SOURCES} "Link-CMP/src/*.c")
endif(FEATURE_CMP_ENABLED)

add_library(iotkit_packages OBJECT ${PACKAGES_CONNECTIVITY_SOURCES})

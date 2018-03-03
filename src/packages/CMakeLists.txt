include_directories(${PROJECT_SOURCE_DIR}/import/ubuntu/include)

file(GLOB PACKAGES_CONNECTIVITY_SOURCES "Link-CMP/src/*.c" "Link-OTA/src/ota.c" "Link-MQTT/*.c" "Link-MQTT/MQTTPacket/*.c" "iotkit-system/src/*.c" "LITE-log/*.c" "LITE-utils/*.c" "mbedtls-in-iotkit/library/*.c" "iot-coap-c/*.c")
add_library(iotkit_packages OBJECT ${PACKAGES_CONNECTIVITY_SOURCES})

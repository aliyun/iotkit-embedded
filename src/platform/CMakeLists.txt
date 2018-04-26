INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/.O/usr/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit/include/mbedtls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit/library)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/include/itls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/ubuntu/include/tfs)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/os)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/os/ubuntu)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/os/win7)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/ssl)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/ssl/mbedtls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/platform/ssl/openssl)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/sdk-impl/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/sdk-impl/exports)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/sdk-impl/imports)

ADD_LIBRARY (iot_platform STATIC
    ${PROJECT_SOURCE_DIR}/src/platform/os/ubuntu/HAL_OS_linux.c
    ${PROJECT_SOURCE_DIR}/src/platform/os/ubuntu/HAL_TCP_linux.c
    ${PROJECT_SOURCE_DIR}/src/platform/os/ubuntu/HAL_UDP_linux.c
    ${PROJECT_SOURCE_DIR}/src/platform/ssl/mbedtls/HAL_DTLS_mbedtls.c
    ${PROJECT_SOURCE_DIR}/src/platform/ssl/mbedtls/HAL_TLS_mbedtls.c
)


IF (WIN32)
    SET (OS_DIR win7)
ELSE (WIN32)
    SET (OS_DIR ubuntu)
ENDIF (WIN32)
FILE (GLOB EXTRA_C_SOURCES ${PROJECT_SOURCE_DIR}/src/platform/os/${OS_DIR}/*.c)

INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/.O/usr/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit/include)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit/include/mbedtls)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/external/tls/mbedtls-in-iotkit/library)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import/include)
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
    ${PROJECT_SOURCE_DIR}/src/platform/ssl/mbedtls/HAL_DTLS_mbedtls.c
    ${PROJECT_SOURCE_DIR}/src/platform/ssl/mbedtls/HAL_TLS_mbedtls.c
    ${EXTRA_C_SOURCES}
)


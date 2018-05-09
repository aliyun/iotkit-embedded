INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/import)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/sdk-impl/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/sdk-impl/exports)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/sdk-impl/imports)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils/)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils/digest)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils/log)
INCLUDE_DIRECTORIES (${PROJECT_SOURCE_DIR}/src/utils/misc)

ADD_LIBRARY (iot_alcs STATIC
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/CoAPSerialize.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/CoAPObserve.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/alcs_api.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/CoAPPlatform.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/CoAPNetwork.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/CoAPExport.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/alcs_client.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/CoAPResource.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/CoAPServer.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/CoAPDeserialize.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/CoAPMessage.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/alcs_server.c
    ${PROJECT_SOURCE_DIR}/src/framework/connectivity/alcs/alcs_coap.c
    ${EXTRA_C_SOURCES}
)


if(FEATURE_CLOUD_CONN_ENABLED)
    add_definitions(-DSUBDEV_VIA_CLOUD_CONN)
endif(FEATURE_CLOUD_CONN_ENABLED)

file(GLOB C_SOURCES "*.c")
add_library(iot_subdev OBJECT ${C_SOURCES})

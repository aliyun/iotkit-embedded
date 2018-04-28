#! /bin/bash

TARGET_FILE=$1
PKG_RPATH=$(echo ${PACKAGE_DIR}|${SED} "s:${TOP_DIR}/*::g")

rm -f ${TARGET_FILE}

cat << EOB >> ${TARGET_FILE}
if(\${CMAKE_SOURCE_DIR} STREQUAL \${CMAKE_BINARY_DIR})
    MESSAGE (FATAL_ERROR "in-tree build not allowded")
endif(\${CMAKE_SOURCE_DIR} STREQUAL \${CMAKE_BINARY_DIR})

CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (${PRJ_NAME}-${PRJ_VERSION})

SET (EXECUTABLE_OUTPUT_PATH \${PROJECT_BINARY_DIR}/bin)
SET (LIBRARY_OUTPUT_PATH \${PROJECT_BINARY_DIR}/lib)
SET (CMAKE_C_FLAGS "${CFLAGS}")

MESSAGE ("---------------------------------------------------------------------")
MESSAGE ("Project Name              : " \${PROJECT_NAME})
MESSAGE ("Source Dir                : " \${PROJECT_SOURCE_DIR})
MESSAGE ("Binary Dir                : " \${PROJECT_BINARY_DIR})
MESSAGE ("System Processor          : " \${CMAKE_SYSTEM_PROCESSOR})
MESSAGE ("System Platform           : " \${CMAKE_SYSTEM})
MESSAGE ("C Compiler                : " \${CMAKE_C_COMPILER})
MESSAGE ("Executable Dir            : " \${EXECUTABLE_OUTPUT_PATH})
MESSAGE ("Library Dir               : " \${LIBRARY_OUTPUT_PATH})
MESSAGE ("SDK Version               : " ${PRJ_VERSION})
MESSAGE ("")

if(WIN32)
    MESSAGE ("Building on WINDOWS ...")
else(WIN32)
    MESSAGE ("Building on LINUX ...")
endif(WIN32)
MESSAGE ("---------------------------------------------------------------------")

FILE (REMOVE_RECURSE \${PROJECT_SOURCE_DIR}/../$(basename ${TOP_DIR}).pkgs)
FILE (MAKE_DIRECTORY \${PROJECT_SOURCE_DIR}/../$(basename ${TOP_DIR}).pkgs)

$(for i in $(grep 'PKG_SOURCE' \
                $(find ${TOP_DIR} -name ${MAKE_SEGMENT} -not -path "*.O*") \
                    | ${SED} "s:${TOP_DIR}/\(.*\)/${MAKE_SEGMENT}.*= *\(.*\):\1~\2:g"); do
    UNZIP_DIR=${i%%~*}
    GREPO_DIR=${i##*~}
    GREPO_PRE=${GREPO_DIR%%.git}

    echo "FILE (REMOVE_RECURSE \${PROJECT_SOURCE_DIR}/${UNZIP_DIR}/${GREPO_PRE})"
    echo "EXECUTE_PROCESS (COMMAND git clone -q \${PROJECT_SOURCE_DIR}/${PKG_RPATH}/${GREPO_DIR} \${PROJECT_SOURCE_DIR}/${UNZIP_DIR}/${GREPO_PRE})"
done)

EOB

if [ "${COMP_LIB_NAME}" != "" ]; then
cat << EOB >> ${TARGET_FILE}
$(for i in ${COMP_LIB_COMPONENTS}; do
    echo "ADD_SUBDIRECTORY (${i})"
done)

EOB

cat << EOB >> ${TARGET_FILE}
$(for i in ${SUBDIRS}; do
    if [ ! -d ${TOP_DIR}/${i} ]; then
        continue
    fi
    if echo ${COMP_LIB_COMPONENTS} | grep -qw ${i}; then
        continue
    fi
    echo "ADD_SUBDIRECTORY (${i})"
done)

EOB

cat << EOB >> ${TARGET_FILE}
ADD_LIBRARY (${COMP_LIB_NAME} STATIC
$(for i in ${SEP_LIBS}; do
    j=${i%.a}
    j=${j#lib}
    echo "    \$<TARGET_OBJECTS:${j}>"
done)
)

EOB
fi

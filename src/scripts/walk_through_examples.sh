#! /bin/bash
set -e

if [ "${OUTPUT_DIR}" = "" ] || [ ! -d ${OUTPUT_DIR} ]; then exit 1; fi

PROGS="./mqtt-example ./shadow-example"
cd ${OUTPUT_DIR}/usr/bin

echo ""
for iter in ${PROGS}; do
    if [ -f ${iter} ]; then
        ${iter}
    fi  
done

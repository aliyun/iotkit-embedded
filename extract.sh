#! /bin/bash

# Prepare Directory
OUTPUT_DIR=output
OUTPUT_TMPDIR=.O
INFRA_DIR=${OUTPUT_DIR}/eng/src/infra
DEV_SIGN_DIR=${OUTPUT_DIR}/eng/src/dev_sign

mkdir -p ${INFRA_DIR}
mkdir -p ${DEV_SIGN_DIR}

# Prepare Config Macro In make.settings
MACRO_LIST=$(sed -n '/=y/p' make.settings | sed -n 's/=y//gp' | sed -n 's/FEATURE_//gp')
CONFIG_H=$(echo "${MACRO_LIST}" | sed -n 's/^/#define /p')

# Function
gen_infra_default () {
    echo "#ifndef _INFRA_CONFIG_H_" > ${INFRA_DIR}/infra_config.h
    echo "#define _INFRA_CONFIG_H_" >> ${INFRA_DIR}/infra_config.h
    echo "$1" >> ${INFRA_DIR}/infra_config.h
    echo "#endif" >> ${INFRA_DIR}/infra_config.h
}

gen_infra_module() {
    find . -path ./${OUTPUT_DIR} -prune -type f -o -name "infra_types.h" | xargs -i cp -f {} ${INFRA_DIR}
    find . -path ./${OUTPUT_DIR} -prune -type f -o -name "infra_defs.[ch]" | xargs -i cp -f {} ${INFRA_DIR}
    find . -path ./${OUTPUT_DIR} -prune -type f -o -name "infra_compat.h" | xargs -i cp -f {} ${INFRA_DIR}
    [[ ${1} =~ "INFRA" ]] && find . \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname "${1}.[ch]" | xargs -i cp -f {} ${INFRA_DIR}
}

gen_dev_sign_module() {
    SRC_DEV_SIGN=$([[ ${1} =~ "DEV_SIGN" ]] && find . \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname ${1} -type d)
    if [ ${SRC_DEV_SIGN} ];then
        find ${SRC_DEV_SIGN} -maxdepth 1 -name *.[ch] | grep -v example | xargs -i cp -f {} ${DEV_SIGN_DIR}
        echo ${SRC_DEV_SIGN}
    fi
}

# echo "${MACRO_LIST}"
# echo "${CONFIG_H}"

# Generate infra_config.h
gen_infra_default "${CONFIG_H}"

echo "${MACRO_LIST}" | while read line;
do
    echo ${line}
    gen_infra_module ${line}
    gen_dev_sign_module ${line}
done
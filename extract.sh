#! /bin/bash

# Prepare Directory
OUTPUT_DIR=output
OUTPUT_TMPDIR=.O
INFRA_DIR=${OUTPUT_DIR}/eng/infra
WRAPPERS_DIR=${OUTPUT_DIR}/eng/wrappers
EXAMPLES_DIR=${OUTPUT_DIR}/eng/examples

gen_eng_dir() {
    rm -rf ${OUTPUT_DIR}/eng
    mkdir -p ${INFRA_DIR}
    mkdir -p ${EXAMPLES_DIR}
    mkdir -p ${WRAPPERS_DIR}
}

# Prepare Config Macro In make.settings
MACRO_LIST=$(sed -n '/=y/p' make.settings | sed -n 's/=y//gp' | sed -n 's/FEATURE_//gp')

# Function
gen_infra_default () {
    CONFIG_H=$(echo "${1}" | sed -n 's/^/#define /p')
    echo "#ifndef _INFRA_CONFIG_H_" > ${INFRA_DIR}/infra_config.h
    echo -e "#define _INFRA_CONFIG_H_\n" >> ${INFRA_DIR}/infra_config.h
    echo "${CONFIG_H}" >> ${INFRA_DIR}/infra_config.h
    echo -e "\n#endif" >> ${INFRA_DIR}/infra_config.h
}

gen_infra_module() {
    M_INFRA=$(echo "${1}" | grep 'INFRA')
    echo "extract infra module..."
    echo -e "${M_INFRA}\n"
    find . -path ./${OUTPUT_DIR} -prune -type f -o -name "infra_types.h" | xargs -i cp -f {} ${INFRA_DIR}
    find . -path ./${OUTPUT_DIR} -prune -type f -o -name "infra_defs.[ch]" | xargs -i cp -f {} ${INFRA_DIR}
    find . -path ./${OUTPUT_DIR} -prune -type f -o -name "infra_compat.h" | xargs -i cp -f {} ${INFRA_DIR}

    echo "${M_INFRA}" | while read line;
    do
        find . \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname "${line}.[ch]" | xargs -i cp -f {} ${INFRA_DIR}
    done
}

gen_dev_sign_module() {
    M_DEV_SIGN=$(echo "${1}" | grep -w 'DEV_SIGN')
    DEV_SIGN_DIR=${OUTPUT_DIR}/eng/dev_sign
    mkdir -p ${DEV_SIGN_DIR}
    echo "extract dev_sign module..."
    echo -e "${M_DEV_SIGN}\n"

    SRC_DEV_SIGN=$([[ ${M_DEV_SIGN} ]] && find ./src \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname ${M_DEV_SIGN} -type d)
    if [ ${SRC_DEV_SIGN} ];then
        find ${SRC_DEV_SIGN} -maxdepth 1 -name *.[ch] | grep -v example | xargs -i cp -f {} ${DEV_SIGN_DIR}
        find ${SRC_DEV_SIGN} -maxdepth 1 -name *example*.c | xargs -i cp -f {} ${EXAMPLES_DIR}
    fi
}

gen_mqtt_module() {
    M_MQTT_COMM_ENABLED=$(echo "${1}" | grep -w 'MQTT_COMM_ENABLED')
    M_MQTT_DEFAULT_IMPL=$(echo "${1}" | grep -w 'MQTT_DEFAULT_IMPL')
    M_MAL_ENABLED=$(echo "${1}" | grep -w 'MAL_ENABLED')
    M_MAL_ICA_ENABLED=$(echo "${1}" | grep -w 'MAL_ICA_ENABLED')

    MQTT_DIR=${OUTPUT_DIR}/eng/mqtt
    mkdir -p ${MQTT_DIR}
    echo "extract mqtt module..."
    echo -e "$(echo "${1}" | grep -E 'MQTT|MAL')\n"

    SRC_MQTT_SIGN=$([[ ${M_MQTT_COMM_ENABLED} ]] && find ./src \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname "mqtt" -type d)
    if [ ${SRC_MQTT_SIGN} ];then
        find ${SRC_MQTT_SIGN} -maxdepth 1 -name *.[ch] | grep -v example | xargs -i cp -f {} ${MQTT_DIR}
        [[ ${M_MQTT_DEFAULT_IMPL} ]] && find ${SRC_MQTT_SIGN} -name mqtt_impl -type d | xargs -i cp -rf {} ${MQTT_DIR}
        [[ ${M_MQTT_DEFAULT_IMPL} ]] && find ${SRC_MQTT_SIGN} -maxdepth 1 -name *example.c | xargs -i cp -f {} ${EXAMPLES_DIR}
    fi

    SRC_MAL_WRAPPER=$([[ ${M_MAL_ENABLED} ]] && find ./wrappers \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname "mal" -type d)
    if [ ${SRC_MAL_WRAPPER} ];then
        mkdir -p ${WRAPPERS_DIR}/mqtt/mal
        find ${SRC_MAL_WRAPPER} -maxdepth 1 -name *.[ch] | grep -v example | xargs -i cp -f {} ${WRAPPERS_DIR}/mqtt/mal
        [[ ${M_MAL_ICA_ENABLED} ]] && find ${SRC_MAL_WRAPPER} -name ica -type d | xargs -i cp -rf {} ${WRAPPERS_DIR}/mqtt/mal
    fi
}

# Generate Directory
gen_eng_dir

# Generate infra_config.h
gen_infra_default "${MACRO_LIST}"

# Generate Module Code
gen_infra_module "${MACRO_LIST}"
gen_dev_sign_module "${MACRO_LIST}"
gen_mqtt_module "${MACRO_LIST}"
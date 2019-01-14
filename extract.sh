#! /bin/bash

# Prepare Directory
OUTPUT_DIR=output
OUTPUT_TMPDIR=.O
INFRA_DIR=${OUTPUT_DIR}/eng/infra
WRAPPERS_DIR=${OUTPUT_DIR}/eng/wrappers
EXAMPLES_DIR=${OUTPUT_DIR}/eng/examples
WRAPPER_DOC=./tools/misc/wrapper

MODULES=( \
"gen_infra" \
"gen_dev_sign" \
"gen_mqtt" \
"gen_sal" \
"gen_dynreg" \
"gen_atparser" \
"gen_dev_model" \
)

gen_eng_dir() {
    rm -rf ${OUTPUT_DIR}/eng
    mkdir -p ${INFRA_DIR}
    mkdir -p ${EXAMPLES_DIR}
    mkdir -p ${WRAPPERS_DIR}
    echo "" > ${WRAPPERS_DIR}/wrapper.c
}

# Prepare Config Macro In make.settings
MACRO_LIST=$(sed -n '/#/!{/=y/p}' make.settings | sed -n 's/=y//gp' | sed -n 's/FEATURE_//gp')

# Function
gen_infra_default () {
    CONFIG_H=$(echo "${1}" | sed -n 's/^/#define /p')
    echo "#ifndef _INFRA_CONFIG_H_" > ${INFRA_DIR}/infra_config.h
    echo -e "#define _INFRA_CONFIG_H_\n" >> ${INFRA_DIR}/infra_config.h
    echo "${CONFIG_H}" >> ${INFRA_DIR}/infra_config.h
    echo -e "\n#endif" >> ${INFRA_DIR}/infra_config.h
}

gen_infra() {
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

gen_wrapper_c() {
    M_MQTT_COMM_ENABLED=$(echo "${1}" | grep -w 'MQTT_COMM_ENABLED')
    M_MQTT_DEFAULT_IMPL=$(echo "${1}" | grep -w 'MQTT_DEFAULT_IMPL')
    M_MAL_ENABLED=$(echo "${1}" | grep -w 'MAL_ENABLED')

    WRAPPER_FUNCS=$(find ./${OUTPUT_DIR}/eng -name *wrapper.h | xargs -i grep -ro "HAL_.*(" {} | sed 's/(//g' | sort -u)"\n"
    WRAPPER_FUNCS+=$(find ./${OUTPUT_DIR}/eng -name *wrapper.h | xargs -i grep -ro "wrapper_.*(" {} | sed 's/(//g' | sort -u)"\n"

    [[ ${M_MQTT_COMM_ENABLED} && ( ${M_MQTT_DEFAULT_IMPL} || ${M_MAL_ENABLED} ) ]] && \
    WRAPPER_FUNCS=$(echo -e "${WRAPPER_FUNCS}" | sed -n '/wrapper_mqtt/!{p}')

    # ATM wrapper/HAL
    M_ATM_ENABLED=$(echo "${1}" | grep -w 'ATM_ENABLED')
    M_AT_PARSER_ENABLED=$(echo "${1}" | grep 'AT_PARSER_ENABLED')
    M_AT_TCP_ENABLED=$(echo "${1}" | grep 'AT_TCP_ENABLED')
    M_AT_MQTT_ENABLED=$(echo "${1}" | grep 'AT_MQTT_ENABLED')
    M_AT_TCP_HAL_MK3060=$(echo "${1}" | grep 'AT_TCP_HAL_MK3060')
    M_AT_TCP_HAL_SIM800=$(echo "${1}" | grep 'AT_TCP_HAL_SIM800')
    M_AT_MQTT_HAL_ICA=$(echo "${1}" | grep 'AT_MQTT_HAL_ICA')

    [[ ${M_AT_PARSER_ENABLED} ]] && WRAPPER_FUNCS+="\n"$(find src/atm/at_wrapper.h | xargs -i grep -ro "HAL_AT_Uart_.*(" {} | sed 's/(//g' | sort -u)"\n"
    [[ ${M_AT_TCP_ENABLED} ]] && WRAPPER_FUNCS+=$(find src/atm/at_wrapper.h | xargs -i grep -ro "HAL_AT_CONN_.*(" {} | sed 's/(//g' | sort -u)"\n"
    [[ ${M_AT_MQTT_ENABLED} ]] && WRAPPER_FUNCS+=$(find src/atm/at_wrapper.h | xargs -i grep -ro "HAL_MDAL_MAL_.*(" {} | sed 's/(//g' | sort -u)"\n"
    [[ ${M_AT_TCP_HAL_MK3060} ]] && WRAPPER_FUNCS+=$(find src/atm/at_wrapper.h | xargs -i grep -ro "HAL_AT_Uart_.*(" {} | sed 's/(//g' | sort -u)"\n"
    [[ ${M_AT_TCP_HAL_SIM800} ]] && WRAPPER_FUNCS+=$(find src/atm/at_wrapper.h | xargs -i grep -ro "HAL_AT_Uart_.*(" {} | sed 's/(//g' | sort -u)"\n"
    [[ ${M_AT_MQTT_HAL_ICA} ]] && WRAPPER_FUNCS+=$(find src/atm/at_wrapper.h | xargs -i grep -ro "HAL_AT_Uart_.*(" {} | sed 's/(//g' | sort -u)"\n"

    WRAPPER_FUNCS=$(echo -e "${WRAPPER_FUNCS}" | sort -u)

    # echo -e "${WRAPPER_FUNCS}"

    cp src/atm/at_wrapper.h ${OUTPUT_DIR}/eng/atm/

    cp -f wrappers/wrappers_defs.h ${WRAPPERS_DIR}/

    echo -e "#include \"infra_types.h\"" >> ${WRAPPERS_DIR}/wrapper.c
    echo -e "#include \"infra_defs.h\"" >> ${WRAPPERS_DIR}/wrapper.c
    echo -e "#include \"wrappers_defs.h\"" >> ${WRAPPERS_DIR}/wrapper.c
    find ./output/eng -name *wrapper.h | sed -n 's/.*\//#include "/p' | sed -n 's/$/"/p' >> ${WRAPPERS_DIR}/wrapper.c
    [[ ${M_MQTT_COMM_ENABLED} && ! ${M_MQTT_DEFAULT_IMPL} ]] && echo -e "#include \"mqtt_api.h\"" >> ${WRAPPERS_DIR}/wrapper.c
    echo -e "\n" >> ${WRAPPERS_DIR}/wrapper.c

    # echo -e "${WRAPPER_FUNCS}" |awk '{ printf("%03d %s\n", NR, $0); }'

    sed -n  '/WRAPPER_NOTE:/{:a;N;/*\//!ba;p}' ${WRAPPER_DOC} | sed -n '1d;p' >> ${WRAPPERS_DIR}/wrapper.c
    echo -e "\n" >> ${WRAPPERS_DIR}/wrapper.c

    echo -e "${WRAPPER_FUNCS}" | while read func
    do
        [[ ! ${func} ]] && continue

        FUNC_DEC=$(find ./${OUTPUT_DIR}/eng -name *wrapper.h | xargs -i cat {} 2>/dev/null | sed -n '/.*'$func'(.*/{/.*);/ba;{:c;N;/.*);/!bc};:a;p;q}')
        DATA_TYPE=$(echo "${FUNC_DEC}" | head -1 | awk -F'wrapper|HAL' '{print $1}' | sed s/[[:space:]]//g)
        # echo "${func}"

        sed -n '/'${func}':/{:a;N;/*\//!ba;p}' ${WRAPPER_DOC} | sed -n '1d;p' >> ${WRAPPERS_DIR}/wrapper.c

        if [ "${DATA_TYPE}" == "void" ];then
            echo "${FUNC_DEC}" | sed -n '/;/{s/;/\n{\n\treturn;\n}\n\n/g};p' >> ${WRAPPERS_DIR}/wrapper.c
        else
            echo "${FUNC_DEC}" | sed -n '/;/{s/;/\n{\n\treturn ('${DATA_TYPE}')1;\n}\n\n/g};p' >> ${WRAPPERS_DIR}/wrapper.c
        fi
    done

}

gen_dev_sign() {
    M_DEV_SIGN=$(echo "${1}" | grep -w 'DEV_SIGN')
    [[ ! ${M_DEV_SIGN} ]] && return

    echo "extract dev_sign module..."
    echo -e "${M_DEV_SIGN}\n"

    SRC_DEV_SIGN=$([[ ${M_DEV_SIGN} ]] && find ./src \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname ${M_DEV_SIGN} -type d)
    [[ ! ${SRC_DEV_SIGN} ]] &&return

    DEV_SIGN_DIR=$(echo "${SRC_DEV_SIGN}" | sed -n 's/.*\///p' | sed -n 's/^/'${OUTPUT_DIR}'\/eng\//p')
    mkdir -p ${DEV_SIGN_DIR}

    find ${SRC_DEV_SIGN} -maxdepth 1 -name *.[ch] | grep -v example | xargs -i cp -f {} ${DEV_SIGN_DIR}
    find ${SRC_DEV_SIGN} -maxdepth 1 -name *example*.c | xargs -i cp -f {} ${EXAMPLES_DIR}
}

gen_dynreg() {
    M_DYNREG=$(echo "${1}" | grep -w 'DYNAMIC_REGISTER')
    [[ ! ${M_DYNREG} ]] && return

    echo "extract dynamic_register module..."
    echo -e "${M_DYNREG}\n"

    SRC_DYNREG=$(find ./src \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname ${M_DYNREG} -type d)
    [[ ! ${SRC_DYNREG} ]] && return

    DYNREG_DIR=$(echo "${SRC_DYNREG}" | sed -n 's/.*\///p' | sed -n 's/^/'${OUTPUT_DIR}'\/eng\//p')
    mkdir -p ${DYNREG_DIR}

    find ${SRC_DYNREG} -maxdepth 1 -name *.[ch] | grep -v example | xargs -i cp -f {} ${DYNREG_DIR}
    find ${SRC_DYNREG} -maxdepth 1 -name *example*.c | xargs -i cp -f {} ${EXAMPLES_DIR}
    
}

gen_mqtt() {
    M_MQTT_COMM_ENABLED=$(echo "${1}" | grep -w 'MQTT_COMM_ENABLED')
    M_MQTT_DEFAULT_IMPL=$(echo "${1}" | grep -w 'MQTT_DEFAULT_IMPL')
    M_MAL_ENABLED=$(echo "${1}" | grep -w 'MAL_ENABLED')
    M_MAL_ICA_ENABLED=$(echo "${1}" | grep -w 'MAL_ICA_ENABLED')

    [[ ! ${M_MQTT_COMM_ENABLED} ]] && return

    echo "extract mqtt module..."
    echo -e "$(echo "${1}" | grep -E 'MQTT|MAL')\n"

    # extract src/mqtt
    SRC_MQTT_SIGN=$([[ ${M_MQTT_COMM_ENABLED} ]] && find ./src \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname "mqtt" -type d)
    [[ ! ${SRC_MQTT_SIGN} ]] && return

    mkdir -p ${OUTPUT_DIR}/eng/mqtt

    find ${SRC_MQTT_SIGN} -maxdepth 1 -name *.[ch] | grep -v example | xargs -i cp -f {} ${OUTPUT_DIR}/eng/mqtt
    [[ ${M_MQTT_DEFAULT_IMPL} ]] && find ${SRC_MQTT_SIGN} -name mqtt_impl -type d | xargs -i cp -rf {} ${OUTPUT_DIR}/eng/mqtt
    [[ ${M_MQTT_DEFAULT_IMPL} ]] && find ${SRC_MQTT_SIGN} -maxdepth 1 -name *example.c | xargs -i cp -f {} ${EXAMPLES_DIR}

    # extract wrappers/mqtt/mal
    SRC_MAL_WRAPPER=$([[ ${M_MAL_ENABLED} ]] && find ./wrappers \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname "mal" -type d)
    [[ ! ${SRC_MAL_WRAPPER} ]] && return

    mkdir -p ${WRAPPERS_DIR}/mqtt/mal

    find ${SRC_MAL_WRAPPER} -maxdepth 1 -name *.[ch] | grep -v example | xargs -i cp -f {} ${WRAPPERS_DIR}/mqtt/mal
    [[ ${M_MAL_ICA_ENABLED} ]] && find ${SRC_MAL_WRAPPER} -name ica -type d | xargs -i cp -rf {} ${WRAPPERS_DIR}/mqtt/mal
}

gen_sal() {
    M_ATM_ENABLED=$(echo "${1}" | grep -w 'ATM_ENABLED')
    M_AT_PARSER_ENABLED=$(echo "${1}" | grep 'AT_PARSER_ENABLED')
    M_AT_TCP_ENABLED=$(echo "${1}" | grep 'AT_TCP_ENABLED')
    M_AT_MQTT_ENABLED=$(echo "${1}" | grep 'AT_MQTT_ENABLED')
    M_AT_TCP_HAL_MK3060=$(echo "${1}" | grep 'AT_TCP_HAL_MK3060')
    M_AT_TCP_HAL_SIM800=$(echo "${1}" | grep 'AT_TCP_HAL_SIM800')
    M_AT_MQTT_HAL_ICA=$(echo "${1}" | grep 'AT_MQTT_HAL_ICA')

    [[ ! ${M_ATM_ENABLED} ]] && return

    echo "extract atm module..."
    echo -e "$(echo "${1}" | grep -E 'ATM|^AT')\n"

    # extract wrappers/sal and wrappers/at
    SRC_ATM=$(find ./src \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname "atm" -type d)
    [[ ! ${SRC_ATM} ]] && return

    mkdir -p ${OUTPUT_DIR}/eng/atm

    cp ${SRC_ATM}/at_api.[ch] ${OUTPUT_DIR}/eng/atm/
    [[ ${M_AT_PARSER_ENABLED} ]] && cp ${SRC_ATM}/at_parser.[ch] ${OUTPUT_DIR}/eng/atm/
    [[ ${M_AT_PARSER_ENABLED} ]] && cp ${SRC_ATM}/at_parser.[ch] ${OUTPUT_DIR}/eng/atm/
    [[ ${M_AT_TCP_ENABLED} ]] && cp ${SRC_ATM}/at_conn*.[ch] ${SRC_ATM}/at_tcp.c ${OUTPUT_DIR}/eng/atm/
    [[ ${M_AT_MQTT_ENABLED} ]] && cp ${SRC_ATM}/at_mqtt.[ch] ${OUTPUT_DIR}/eng/atm/
    [[ ${M_AT_TCP_HAL_SIM800} ]] && cp wrappers/atm//at_tcp/mk3060.c ${OUTPUT_DIR}/eng/atm/
    [[ ${M_AT_TCP_HAL_MK3060} ]] && cp wrappers/atm//at_tcp/sim800.c ${OUTPUT_DIR}/eng/atm/
    [[ ${M_AT_MQTT_HAL_ICA} ]] && cp wrappers/atm/at_mqtt/mdal􏰊_ica_at􏰃_c􏰊lient􏰇􏰃.c ${OUTPUT_DIR}/eng/atm/

    find ${SRC_SAL} -maxdepth 1 -name *.[ch] | grep -v example | xargs -i cp -f {} ${OUTPUT_DIR}/eng/sal/
}

gen_atparser() {
    M_ATPARSER_ENABLED=$(echo "${1}" | grep -w 'ATPARSER_ENABLED')

    [[ ! ${M_ATPARSER_ENABLED} ]] && return

    echo "extract atparser module..."
    echo -e "$(echo "${1}" | grep -E 'ATPARSER')\n"

    mkdir -p ${OUTPUT_DIR}/eng/sal/

    SRC_ATPARSER=$(find ./external_libs \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname "at" -type d)

    find ${SRC_ATPARSER} -maxdepth 1 -name *.[ch] | grep -v example | xargs -i cp -f {} ${OUTPUT_DIR}/eng/sal/
}

gen_dev_model() {
    M_DEVICE_MODEL_ENABLE=$(echo "${1}" | grep -w 'DEVICE_MODEL_ENABLE')

    [[ ! ${M_DEVICE_MODEL_ENABLE} ]] && return

    echo "extract dev_model module..."
    echo -e "$(echo "${1}" | grep -E 'DEVICE_MODEL')\n"

    mkdir -p ${OUTPUT_DIR}/eng/dev_model

    SRC_DEVICE_MODEL=$(find ./src \( -path ./${OUTPUT_DIR} -o -path ./${OUTPUT_TMPDIR} \) -prune -type f -o -iname "dev_model" -type d)

    find ${SRC_DEVICE_MODEL} -maxdepth 1 -name *.[ch] | grep -v example | xargs -i cp -f {} ${OUTPUT_DIR}/eng/dev_model/
}

# Generate Directory
gen_eng_dir

# Generate infra_config.h
gen_infra_default "${MACRO_LIST}"

# Generate Module Code
for mod in ${MODULES[*]}
do
    ${mod} "${MACRO_LIST}"
done

# Genrate wrapper.c
gen_wrapper_c "${MACRO_LIST}"


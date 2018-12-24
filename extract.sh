#! /bin/sh

OUTPUT_DIR=output
INFRA_DIR=${OUTPUT_DIR}/eng/src/infra

mkdir -p ${INFRA_DIR}

echo_infra_config () {
    echo "#ifndef _INFRA_CONFIG_H_" > ${INFRA_DIR}/infra_config.h
    echo "#define _INFRA_CONFIG_H_" >> ${INFRA_DIR}/infra_config.h
    echo "$1" >> ${INFRA_DIR}/infra_config.h
    echo "#endif" >> ${INFRA_DIR}/infra_config.h
}

MACRO_LIST=$(sed -n '/=y/p' make.settings | sed -n 's/=y//gp' | sed -n 's/FEATURE_//gp')
CONFIG_H=$(echo "${MACRO_LIST}" | sed -n 's/^/#define /p')

# echo "${MACRO_LIST}"
# echo "${CONFIG_H}"
echo_infra_config "${CONFIG_H}"

echo "${MACRO_LIST}" | while read line;
do
    echo ${line}
    [[ ${line} =~ "INFRA" ]] && find ./ -path ${OUTPUT_DIR} -iname "${line}.[hc]" | xargs -i cp -f {} ${INFRA_DIR}
done
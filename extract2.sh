#! /bin/bash

OUTPUT_DIR=output
INFRA_DIR=${OUTPUT_DIR}/eng/infra

XTRC_FILE_RULS=./tools/misc/xtrc_file_rules
XTRC_WRAPPER_RULS=./tools/misc/xtrc_wrapper_rules
WRAPPER_DOC=./tools/misc/wrapper

# Prepare Config Macro In make.settings
MACRO_LIST=$(sed -n '/#/!{/=y/p}' make.settings | sed -n 's/=y//gp' | sed -n 's/FEATURE_//gp')

rm -rf ${OUTPUT_DIR}/eng

# Generate infra_config.h and extract necessary infra files
mkdir -p ${INFRA_DIR}
echo "#ifndef _INFRA_CONFIG_H_" > ${INFRA_DIR}/infra_config.h
echo -e "#define _INFRA_CONFIG_H_\n" >> ${INFRA_DIR}/infra_config.h
echo "${MACRO_LIST}" | sed -n 's/^/#define /p' >> ${INFRA_DIR}/infra_config.h
echo -e "\n#endif" >> ${INFRA_DIR}/infra_config.h

find . -path ./${OUTPUT_DIR} -prune -type f -o -name "infra_types.h" | xargs -i cp -f {} ${INFRA_DIR}
find . -path ./${OUTPUT_DIR} -prune -type f -o -name "infra_defs.[ch]" | xargs -i cp -f {} ${INFRA_DIR}
find . -path ./${OUTPUT_DIR} -prune -type f -o -name "infra_compat.h" | xargs -i cp -f {} ${INFRA_DIR}

# echo -e "${MACRO_LIST}"

cond_and_check()
{
    COND_AND_VAR=$(echo $1 | awk -F '&' '{for(i=1;i<=NF;i++){print $i;}}')
    # echo "${COND_AND_VAR}"
    for item in ${COND_AND_VAR}
    do
        echo ${MACRO_LIST} | grep -wo ${item} > /dev/null
        [[ $? -ne 0 ]] && return -1
    done
    return 0
}

cond_not_check()
{
    COND_NOT_VAR=$(echo $1 | awk -F '&' '{for(i=1;i<=NF;i++){print $i;}}')
    # echo "${COND_AND_VAR}"
    for item in ${COND_NOT_VAR}
    do
        echo ${MACRO_LIST} | grep -wo ${item} > /dev/null
        [[ $? -eq 0 ]] && return 0
    done
    return -1
}

# Read xtrc_file_rules
cat ${XTRC_FILE_RULS} | while read rule
do
    COND_AND=$(echo $rule | awk -F'|' '{print $1}')
    COND_NOT=$(echo $rule | awk -F'|' '{print $2}')
    SRC_DIR=$(echo $rule | awk -F'|' '{print $3}')
    DEST_DIR=$(echo $rule | awk -F'|' '{print $4}')

    # echo "${COND_AND}"
    cond_and_check "${COND_AND}"
    [[ $? -ne 0 ]] && continue

    # echo "${COND_NOT}"
    cond_not_check "${COND_NOT}"
    [[ $? -eq 0 ]] && continue

    mkdir -p ${DEST_DIR}
    find ${SRC_DIR} -maxdepth 1 -name *.[ch] | xargs -i cp -rf {} ${DEST_DIR}
done
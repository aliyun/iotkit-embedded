#! /bin/bash

OUTPUT_DIR=output
INFRA_DIR=${OUTPUT_DIR}/eng/infra
WRAPPERS_DIR=${OUTPUT_DIR}/eng/wrappers

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

    # echo "${SRC_DIR}"
    # echo "${DEST_DIR}"

    [[ ${DEST_DIR} ]] && mkdir -p ${DEST_DIR} && find ${SRC_DIR} -maxdepth 1 -name *.[ch] | xargs -i cp -rf {} ${DEST_DIR}
done

# Generate wrapper.c
mkdir -p ${WRAPPERS_DIR}
cp -f wrappers/wrappers_defs.h ${WRAPPERS_DIR}/

# Read xtrc_wrapper_rules
while read rule
do
    # echo $rule
    COND_AND=$(echo $rule | awk -F'|' '{print $1}')
    COND_NOT=$(echo $rule | awk -F'|' '{print $2}')
    FUNC_NAME=$(echo $rule | awk -F'|' '{print $3}')
    HEADER_FILE=$(echo $rule | awk -F'|' '{print $4}')

    # echo "${COND_AND}"
    cond_and_check "${COND_AND}"
    [[ $? -ne 0 ]] && continue

    # echo "${COND_NOT}"
    cond_not_check "${COND_NOT}"
    [[ $? -eq 0 ]] && continue

    [[ ${FUNC_NAME} ]] && FUNC_NAME_LIST+=${FUNC_NAME}"\n"
    [[ ${HEADER_FILE} ]] && HEADER_FILE_LIST+=${HEADER_FILE}"\n"
    
    # [[ ${DEST_DIR} ]] && mkdir -p ${DEST_DIR} && find ${SRC_DIR} -maxdepth 1 -name *.[ch] | xargs -i cp -rf {} ${DEST_DIR}
done < ${XTRC_WRAPPER_RULS}

FUNC_NAME_LIST=$(echo -e "${FUNC_NAME_LIST}" | sed -n '/^$/!{p}' | sort -u)
HEADER_FILE_LIST=$(echo -e "${HEADER_FILE_LIST}" | sed -n '/^$/!{p}' | sort -u)

# For Debug
echo -e "\nHAL/Wrapper Function List:"
echo -e "${FUNC_NAME_LIST}" |awk '{ printf("%03d %s\n", NR, $0); }'
echo -e "\nHAL/Wrapper Header File List:"
echo -e "${HEADER_FILE_LIST}" |awk '{ printf("%03d %s\n", NR, $0); }'
echo ""

# Annotation For wrapper.c
sed -n  '/WRAPPER_NOTE:/{:a;N;/*\//!ba;p}' ${WRAPPER_DOC} | sed -n '1d;p' >> ${WRAPPERS_DIR}/wrapper.c

# Output Header File Into wrapper.c
echo -e "#include \"infra_types.h\"" >> ${WRAPPERS_DIR}/wrapper.c
echo -e "#include \"infra_defs.h\"" >> ${WRAPPERS_DIR}/wrapper.c
echo -e "#include \"wrappers_defs.h\"" >> ${WRAPPERS_DIR}/wrapper.c
echo -e "${HEADER_FILE_LIST}" | sed -n '/.h/{s/^/#include "/p}' | sed -n 's/$/"/p' >> ${WRAPPERS_DIR}/wrapper.c
echo -e "" >> ${WRAPPERS_DIR}/wrapper.c

# Generate Default Implenmentation For HAL/Wrapper Function
echo -e "${FUNC_NAME_LIST}" | while read func
do
    [[ ! ${func} ]] && return

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

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

find ./src -name "infra_types.h" | xargs -i cp -f {} ${INFRA_DIR}
find ./src -name "infra_defs.[ch]" | xargs -i cp -f {} ${INFRA_DIR}
find ./src -name "infra_compat.h" | xargs -i cp -f {} ${INFRA_DIR}

# echo -e "${MACRO_LIST}"

cond_and_check()
{
    COND_AND_VAR=$(echo $1 | awk -F '&' '{for(i=1;i<=NF;i++){print $i;}}')
    # echo "${COND_AND_VAR}"
    for item in ${COND_AND_VAR}
    do
        echo ${MACRO_LIST} | grep -wo ${item} > /dev/null
        if [ $? -ne 0 ];then
            return -1
        fi
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
        if [ $? -eq 0 ];then
            return 0
        fi
    done
    return -1
}

DOTS_LINE=".................................................................."

# Read xtrc_file_rules
TOTAL_ITERATION=$(wc -l ${XTRC_FILE_RULS}|awk '{ print $1 }')
ITER=0
while read rule
do
    ITER=$(( ${ITER} + 1 ))
    printf "\r%.40s %.2f%%" "Extract Files ${DOTS_LINE}" $(echo 100*${ITER}/${TOTAL_ITERATION}|bc -l)
    TEST=$(echo $rule | awk -F'|' '{print NF}')
    if [ ${TEST} -ne 4 ];then
        continue
    fi

    COND_AND=$(echo $rule | awk -F'|' '{print $1}')
    COND_NOT=$(echo $rule | awk -F'|' '{print $2}')
    SRC_DIR=$(echo $rule | awk -F'|' '{print $3}')
    DEST_DIR=$(echo $rule | awk -F'|' '{print $4}')

    # echo "${COND_AND}"
    cond_and_check "${COND_AND}"
    if [ $? -ne 0 ];then
        continue
    fi

    # echo "${COND_NOT}"
    cond_not_check "${COND_NOT}"
    if [ $? -eq 0 ];then
        continue
    fi

    # echo "${SRC_DIR}"
    # echo "${DEST_DIR}"

    if [ "${DEST_DIR}" != "" ];then
        mkdir -p ${DEST_DIR} && find ${SRC_DIR} -maxdepth 1 -name *.[ch] | xargs -i cp -rf {} ${DEST_DIR}
    fi
done < ${XTRC_FILE_RULS}

echo -e ""

# Generate wrapper.c
mkdir -p ${WRAPPERS_DIR}
cp -f wrappers/wrappers_defs.h ${WRAPPERS_DIR}/

# Read xtrc_wrapper_rules
TOTAL_ITERATION=$(wc -l ${XTRC_WRAPPER_RULS}|awk '{ print $1 }')
ITER=0
while read rule
do
    ITER=$(( ${ITER} + 1 ))
    printf "\r%.40s %.2f%%" "Extract HAL/Wrapper Functions ${DOTS_LINE}" $(echo 100*${ITER}/${TOTAL_ITERATION}|bc -l)
    TEST=$(echo $rule | awk -F'|' '{print NF}')
    if [ ${TEST} -ne 4 ];then
        continue
    fi

    COND_AND=$(echo $rule | awk -F'|' '{print $1}')
    COND_NOT=$(echo $rule | awk -F'|' '{print $2}')
    FUNC_NAME=$(echo $rule | awk -F'|' '{print $3}')
    HEADER_FILE=$(echo $rule | awk -F'|' '{print $4}')

    # echo "${COND_AND}"
    cond_and_check "${COND_AND}"
    if [ $? -ne 0 ];then
        continue
    fi

    # echo "${COND_NOT}"
    cond_not_check "${COND_NOT}"
    if [ $? -eq 0 ];then
        continue
    fi

    if [ "${FUNC_NAME}" != "" ];then
        FUNC_NAME_LIST="${FUNC_NAME_LIST}""${FUNC_NAME}\n"
    fi
    if [ "${HEADER_FILE}" != "" ];then
        HEADER_FILE_LIST="${HEADER_FILE_LIST}""${HEADER_FILE}\n"
    fi
done < ${XTRC_WRAPPER_RULS}

echo -e ""

FUNC_NAME_LIST=$(echo -e "${FUNC_NAME_LIST}" | sed -n '/^$/!{p}' | sort -u)
HEADER_FILE_LIST=$(echo -e "${HEADER_FILE_LIST}" | sed -n '/^$/!{p}' | sort -u)

# For Debug
if [ "${FUNC_NAME_LIST}" != "" ];then
    echo -e "\nHAL/Wrapper Function List:" && echo -e "${FUNC_NAME_LIST}" |awk '{ printf("%03d %s\n", NR, $0); }'
fi

if [ "${HEADER_FILE_LIST}" != "" ];then
    echo -e "\nHAL/Wrapper Header File List:" && echo -e "${HEADER_FILE_LIST}" |awk '{ printf("%03d %s\n", NR, $0); }'
fi

# Annotation For wrapper.c
sed -n  '/WRAPPER_NOTE:/{:a;N;/*\//!ba;p}' ${WRAPPER_DOC} | sed -n '1d;p' >> ${WRAPPERS_DIR}/wrapper.c

# Output Header File Into wrapper.c
echo -e "#include \"infra_types.h\"" >> ${WRAPPERS_DIR}/wrapper.c
echo -e "#include \"infra_defs.h\"" >> ${WRAPPERS_DIR}/wrapper.c
echo -e "#include \"wrappers_defs.h\"" >> ${WRAPPERS_DIR}/wrapper.c
echo -e "${HEADER_FILE_LIST}" | sed -n '/.h/{s/^/#include "/p}' | sed -n 's/$/"/p' >> ${WRAPPERS_DIR}/wrapper.c
echo -e "" >> ${WRAPPERS_DIR}/wrapper.c

# Generate Default Implenmentation For HAL/Wrapper Function
echo ""
TOTAL_ITERATION=$(echo "${FUNC_NAME_LIST}"|wc -w|sed -n 's/[[:space:]]//gp')

ITER=0
for func in $(echo "${FUNC_NAME_LIST}")
do
    ITER=$(( ${ITER} + 1 ))
    printf "\r%.40s %.2f%%" "Generate wrapper.c ${DOTS_LINE}" $(echo 100*${ITER}/${TOTAL_ITERATION}|bc -l)

    # echo ${func}
    if [ "${func}" = "" ];then
        continue
    fi

    FUNC_DEC=$(find ./${OUTPUT_DIR}/eng -name *wrapper.h | xargs -i cat {})
    FUNC_DEC=$(echo "${FUNC_DEC}" | sed -n '/.*'${func}'(.*/{/.*);/ba;{:c;N;/.*);/!bc};:a;p;q}')
    
    DATA_TYPE=$(echo "${FUNC_DEC}" | head -1 | awk -F'wrapper|HAL' '{print $1}' | sed s/[[:space:]]//g)
    # echo "${func}"

    sed -n '/'${func}':/{:a;N;/*\//!ba;p}' ${WRAPPER_DOC} | sed -n '1d;p' >> ${WRAPPERS_DIR}/wrapper.c

    if [ "${DATA_TYPE}" = "void" ];then
        echo "${FUNC_DEC}" | sed -n '/;/{s/;/\n{\n\treturn;\n}\n\n/g};p' >> ${WRAPPERS_DIR}/wrapper.c
    else
        echo "${FUNC_DEC}" | sed -n '/;/{s/;/\n{\n\treturn ('${DATA_TYPE}')1;\n}\n\n/g};p' >> ${WRAPPERS_DIR}/wrapper.c
    fi
done

if [ "${TOTAL_ITERATION}" = "0" ]; then
    echo "Only [dev_sign] enabled, so NO function requires being implemented in [${WRAPPERS_DIR}/wrapper.c]"
else
    echo ""
fi

echo ""
echo "Please pick up extracted source files in [${PWD}/${OUTPUT_DIR}]"

echo -e "\n"

#! /bin/bash

value_of()
{
    VAR_ALIAS="$1"
    VAR_NAME="$2"
    VAR_VAL="$(eval echo '${'${VAR_NAME}'}')"

    echo "${VAR_ALIAS}=${VAR_VAL}"
}

SELF_DIR=$(cd "$(dirname "$0")";pwd)
JPARSER=${SELF_DIR}/JSON.sh
MODEL_FL=$1
VARS_FL=$(mktemp)

if [ "${MODEL_FL}" = "" ] || [ ! -f "${MODEL_FL}" ]; then
    echo "Invalid Argument: ${MODEL_FL}"
    exit 1
fi

cat ${MODEL_FL}|${JPARSER} -c > ${VARS_FL}
source ${VARS_FL}

value_of "DEVICE_PK"        "profile__productKey"
value_of "DM_PROP_ID"       "properties__0__identifier"
value_of "DM_EVT_ID"        "events__1__identifier"
value_of "DM_EVT_OU_KEY"    "events__1__outputData__0__identifier"

rm -f ${VARS_FL}

#! /bin/bash

GIT_PREFIX="https://gitee.com/alios-things"

LOCAL_CC=${TOOLCHAIN_DLDIR}/${RELPATH}/${CC}
LOCAL_AR=${TOOLCHAIN_DLDIR}/${RELPATH}/${AR}

# echo "[RELPATH]: ${RELPATH}"
# echo "[GITPATH]: ${GITPATH}"

which ${CC} > /dev/null

if [ $? != 0 ] && [ ! -f ${LOCAL_CC} ]; then
    echo "${GIT_PREFIX}${GITPATH} -> ${TOOLCHAIN_DLDIR}" > /dev/stderr
    echo "---" > /dev/stderr
    mkdir -p ${TOOLCHAIN_DLDIR}
    cd ${TOOLCHAIN_DLDIR}
    git clone -q ${GIT_PREFIX}/${GITPATH}
    cd ${OLDPWD}
fi

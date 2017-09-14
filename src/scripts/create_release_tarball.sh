#! /bin/bash

VERSION="V2.0"
DATE_STR=$(date +"%Y%m%d")
REL_NAME="IoT-SDK_${VERSION}"
TARBALL="${REL_NAME}.tar.bz2"
DIFF_STAT="Release_${DATE_STR}.diff"

if [ ! -d src ] || [ ! -d build-rules ]; then
    pwd
    echo "Incorrect path to run! abort!"
    exit 1
fi

# Clean Section
make distclean
if [ "$(git status -s|wc -c)" != 0 ]; then
    echo "Dirty work directory! abort!"
    exit 2
fi

# Initial Section
WORKDIR=$(mktemp -d -t "${REL_NAME}-XXXXXX")
cp -rf * ${WORKDIR}/
echo ". Created [${WORKDIR}] as temp work-dir"

cd ${WORKDIR}

# Modify Section
sed -i '/armcc/d;/armar/d' make.settings
sed -i 's:^# PLATFORM_CC.*:# PLATFORM_CC = /path/to/your/cross/gcc:1' make.settings
sed -i 's:^# PLATFORM_AR.*:# PLATFORM_AR = /path/to/your/cross/ar:1'  make.settings
sed -i '/CFLAGS.*-DTEST.*/d' makefile
sed -i '/COVERAGE_CMD/d' makefile
sed -i 's/^.*WITH_MEM_STATS.*/#define WITH_MEM_STATS  0/g' src/packages/LITE-utils/lite-utils_config.h
sed -i 's/-rdynamic//g; s/--coverage//g' src/configs/config.desktop.x86
find . -name "iot.mk" -exec sed -i '/PKG_UPDATE/d' {} \;
find src -name "*.c" -exec sed -i 's:[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*:10.10.10.10:g' {} \;
for iter in $(find . -name "*.[ch]"); do
    for REPL in PRODUCT_KEY DEVICE_NAME DEVICE_SECRET DEVICE_ID; do
        sed -i "s!\(#define *[_A-Z]*${REPL} *\)\".*\"!\1\"*******************\"!g" ${iter}
    done
done

# Release Section
cd ${OLDPWD}

diff -ur . ${WORKDIR} > "${DIFF_STAT}"
echo ". Created [${DIFF_STAT}] as modified stats"

cp ${WORKDIR} ./${REL_NAME} -rf
tar jcf ${TARBALL} ./${REL_NAME}
echo ". Created [${TARBALL}] as release tarball"

# Closure Section
rm -rf ${REL_NAME} ${WORKDIR}
echo ". Removed [${REL_NAME}] from ${PWD}"

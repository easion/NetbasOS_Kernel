#!/bin/bash

TARGET=mcore-elf
PREFIX=/opt/devsys/mcore
SOURCE=/opt/devsys/src
GDB=insight-5.2.1

##########################

LOG_DIR=${PREFIX}/logs
mkdir -pv ${LOG_DIR}

DATEZ="date -u +%Y%m%d%H%MZ"

# create build directory
BUILD_DIR=${PREFIX}/build/${TARGET}/${GDB}
rm -rvf ${BUILD_DIR}
mkdir -pv ${BUILD_DIR}

# switch to build dir and configure
cd ${BUILD_DIR}

echo; echo "-----> configuring ${GDB} for ${TARGET}"
${SOURCE}/gdb/${GDB}/configure \
  --target=${TARGET} \
  --prefix=${PREFIX} \
  2>&1 | tee ${LOG_DIR}/${GDB}-${TARGET}-config-`${DATEZ}`.log

# build and install ${GDB} for ${TARGET}
echo; echo "-----> making ${GDB} for ${TARGET}"
make all install 2>&1 | tee ${LOG_DIR}/${GDB}-${TARGET}-make-`${DATEZ}`.log

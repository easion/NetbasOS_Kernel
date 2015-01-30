#!/bin/bash

TARGET=mcore-elf
PREFIX=/opt/devsys/mcore
SOURCE=/opt/devsys/src
DDD=ddd-3.3.1

##########################

LOG_DIR=${PREFIX}/logs
mkdir -pv ${LOG_DIR}

DATEZ="date -u +%Y%m%d%H%MZ"

# create build directory
BUILD_DIR=${PREFIX}/build/${TARGET}/${DDD}
rm -rvf ${BUILD_DIR}
mkdir -pv ${BUILD_DIR}

# switch to build dir and configure
cd ${BUILD_DIR}

echo; echo "-----> configuring ${DDD} for ${TARGET}"
${SOURCE}/ddd/${DDD}/configure \
  --target=${TARGET} \
  --prefix=${PREFIX} \
  2>&1 | tee ${LOG_DIR}/${DDD}-${TARGET}-config-`${DATEZ}`.log

# build and install ${DDD} for ${TARGET}
echo; echo "-----> making ${DDD} for ${TARGET}"
make install 2>&1 | tee ${LOG_DIR}/${DDD}-${TARGET}-make-`${DATEZ}`.log

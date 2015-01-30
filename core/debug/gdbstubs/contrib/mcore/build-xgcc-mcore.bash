#!/bin/bash

TARGET=mcore-elf
PREFIX=/opt/devsys/mcore
SOURCE=/opt/devsys/src
GCC=gcc-3.1.1
BINUTILS=binutils-2.12.1
NEWLIB=newlib-1.10.0

##########################

export PATH=${PATH}:${PREFIX}/bin
LOG_DIR=${PREFIX}/logs
mkdir -pv ${LOG_DIR}
DATEZ="date -u +%Y%m%d%H%MZ"

# configure binary utilities for target
echo; echo "-----> configuring ${BINUTILS} for ${TARGET}"

# create the build directory
BUILD_DIR=${PREFIX}/build/${TARGET}/${BINUTILS}
rm -rvf ${BUILD_DIR}
mkdir -pv ${BUILD_DIR}

# switch to build directory and configure
cd ${BUILD_DIR}
${SOURCE}/binutils/${BINUTILS}/configure \
  --prefix=${PREFIX} \
  --target=${TARGET} \
  2>&1 | tee ${LOG_DIR}/${BINUTILS}-${TARGET}-config-`${DATEZ}`.log

# make and install binutils for target
echo; echo "-----> making ${BINUTILS} for ${TARGET}"
make all install 2>&1 | tee ${LOG_DIR}/${BINUTILS}-${TARGET}-make-`${DATEZ}`.log

# configure core compiler to use to compile newlib
echo; echo "-----> configuring core cross-compiler for ${TARGET}"

# create the build directory
BUILD_DIR=${PREFIX}/build/${TARGET}/xgcc-core
rm -rvf ${BUILD_DIR}
mkdir -pv ${BUILD_DIR}

# switch to build directory and configure
cd ${BUILD_DIR}
${SOURCE}/gcc/${GCC}/configure \
  --target=${TARGET} \
  --prefix=${PREFIX} \
  --enable-languages=c \
  --without-headers \
  --with-newlib \
  2>&1 | tee ${LOG_DIR}/gcc-core-${TARGET}-config-`${DATEZ}`.log

# make and install xgcc core compiler for target
echo; echo "-----> making core cross-compiler for ${TARGET}"
make all-gcc install-gcc 2>&1 | tee ${LOG_DIR}/gcc-core-${TARGET}-make-`${DATEZ}`.log

# configure newlib to use with the full cross-compiler
echo; echo "-----> configuring ${NEWLIB} for ${TARGET}"

# create a build directory
BUILD_DIR=${PREFIX}/build/${TARGET}/${NEWLIB}
rm -rvf ${BUILD_DIR}
mkdir -pv ${BUILD_DIR}

# switch to build directory and configure newlib for target
cd ${BUILD_DIR}
${SOURCE}/newlib/${NEWLIB}/configure \
  --target=${TARGET} \
  --prefix=${PREFIX} \
  2>&1 | tee ${LOG_DIR}/${NEWLIB}-${TARGET}-config-`${DATEZ}`.log

# make and install newlib
echo; echo "-----> making ${NEWLIB} for ${TARGET}"
make all install info install-info \
  2>&1 | tee ${LOG_DIR}/${NEWLIB}-${TARGET}-make-`${DATEZ}`.log

# configure the full GCC cross-compiler with newlib for target
echo; echo "-----> configuring ${GCC} cross-compiler for ${TARGET}"

# create a build directory
BUILD_DIR=${PREFIX}/build/${TARGET}/xgcc
rm -rvf ${BUILD_DIR}
mkdir -pv ${BUILD_DIR}

# switch to build directory and configure for target
cd ${BUILD_DIR}
${SOURCE}/gcc/${GCC}/configure \
  --target=$TARGET \
  --prefix=${PREFIX} \
  --enable-languages=c \
  --with-newlib \
    2>&1 | tee ${LOG_DIR}/${GCC}-${TARGET}-config-`${DATEZ}`.log

# make and install cross-compiler for target
echo; echo "-----> making ${GCC} cross-compiler for ${TARGET}"
make all install 2>&1 | tee ${LOG_DIR}/${GCC}-${TARGET}-make-`${DATEZ}`.log

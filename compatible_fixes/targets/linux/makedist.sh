#!/bin/bash

# call from main directory to build versioned source archive

set -x

DIR=$(dirname $0)

VERSION_RAW=$({ echo \#define TARGET_OS_LINUX; cat ./source/lib/universal_include.h; echo APPVERSION=APP_VERSION; } | cpp | grep APPVERSION | sed -e s,APPVERSION=,, -e s,\",,g)
VERSION=$(echo $VERSION_RAW | sed -e "s, ,_,")
NAME=defcon-$VERSION
TARNAME=defcon-src-$VERSION
BINNAME=defcon-$VERSION

bzr status || exit -1

# create clean branch
rm -rf ./$TARNAME
bzr branch . ./$TARNAME || exit -1
rm ./$TARNAME/.bzr -rf 

# build
rm defcon.full
make -f ${DIR}/Makefile.release LDFLAGS="-Wl,-rpath=./lib" || exit -1

# prepare distribution
rm -rf ${BINNAME}
mkdir ${BINNAME}

# add executable
cp defcon.full ${BINNAME}/defcon.bin || exit -1

# add launch script
cp ${DIR}/launch.sh ${BINNAME}/defcon || exit -1

# add libraries
mkdir ${BINNAME}/lib || exit -1

# this line will probably fail for you. It's supposed to copy a version of
# SDL built also with apbuild into the libs subdirectory.
cp -ax /usr/local/apbuild/lib/libSDL-* ${BINNAME}/lib/ || exit -1

# package data. Requires original rar. the -s is required for DEFCON to be
# able to read the archive properly
pushd ${TARNAME} 
rm -rf ../sounds.dat ../main.dat 
rar a -r -s ../${BINNAME}/sounds.dat data/sounds || exit -1
rm -rf data/sounds
rar a -r -s ../${BINNAME}/main.dat data || exit -1
popd

# package up
tar -cjf ./${BINNAME}.tbz ./$BINNAME || exit -1
tar -cjf ./${TARNAME}.tbz ./$TARNAME || exit -1

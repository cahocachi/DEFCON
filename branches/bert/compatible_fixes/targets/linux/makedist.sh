#!/bin/bash

# call from main directory to build versioned source archive

#set -x

DIR=$(dirname $0)

VERSION_RAW=$({ echo \#define TARGET_OS_LINUX; cat ./source/lib/universal_include.h; echo APPVERSION=APP_VERSION; } | cpp | grep APPVERSION | sed -e s,APPVERSION=,, -e "s, *LINUX,," -e s,\",,g)
VERSION=$(echo $VERSION_RAW | sed -e "s, ,_,g")
NAME=defcon-$VERSION
TARNAME=defcon-src-$VERSION
BINNAME=defcon-$VERSION

bzr status || exit -1

function filterlog()
{
    # keep a backlog of two lines so we can properly cut out merges
    lastline2=$(line)
    lastline1=$(line)

    while line=$(line); do
        if $( echo $line | grep authkey > /dev/null ); then line="        <REDACTED>"; fi
        if $( echo $line | grep -i "merging" > /dev/null ); then 
            # swallow four lines (this one is included)
            line=$(line)
            lastline2=$(line)
            lastline1=$(line)
        else
            echo "$lastline2"

            lastline2="$lastline1"
            lastline1="$line"
        fi
    done

    echo "$lastline2"
    echo "$lastline1"
}

# create clean branch
rm -rf ./$TARNAME
if ! bzr branch . ./$TARNAME; then
    files=*
    mkdir $TARNAME
    cp -ax $files $TARNAME
    find $TARNAME -name *~ -exec rm \{\} \;
else
  # generate changelog
  bzr log --gnu-changelog --include-merges | filterlog > ${TARNAME}/ChangeLog.txt
fi
rm ./$TARNAME/.bzr -rf 
if test -z "$DONTTAR"; then
  tar -cjf ./${TARNAME}.tbz ./$TARNAME || exit -1
else
  VERSION=${VERSION}-TEST
fi

# build
make -f ${DIR}/Makefile.release || exit -1

# prepare distribution
rm -rf ${BINNAME}
mkdir ${BINNAME}

# add executable
cp defcon.full ${BINNAME}/defcon.bin || exit -1

# add launch script
cp ${DIR}/launch.sh ${BINNAME}/defcon || exit -1

# add libraries
mkdir ${BINNAME}/lib || exit -1

# add READMEs
cp README.txt ${BINNAME} || exit 1
cp "LICENSE AGREEMENT.txt" ${BINNAME}/DEVELOPER_LICENSE_AGREEMENT.txt || exit 1
cp ${TARNAME}/ChangeLog.txt ${BINNAME}/ || exit 1

# this line will probably fail for you. It's supposed to copy a version of
# SDL and ogg/vorbis built also with apbuild into the libs subdirectory.
LIBSRC=/usr/local/apbuild/lib
cp -ax ${LIBSRC}/libSDL-* ${BINNAME}/lib/ || exit -1
cp -ax ${LIBSRC}/libogg* ${BINNAME}/lib/ || exit -1
cp -ax ${LIBSRC}/libvorbis* ${BINNAME}/lib/ || exit -1

# package data. Requires original rar. the -s is required for DEFCON to be
# able to read the archive properly
pushd ${TARNAME} 
rm -rf ../sounds.dat ../main.dat 
rar a -r -s ../${BINNAME}/sounds.dat data/sounds || exit -1
rm -rf data/sounds
rar a -r -s ../${BINNAME}/main.dat data || exit -1
popd

# package up
if test -z "$DONTTAR"; then
  tar -cjf ./${BINNAME}-linux.tbz ./$BINNAME || exit -1
fi

# prepare Mac source tarball
rm -rf ${TARNAME}/data
rm -rf ${TARNAME}/localisation
cp ${BINNAME}/*.dat ${TARNAME} || exit -1
sed < targets/macosx_tiger/DMGMakefile -e "s,VERSION ?=.*,VERSION ?= ${VERSION}", > ${TARNAME}/targets/macosx_tiger/DMGMakefile || exit -1
if test -z "$DONTTAR"; then
  tar -cjf ./${TARNAME}-mac.tbz ./$TARNAME || exit -1
fi

# prepare Windows source tarball
sed < targets/msvc/Defcon.nsi -e "s,define PRODUCT_VERSION.*,define PRODUCT_VERSION \"${VERSION}\"", > ${TARNAME}/targets/msvc/Defcon.nsi || exit -1
todos ${TARNAME}/*.txt || exit -1
todos ${TARNAME}/targets/msvc/Defcon.nsi || exit -1
if test -z "$DONTTAR"; then
  tar -cjf ./${TARNAME}-windows.tbz ./$TARNAME || exit -1

  # cleanup
  rm -rf ${TARNAME} ${BINNAME}
fi

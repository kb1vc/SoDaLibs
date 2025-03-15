#!/bin/bash

# build the whole shebang.

if [ "$1" == "LOCAL" ]
then
    INSTALL_ROOT=/mnt/install_root
    SOURCE_ROOT=/source_tree
else
    INSTALL_ROOT=/usr
    SOURCE_ROOT=/mnt/source_tree
fi

if [ "$2" == "BUILD_KIT" ]
then
    KBUILD_OPT="-DBUILD_RPM=ON"
else
    KBUILD_OPT=""
fi



echo "****************"
echo ${INSTALL_ROOT}
echo $1
echo "***************"

cd /mnt
mkdir tool_tree
cd tool_tree
mkdir build
cd build
cmake  -DCMAKE_INSTALL_PREFIX=${INSTALL_ROOT} ${KBUILD_OPT} ${SOURCE_ROOT}
make
make install
if [ "$2" == "BUILD_KIT" ]
then
    make package
fi



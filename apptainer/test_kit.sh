#!/bin/bash -v

if  [ "$1" == "LOCAL" ]
then
    EXAMPLE_DIR=/mnt/install_root/share/SoDaUtils/examples
    INSTALL_DIR=/mnt/install_root
else
    # rpm and install are the same
    EXAMPLE_DIR=/usr/share/SoDaUtils/examples
    INSTALL_DIR=/usr
fi
   

echo "****************"
echo ${EXAMPLE_DIR}
echo "$1"
echo "***************"

cd /mnt
# then build the  examples
mkdir example
cd example
cp ${EXAMPLE_DIR}/* .
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=${INSTALL_DIR} ../
make
./Format_Test
./FormatExample

if [ "$2" == "TESTYAML" ]
then
    ./PropertyTreeYamlExample ../test1.yaml
fi

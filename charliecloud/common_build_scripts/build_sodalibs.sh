#!/bin/bash

echo 'Building SoDaLibs'

cmake ../
make
ctest -j 4
make install

#!/bin/bash

echo 'Building SoDaLibs'

cmake ../
make
ctest
make install

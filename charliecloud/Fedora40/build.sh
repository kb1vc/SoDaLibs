#!/bin/bash

export CH_IMAGE_STORAGE=`pwd`/images
ch-image build --bind `pwd`/build_scripts:/mnt/0 -t fedora40_sodasignals -f Dockerfile .

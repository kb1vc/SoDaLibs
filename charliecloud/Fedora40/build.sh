#!/bin/bash

export CH_IMAGE_STORAGE=`pwd`/images
ch-image build  --bind `pwd`/build_scripts:/mnt/0 -t fedora40_sodabase -f DockerBase .
ch-image build --rebuild --bind `pwd`/../common_build_scripts:/mnt/0 -t fedora40_sodalibs -f DockerKit .

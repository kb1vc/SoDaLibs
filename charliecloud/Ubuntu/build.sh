#!/bin/bash

export CH_IMAGE_STORAGE=`pwd`/images
ch-image build -t ubuntu_sodabase -f DockerBase .
ch-image build --rebuild --bind=`pwd`/../common_build_scripts:/mnt/0 -t ubuntu_sodasignals -f DockerKit .

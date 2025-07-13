#!/bin/bash -v

# build_any.sh <container> <dockerfile>
# build a container from fedora40_sodabase

echo "[$1] [$2] [$3]"

if [ -z "$1" ]
then
    echo "build_any.sh <container> <dockerfile>"
    echo "build a container from fedora40_sodabase [optional-args]"
    exit
fi

if [ -z "$2" ]
then
    echo "build_any.sh <container> <dockerfile> [optional-args]"
    echo "build a container from fedora40_sodabase"
    exit
fi

if [ -z "$3" ]
then
    argstr=""
else
    argstr="--build-arg $3"    
fi

container=$1
dockerfile=$2

echo "Container ${container} $1 Dockerfile ${dockerfile} argstr ${argstr}"

export CH_IMAGE_STORAGE=`pwd`/images
ch-image build --bind `pwd`/:/mnt1 --bind `pwd`/../common_build_scripts:/mnt/0 ${argstr} -t ${container} -f ${dockerfile} .


# BSD 2-Clause License
# 
# Copyright (c) 2025, kb1vc
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

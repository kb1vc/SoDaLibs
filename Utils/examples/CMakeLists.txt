CMAKE_MINIMUM_REQUIRED(VERSION 3.10.0) # this lets out really old distros like CentOS 6

IF(CMAKE_VERSION VERSION_GREATER 3.10.0)
  CMAKE_POLICY(SET CMP0048 NEW)
  SET(CMAKE_CXX_STANDARD 11)
ENDIF()

PROJECT(SoDaUtilsExample)

FIND_PACKAGE(Threads REQUIRED)
## taken care of by sodautils package FIND_PACKAGE(jsoncpp REQUIRED)

# optimizations
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

# This is all you need to do if the SoDaUtils stuff
# was installed correctly. 
FIND_PACKAGE(SoDaUtils REQUIRED)

########### next target ###############


add_executable(UtilsExample UtilsExample.cxx)
target_link_libraries(UtilsExample PRIVATE SoDaUtils::sodautils)


add_executable(OptionsExample OptionsExample.cxx)
target_link_libraries(OptionsExample PRIVATE SoDaUtils::sodautils)

add_executable(SafeOptionsExample SafeOptionsExample.cxx)
target_link_libraries(SafeOptionsExample PRIVATE SoDaUtils::sodautils)

add_executable(FormatExample FormatExample.cxx)
target_link_libraries(FormatExample PRIVATE SoDaUtils::sodautils)

add_executable(Format_Test Format_Test.cxx)
target_link_libraries(Format_Test PRIVATE SoDaUtils::sodautils)


add_executable(BarrierExample BarrierExample.cxx)
target_link_libraries(BarrierExample PRIVATE SoDaUtils::sodautils Threads::Threads)

add_executable(PropertyTreeExample PropertyTreeExample.cxx)
target_link_libraries(PropertyTreeExample PRIVATE SoDaUtils::sodautils Threads::Threads)

find_package(jsoncpp)
if(jsoncpp_FOUND)
  add_executable(PropertyTreeExample2 PropertyTreeExample2.cxx)
  target_link_libraries(PropertyTreeExample2 PRIVATE SoDaUtils::sodautils Threads::Threads JsonCpp::JsonCpp)
endif()




############################################################################
#
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
#
############################################################################

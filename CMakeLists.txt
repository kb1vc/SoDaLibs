CMAKE_MINIMUM_REQUIRED(VERSION 3.10.0) # this lets out really old distros like CentOS 6


IF(CMAKE_VERSION VERSION_GREATER 3.0.0)
  CMAKE_POLICY(SET CMP0048 NEW)
  SET(CMAKE_CXX_STANDARD 11)
ENDIF()

PROJECT(SoDaLibs)

# version 5.1.0 adds charlicloud builds and RPM and DEB package builds
# version 5.2.0 removes the external jsoncpp-devel requirement -
# the jsoncpp library is built as a part of this build. It is the
# only way I could figure out how to keep the dependency away from
# client applications. dumb.
# version 5.3.0 fixes a bunch of stuff where client apps still needed jsoncpp-devel... 

SET(SoDaLibs_VERSION_MAJOR 5)
SET(SoDaLibs_VERSION_MINOR 3)
SET(SoDaLibs_VERSION_PATCH 0)
SET(SoDaLibs_VERSION "${SoDaLibs_VERSION_MAJOR}.${SoDaLibs_VERSION_MINOR}.${SoDaLibs_VERSION_PATCH}")

INCLUDE(GNUInstallDirs)
INCLUDE(CTest)
INCLUDE(CMakePrintHelpers)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH} )

# optimizations
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

SET(CMAKE_ARGS "-DBUILD_SHARED_LIBS=0 ${CMAKE_ARGS}" )

SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")


FIND_PACKAGE(Git QUIET)


IF((EXISTS "${CMAKE_SOURCE_DIR}/.git/") AND GIT_FOUND)
  EXECUTE_PROCESS(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  EXECUTE_PROCESS(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  SET(SoDaLibs_GIT_ID "${GIT_BRANCH}:${GIT_COMMIT_HASH}")
  FILE(WRITE ${CMAKE_BINARY_DIR}/saved_git_version.txt "${GIT_BRANCH}\n")
  FILE(APPEND ${CMAKE_BINARY_DIR}/saved_git_version.txt "${GIT_COMMIT_HASH}\n")    
elseIF(EXISTS ${CMAKE_BINARY_DIR}/saved_git_version.txt)
  MESSAGE("Retrieving GIT version info from saved_git_version.txt")
  FILE(STRINGS ${CMAKE_BINARY_DIR}/saved_git_version.txt git_version_strings)
  LIST(GET git_version_strings 0 GIT_BRANCH)
  LIST(GET git_version_strings 1 GIT_COMMIT_HASH)
  SET(SoDaLibs_GIT_ID "${GIT_BRANCH}:${GIT_COMMIT_HASH}")
else()
  SET(SoDaLibs_GIT_ID "UNKNOWN")  
ENDIF()


FIND_PACKAGE(PkgConfig REQUIRED)
pkg_check_modules(ThisShouldNotExist jsoncpp IMPORTED_TARGET QUIET) # GLOBAL ?

IF(JSONCPP_FOUND)
  pkg_check_modules(JSONCPP jsoncpp IMPORTED_TARGET) # GLOBAL ? 
  set(JSONCPP_LIB_DEP "${JSONCPP_LIBRARY_DIRS}/libjsoncpp.a")
  set(JSONCPP_INCL_DIR ${JSONCPP_INCLUDE_DIRS})
  set(JSONCPP_IS_EXTERNAL 0)
ELSE()
  # build everything from an imported kit
  
  # we depend on jsoncpp but I don't want to depend on the
  # local system to have it.  Not sure how well this is
  # going to work...
  include(ExternalProject)
  set(EXTERNAL_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/externs)

  ExternalProject_Add(jsoncpp-proj
    GIT_REPOSITORY    https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG           1.9.6
    CMAKE_ARGS -DBUILD_SHARED_LIBS=0
    CMAKE_ARGS -DBUILD_STATIC_LIBS=1
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_PREFIX}
  )

  SET(JSONCPP_INCL_DIR ${EXTERNAL_INSTALL_PREFIX}/include)
  SET(JSONCPP_LIB_DEP ${EXTERNAL_INSTALL_PREFIX}/lib64/libjsoncpp.a)
  SET(JSONCPP_IS_EXTERNAL 1)
ENDIF()

# this helps in debug, let's keep it around. 
if(0)
  get_cmake_property(_vns VARIABLES)
  list(SORT _vns)
  foreach(_vn ${_vns})
    message(STATUS "${_vn}=[${${_vn}}]")
  endforeach()
endif()


  

ADD_SUBDIRECTORY(Utils)

set(FFTW_USE_STATIC_LIBS TRUE)
FIND_PACKAGE(SoDa_FFTW)
ADD_SUBDIRECTORY(Signals)

IF(BUILD_RPM OR BUILD_DEB)
  INCLUDE(cmake/BuildPackage.cmake)
ENDIF()


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

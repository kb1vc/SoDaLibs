# BSD 2-Clause License
# 
# Copyright (c) 2021, 2025 kb1vc
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

include(GNUInstallDirs)

# SoDaUtils is a library containing:
#   SoDa::Format intelligent formatting of text from
#                C++.  It pays special attention to formatting 
#                floating point values,
#                offering an option to print in "engineering notation"
#                where the exponent is always a multiple of 3.
#
# If the library isn't installed, we'll make a local version.
# This trick doesn't work for MacOS, as the system relies on dynamic libraries
# and so it gets tripped up if the dynamic library doesn't get installed.
#

include(ExternalProject)

IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  # If the package isn't installed, we quit. 
  FIND_PACKAGE(SoDaUtils REQUIRED)
ELSE()
  # If we find the package, yippeeee!
  FIND_PACKAGE(SoDaUtils QUIET)
  message("SoDaUtils_FOUND = ${SoDaUtils_FOUND}")
  IF(NOT SoDaUtils_FOUND)
    # The SoDa::Utils package hasn't been installed.
    # Get it and build it as an external package.
    # get sodaformat
    ExternalProject_Add(
      SoDaUtilsLib
      #  PREFIX ${PROJECT_BINARY_DIR}/sodaformat-kit
      GIT_REPOSITORY https://github.com/kb1vc/SoDaUtils.git
      GIT_TAG v_3.0.0
      SOURCE_DIR sodaformatlib
      CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
      )

    ExternalProject_Get_Property(SoDaUtilsLib INSTALL_DIR)
    message("About to set SoDaUtils stuff... to ${INSTALL_DIR}")
    set(SoDaUtils_ROOT ${INSTALL_DIR})
    set(SoDaUtils_INCLUDE_DIR ${SoDaUtils_ROOT}/include)
    set(SoDaUtils_LIBRARIES ${SoDaUtils_ROOT}/${CMAKE_INSTALL_LIBDIR}/libsodautils.a)
    set_property(TARGET SoDaUtilsLib PROPERTY IMPORTED_LOCATION ${SoDaUtils_LIB_DIR}/libsodautils)
  ENDIF()
ENDIF()

  

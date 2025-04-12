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

INCLUDE(GNUInstallDirs)
INCLUDE(CTest)
  
SET(SoDaSignals_VERSION_MAJOR 4)
SET(SoDaSignals_VERSION_MINOR 2)
SET(SoDaSignals_VERSION_PATCH 0)
SET(SoDaSignals_VERSION "${SoDaSignals_VERSION_MAJOR}.${SoDaSignals_VERSION_MINOR}.${SoDaSignals_VERSION_PATCH}")

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH} )

MESSAGE("SoDaSignals_VERSION=[${SoDaSignals_VERSION}]")

SET(DOXY_PROJECT_VERSION "${SoDaSignals_VERSION}-${SoDaUtils_GIT_ID}")

set(SIGNALS_SUBLIB_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

CONFIGURE_FILE (
"${SIGNALS_SUBLIB_SOURCE_DIR}/version.hxx.in"
"${CMAKE_CURRENT_BINARY_DIR}/SignalsVersion.hxx"
)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
  
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/SignalsVersion.hxx DESTINATION "include/SoDa")

set(SIGNALS_CONFIG_EXPORT_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/SoDaSignals")
set(FFTW_CONFIG_EXPORT_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/SoDa_FFTW")

include(InstallRequiredSystemLibraries)

install(EXPORT SoDaSignalsTargets
  FILE SoDaSignalsTargets.cmake
  DESTINATION ${SIGNALS_CONFIG_EXPORT_DIR}
)

set(SIGNALS_SUBLIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

configure_package_config_file (
  "${SIGNALS_SUBLIB_SOURCE_DIR}/SoDaSignalsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/SoDaSignalsConfig.cmake"
  INSTALL_DESTINATION "${SIGNALS_CONFIG_EXPORT_DIR}"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)  

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/SoDaSignalsConfigVersion.cmake"
  VERSION "${SoDaSignals_VERSION}"  
  COMPATIBILITY AnyNewerVersion)
  
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/SoDaSignalsConfig.cmake
  DESTINATION ${SIGNALS_CONFIG_EXPORT_DIR}
)

## FFTW still doesn't have an actual find cmake package.
## I'm usign one kindly created by Wenzel Jakop and Patrick Bos (see the LICENSE files)
FILE(GLOB find_fftw_files ${PROJECT_SOURCE_DIR}/cmake/Modules/FindFFTW.*E*)
INSTALL(FILES
  ${find_fftw_files}
  DESTINATION ${FFTW_CONFIG_EXPORT_DIR}
)

INSTALL(FILES
  ${PROJECT_SOURCE_DIR}/cmake/Modules/FindFFTW.cmake
  DESTINATION ${FFTW_CONFIG_EXPORT_DIR}
  RENAME SoDa_FFTWConfig.cmake
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${SIGNALS_SUBLIB_SOURCE_DIR}/include)

# redirect sodautils includes
set(sodautils_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/Utils/include)



set(DEP_INCLS ${FFTW3f_INCLUDE_DIRS} ${sodautils_include_dirs})

set(SWIG_ENABLED 0)

# get the right directory name/forms

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(test)

FIND_PACKAGE(Doxygen QUIET)

IF(DOXYGEN_FOUND)
  SET(SIGNALS_PROJECT_NAME "SoDa Signals")  
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/DoxyfileSignals @ONLY)
  ADD_CUSTOM_TARGET(sodasignals_doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/DoxyfileSignals
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating Signals API documentation with Doxygen" VERBATIM
    )

  INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/SoDaSignals/doc)
ENDIF()

INSTALL(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/examples/ DESTINATION share/SoDaSignals/examples)


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

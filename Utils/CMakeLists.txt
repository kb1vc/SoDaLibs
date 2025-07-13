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

FIND_PACKAGE(Threads REQUIRED)

# Automatic CMake config generation inspired from https://github.com/jbeder/yaml-cpp.git
INCLUDE(CMakePackageConfigHelpers)
INCLUDE(CMakeDependentOption)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(GNUInstallDirs)
INCLUDE(CTest)

# version 5 adds PropertyTree
SET(SoDaUtils_VERSION_MAJOR 5)
SET(SoDaUtils_VERSION_MINOR 0)
SET(SoDaUtils_VERSION_PATCH 1)
SET(SoDaUtils_VERSION "${SoDaUtils_VERSION_MAJOR}.${SoDaUtils_VERSION_MINOR}.${SoDaUtils_VERSION_PATCH}")

MESSAGE("SoDaUtils_VERSION=[${SoDaUtils_VERSION}]")

SET(DOXY_PROJECT_VERSION "${SoDaUtils_VERSION}-${SoDaUtils_GIT_ID}")
CONFIGURE_FILE("${CMAKE_CURRENT_LIST_DIR}/version.hxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/version.hxx")

SET(SODAUTILS_BUILD_VERSION_INCL_DIR ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR}/include)

FILE(GLOB all_includes ${CMAKE_CURRENT_LIST_DIR}/include/*.hxx)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/version.hxx DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SoDa")

INSTALL(FILES ${all_includes} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SoDa")

ADD_SUBDIRECTORY(src)

set(UTILS_SUBLIB_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

IF(NOT DISABLE_DOXYGEN)	
  FIND_PACKAGE(Doxygen QUIET)

  IF(DOXYGEN_FOUND)
    SET(UTILS_PROJECT_NAME "SoDa Utils")
    CONFIGURE_FILE(${UTILS_SUBLIB_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/DoxyfileUtils @ONLY)
    ADD_CUSTOM_TARGET(sodautils_doc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/DoxyfileUtils
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating Utils API documentation with Doxygen" VERBATIM
    )

    INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/SoDaUtils/doc)
  ENDIF()
ENDIF()

FILE(GLOB util_examp_files "${UTILS_SUBLIB_SOURCE_DIR}/examples/*.[ch]xx")
INSTALL(FILES
  ${util_examp_files}
  ${UTILS_SUBLIB_SOURCE_DIR}/examples/CMakeLists.txt
  ${UTILS_SUBLIB_SOURCE_DIR}/examples/test1.json  
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/SoDaUtils/examples  
)

target_include_directories(sodautils
  PUBLIC
  $<BUILD_INTERFACE:${CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# do this because "LIBDIR" in ubuntu is all screwed up. 
set(UTILS_CONFIG_EXPORT_DIR "${CMAKE_INSTALL_DATADIR}/cmake/SoDaUtils")

#include(InstallRequiredSystemLibraries)

message("******* Installing Utils to [${UTILS_CONFIG_EXPORT_DIR}]")
install(EXPORT SoDaUtilsTargets
  FILE SoDaUtilsTargets.cmake
  DESTINATION ${UTILS_CONFIG_EXPORT_DIR}
  )


configure_package_config_file(
  "${UTILS_SUBLIB_SOURCE_DIR}/SoDaUtilsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/SoDaUtilsConfig.cmake"
  INSTALL_DESTINATION "${UTILS_CONFIG_EXPORT_DIR}"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/SoDaUtilsConfigVersion.cmake"
  VERSION "${SoDaUtils_VERSION}"
  COMPATIBILITY AnyNewerVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/SoDaUtilsConfig.cmake
  DESTINATION ${UTILS_CONFIG_EXPORT_DIR}
)

add_subdirectory(test)

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

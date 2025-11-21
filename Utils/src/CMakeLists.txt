
########### next target ###############

set(UTILS_SRCS
	Split.cxx
	Options.cxx
	Format.cxx
	UtilsBase.cxx
	Barrier.cxx
	PropertyTree.cxx
	${JSON_BASE_DIR}/jsoncpp.cpp
)

add_library(sodautils STATIC ${UTILS_SRCS})
add_library(sodautils::sodautils ALIAS sodautils)
set_target_properties(sodautils PROPERTIES VERSION ${SoDaUtils_VERSION})
target_link_libraries(sodautils PRIVATE Threads::Threads)



target_include_directories(sodautils
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${SODAUTILS_BUILD_VERSION_INCL_DIR}>
  $<BUILD_INTERFACE:${JSON_BASE_DIR}>  
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SoDa>
)

target_sources(sodautils
  PRIVATE
  ${UTILS_SRCS})

set_target_properties(sodautils PROPERTIES
  VERSION ${SoDaUtils_VERSION}
  SOVERSION "${SoDaUtils_VERSION_MAJOR}.${SoDaUtils_VERSION_MINOR}"
  PROJECT_LABEL "SoDaUtils" 
  DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")


set(installable_libs sodautils)

install(TARGETS ${installable_libs}
  EXPORT SoDaUtilsTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


install(EXPORT SoDaUtilsTargets
  FILE SoDaUtilsConfig.cmake 
  NAMESPACE SoDaUtils::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SoDaUtils
)





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

set(SIGNALS_SRCS
	FFT.cxx
	Periodogram.cxx
	Filter.cxx
	FilterSpec.cxx
	ReSampler.cxx
	NCO.cxx
	OSFilter.cxx
)


add_library(sodasignals STATIC ${SIGNALS_SRCS})
target_link_libraries(sodasignals PUBLIC SoDa_FFTW::Float)
target_compile_definitions(sodasignals PRIVATE SODA_LIB_BUILD)	
target_include_directories(sodasignals PRIVATE ${PROJECT_SOURCE_DIR})

target_link_directories(sodasignals PUBLIC ${FFTW3_LIBRARY_DIRS} ${FFTW3F_LIBRARY_DIRS})
set(SIGNALS_VERSION "${SoDaSignals_VERSION_MAJOR}.${SoDaSignals_VERSION_MINOR}.${SoDaSignals_VERSION_PATCH}")
set_target_properties(sodasignals PROPERTIES VERSION ${SIGNALS_VERSION})

target_include_directories(sodasignals
  PUBLIC
  $<BUILD_INTERFACE:${CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
  $<BUILD_INTERFACE:${SODASIGNALS_BUILD_VERSION_INCL_DIR}>
)

#install(TARGETS sodasignals DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS sodasignals
  EXPORT SoDaSignalsTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

install(EXPORT SoDaSignalsTargets
  FILE SoDaSignalsConfig.cmake
  NAMESPACE SoDaSignals
  DESTINATION ${SIGNALS_CONFIG_EXPORT_DIR}
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

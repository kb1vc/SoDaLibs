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
)

#install(TARGETS sodasignals DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS sodasignals
  EXPORT SoDaSignalsTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )


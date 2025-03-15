
########### next target ###############

set(UTILS_SRCS
	Split.cxx
	Options.cxx
	Format.cxx
	UtilsBase.cxx
	Barrier.cxx
	PropertyTree.cxx    
	PropertyTreeYAML.cxx
)


add_library(sodautils STATIC ${UTILS_SRCS})
add_library(sodautils::sodautils ALIAS sodautils)
set_target_properties(sodautils PROPERTIES VERSION ${SoDaUtils_VERSION})
target_link_libraries(sodautils PUBLIC Threads::Threads)

target_include_directories(sodautils
  PUBLIC
  $<BUILD_INTERFACE:${CURRENT_SOURCE_DIR}/include>
  PRIVATE
  $<BUILD_INTERFACE:${CURRENT_SOURCE_DIR}/src>
  INTERFACE
  $<BUILD_INTERFACE:${CURRENT_SOURCE_DIR}/include>
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

target_link_libraries(sodautils PUBLIC yaml-cpp)

set(installable_libs sodautils)

install(TARGETS ${installable_libs}
  EXPORT SoDaUtilsTargets
  DESTINATION lib)


get_property(imported_targets DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
message("imported targets [${imported_targets}]")


########### next target ###############

set(UTILS_SRCS
	Split.cxx
	Options.cxx
	Format.cxx
	UtilsBase.cxx
	Barrier.cxx
)


if(yaml-cpp_FOUND)
  set(PROPS_SRCS
    PropertyTree.cxx    
    PropertyTreeYAML.cxx)    
  add_library(sodaprops STATIC ${PROPS_SRCS})
  add_library(sodaprops::sodaprops ALIAS sodaprops)
  target_link_libraries(sodaprops PRIVATE ${YAML_CPP_LIBRARIES})
  set_target_properties(sodaprops PROPERTIES VERSION ${SoDaUtils_VERSION})
endif()	

add_library(sodautils STATIC ${UTILS_SRCS})
add_library(sodautils::sodautils ALIAS sodautils)
set_target_properties(sodautils PROPERTIES VERSION ${SoDaUtils_VERSION})
target_link_libraries(sodautils Threads::Threads)

target_include_directories(sodautils
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  #$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SoDa>
  PRIVATE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)

target_sources(sodautils
  PRIVATE
  ${UTILS_SRCS})

set_target_properties(sodautils PROPERTIES
  VERSION ${SoDaUtils_VERSION}
  SOVERSION "${SoDaUtils_VERSION_MAJOR}.${SoDaUtils_VERSION_MINOR}"
  PROJECT_LABEL "SoDaUtils"
  DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

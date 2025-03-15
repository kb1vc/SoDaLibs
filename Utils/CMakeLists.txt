
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(yaml-cpp QUIET)
 
IF(yaml-cpp_FOUND)
  # we need to create the lib dir for yaml-cpp
  # alas YAML_CPP_LIB_DIR isn't defined in the config file (!)
  # we need to create it from YAML_CPP_CMAKE_DIR (!)
  set(YAML_CPP_LIB_DIR "${YAML_CPP_CMAKE_DIR}/../../")
ELSE()
  message("YAML-CPP library was not found. We will not build SoDaProperties library.")
ENDIF()

# Automatic CMake config generation inspired from https://github.com/jbeder/yaml-cpp.git
INCLUDE(CMakePackageConfigHelpers)
INCLUDE(CMakeDependentOption)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(GNUInstallDirs)
INCLUDE(CTest)

SET(SoDaUtils_VERSION_MAJOR 3)
SET(SoDaUtils_VERSION_MINOR 0)
SET(SoDaUtils_VERSION_PATCH 0)
SET(SoDaUtils_VERSION "${SoDaUtils_VERSION_MAJOR}.${SoDaUtils_VERSION_MINOR}.${SoDaUtils_VERSION_PATCH}")

MESSAGE("SoDaUtils_VERSION=[${SoDaUtils_VERSION}]")

SET(DOXY_PROJECT_VERSION "${SoDaUtils_VERSION}-${SoDaUtils_GIT_ID}")
CONFIGURE_FILE("${CMAKE_CURRENT_LIST_DIR}/version.hxx.in"
  "${PROJECT_BINARY_DIR}/UtilsVersion.hxx")


INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_LIST_DIR}/include ${YAML_CPP_INCLUDE_DIR})

FILE(GLOB all_includes ${CMAKE_CURRENT_LIST_DIR}/include/*.hxx)

if(NOT yaml-cpp_FOUND) 
  FILE(GLOB ptree_includes ${CMAKE_CURRENT_LIST_DIR}/include/Property*.hxx)
  foreach(pti IN LISTS ptree_includes)
    list(REMOVE_ITEM all_includes ${pti})  
  endforeach()
endif()

INSTALL(FILES ${PROJECT_BINARY_DIR}/UtilsVersion.hxx DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SoDa")
INSTALL(FILES ${all_includes} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SoDa")

ADD_SUBDIRECTORY(src)

set(UTILS_SUBLIB_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

IF(NOT DISABLE_DOXYGEN)	
  FIND_PACKAGE(Doxygen QUIET)

  IF(DOXYGEN_FOUND)
    CONFIGURE_FILE(${UTILS_SUBLIB_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/DoxyfileUtils @ONLY)
    ADD_CUSTOM_TARGET(sodautilsdoc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/DoxyfileUtils
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

    INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/SoDaUtils/doc)
  ENDIF()
ENDIF()

FILE(GLOB examp_files "${UTILS_SUBLIB_SOURCE_DIR}/examples/*.*")
INSTALL(FILES ${examp_files}
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/SoDaUtils/examples  
)

set(CONFIG_EXPORT_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/SoDaUtils")
set(EXPORT_TARGETS sodautils)

configure_package_config_file(
  "${UTILS_SUBLIB_SOURCE_DIR}/SoDaUtilsConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/SoDaUtilsConfig.cmake"
  INSTALL_DESTINATION "${CONFIG_EXPORT_DIR}"
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR CONFIG_EXPORT_DIR
)
  write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/SoDaUtilsConfigVersion.cmake"
    VERSION "${SoDaUtils_VERSION}"  
    COMPATIBILITY AnyNewerVersion)


unset(EXPORT_TARGETS)

install(TARGETS sodautils
  EXPORT sodautils-targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(EXPORT sodautils-targets
  DESTINATION "${CONFIG_EXPORT_DIR}")

install(FILES
  "${PROJECT_BINARY_DIR}/SoDaUtilsConfig.cmake"
  "${PROJECT_BINARY_DIR}/SoDaUtilsConfigVersion.cmake"
  DESTINATION "${CONFIG_EXPORT_DIR}")

unset(CONFIG_EXPORT_DIR)

if(yaml-cpp_FOUND)
  set(CONFIG_EXPORT_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/SoDaProps")
  set(EXPORT_TARGETS sodaprops)
  write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/SoDaPropsConfigVersion.cmake"
    VERSION "${SoDaUtils_VERSION}"  
    COMPATIBILITY AnyNewerVersion)

  configure_package_config_file(
    "${UTILS_SUBLIB_SOURCE_DIR}/SoDaPropsConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/SoDaPropsConfig.cmake"
    INSTALL_DESTINATION "${CONFIG_EXPORT_DIR}"
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR CONFIG_EXPORT_DIR
  )
  unset(EXPORT_TARGETS)
  
  install(TARGETS sodaprops
    EXPORT sodaprops-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(EXPORT sodaprops-targets
    DESTINATION "${CONFIG_EXPORT_DIR}")
  install(FILES
    "${PROJECT_BINARY_DIR}/SoDaPropsConfig.cmake"  
    "${PROJECT_BINARY_DIR}/SoDaPropsConfigVersion.cmake"
    DESTINATION "${CONFIG_EXPORT_DIR}")

endif()

add_subdirectory(test)


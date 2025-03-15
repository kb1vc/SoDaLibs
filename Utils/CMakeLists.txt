
FIND_PACKAGE(Threads REQUIRED)

get_property(th_imported_targets DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
message("after threads imported targets [${th_imported_targets}]")

FIND_PACKAGE(yaml-cpp REQUIRED)
get_property(yaml_imported_targets DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
message("after yaml imported targets [${yaml_imported_targets}]")


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

set(UTILS_CONFIG_EXPORT_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/SoDaUtils")

include(InstallRequiredSystemLibraries)

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

message("CCBD [${CMAKE_CURRENT_BINARY_DIR}] PBD [${CMAKE_CURRENT_BINARY_DIR}]")

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/SoDaUtilsConfig.cmake
  DESTINATION ${UTILS_CONFIG_EXPORT_DIR}
)


add_subdirectory(test)


get_property(imported_targets DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY IMPORTED_TARGETS)
message("imported targets [${imported_targets}]")

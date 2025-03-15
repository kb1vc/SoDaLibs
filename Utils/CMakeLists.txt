CMAKE_MINIMUM_REQUIRED(VERSION 3.6.0) # this lets out really old distros like CentOS 6

OPTION(BUILD_RPM "Build an RPM package for this platform, or something like it." OFF)
OPTION(BUILD_DEB "Build a Debian package for this platform, or something like it." OFF)
OPTION(BUILD_UNIT_TESTS "Build the unit tests -- not normally useful" OFF)

IF(CMAKE_VERSION VERSION_GREATER 3.0.0)
  CMAKE_POLICY(SET CMP0048 NEW)
  SET(CMAKE_CXX_STANDARD 11)
ENDIF()

PROJECT(SoDaUtils)

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

set(SoDaUtils_PACKAGE_DESCRIPTION 
  "SoDaUtils provides simple string formatting, command line parsing, message handling, and barrier synchronization for C++ applications."
)

IF(BUILD_RPM)
  set(CPACK_GENERATOR "RPM")
ENDIF()

IF(${BUILD_DEB})
  set(CPACK_GENERATOR "DEB")
ENDIF()

# Find the platform release name
FIND_PROGRAM(LSB_RELEASE lsb_release)
EXECUTE_PROCESS(COMMAND ${LSB_RELEASE} -is
  OUTPUT_VARIABLE OS_RELEASE_NAME
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
EXECUTE_PROCESS(COMMAND ${LSB_RELEASE} -rs
  OUTPUT_VARIABLE OS_RELEASE_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# optimizations
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

IF(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  SET(MACOSX 1)
ELSE()
  SET(MACOSX 0)
ENDIF()
	
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
# No -ffast-math because we want Format to know a NaN when it sees one. 
SET(CMAKE_CXX_FLAGS_RELEASE "-O2")

MESSAGE("SoDaUtils_VERSION=[${SoDaUtils_VERSION}]")

FIND_PACKAGE(Git QUIET)

IF((EXISTS "${CMAKE_SOURCE_DIR}/.git/") AND GIT_FOUND)
  EXECUTE_PROCESS(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  EXECUTE_PROCESS(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  SET(SoDaUtils_GIT_ID "${GIT_BRANCH}:${GIT_COMMIT_HASH}")
  FILE(WRITE ${CMAKE_BINARY_DIR}/saved_git_version.txt "${GIT_BRANCH}\n")
  FILE(APPEND ${CMAKE_BINARY_DIR}/saved_git_version.txt "${GIT_COMMIT_HASH}\n")    
elseIF(EXISTS ${CMAKE_BINARY_DIR}/saved_git_version.txt)
  MESSAGE("Retrieving GIT version info from saved_git_version.txt")
  FILE(STRINGS ${CMAKE_BINARY_DIR}/saved_git_version.txt git_version_strings)
  LIST(GET git_version_strings 0 GIT_BRANCH)
  LIST(GET git_version_strings 1 GIT_COMMIT_HASH)
  SET(SoDaUtils_GIT_ID "${GIT_BRANCH}:${GIT_COMMIT_HASH}")
else()
  SET(SoDaUtils_GIT_ID "UNKNOWN")  
ENDIF()

SET(DOXY_PROJECT_VERSION "${SoDaUtils_VERSION}-${SoDaUtils_GIT_ID}")
CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/version.hxx.in"
  "${PROJECT_BINARY_DIR}/UtilsVersion.hxx")


INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include ${YAML_CPP_INCLUDE_DIR})

FILE(GLOB all_includes ${CMAKE_CURRENT_LIST_DIR}/include/*.hxx)

if(NOT yaml-cpp_FOUND) 
  FILE(GLOB ptree_includes ${CMAKE_CURRENT_LIST_DIR}/include/Property*.hxx)
  foreach(pti IN LISTS ptree_includes)
    list(REMOVE_ITEM all_includes ${pti})  
  endforeach()
endif()

INSTALL(FILES ${PROJECT_BINARY_DIR}/UtilsVersion.hxx DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SoDa")
#INSTALL(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
#  # FILES_MATCHING PATTERN "*.hxx"
#  DESTINATION "${CMAKE_INSTALL_PREFIX}/include/SoDa")
INSTALL(FILES ${all_includes} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SoDa")

# The library sources
#ADD_SUBDIRECTORY(cmake)
ADD_SUBDIRECTORY(src)

IF(NOT DISABLE_DOXYGEN)	
  FIND_PACKAGE(Doxygen QUIET)

  IF(DOXYGEN_FOUND)
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    ADD_CUSTOM_TARGET(sodautilsdoc ALL
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )

    INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/SoDaUtils/doc)
  ENDIF()
ENDIF()

FILE(GLOB examp_files "${CMAKE_CURRENT_LIST_DIR}/examples/*.*")
INSTALL(FILES ${examp_files}
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/SoDaUtils/examples  
)

IF(BUILD_RPM OR BUILD_DEB) 
INCLUDE(cmake/BuildPackage.cmake)
ENDIF()



# Now all the real work.




set(CONFIG_EXPORT_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/SoDaUtils")
set(EXPORT_TARGETS sodautils)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/SoDaUtilsConfig.cmake.in"
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
    "${PROJECT_SOURCE_DIR}/SoDaPropsConfig.cmake.in"
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
ADD_SUBDIRECTORY(apptainer)

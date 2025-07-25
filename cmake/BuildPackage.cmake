# Borrowed from https://wiki.o-ran-sc.org/display/ORAN/Packaging+Libraries+for+Linux+Distributions+with+CMake

IF( EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake" )
  include( InstallRequiredSystemLibraries )

  # can only build one of the two packages
  set( CPACK_SET_DESTDIR "off" )
  set( CPACK_PACKAGING_INSTALL_PREFIX "${install_root}" )
  set(CPACK_GENERATOR "")
  if(BUILD_RPM) 
    list(APPEND CPACK_GENERATOR "RPM" )
    set(KIT_GEN_STRING "CC_FED")
  endif()
  if(BUILD_DEB)
    list(APPEND CPACK_GENERATOR "DEB" )
    set(KIT_GEN_STRING "CC_UBUNTU")    
  endif()

  set(CPACK_PACKAGE_NAME "sodalibs")
  
  if(PACKAGE_SYSTEM_NAME)
    set(TARGET_SYSNAME "_${PACKAGE_SYSTEM_NAME}")
  else()
    set(TARGET_SYSNAME "_${CMAKE_HOST_SYSTEM}")
  endif()

  
  set( CPACK_PACKAGE_DESCRIPTION "SoDa package for string formatting, command-line parsing, and signal processing." )

  set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "SoDa Libraries" )
  set( CPACK_PACKAGE_VENDOR "kb1vc" )
  set( CPACK_PACKAGE_CONTACT "https://github.com/kb1vc/" )
  set( CPACK_PACKAGE_VERSION_MAJOR "${SoDaLibs_VERSION_MAJOR}")
  set( CPACK_PACKAGE_VERSION_MINOR "${SoDaLibs_VERSION_MINOR}")
  set( CPACK_PACKAGE_VERSION_PATCH "${SoDaLibs_VERSION_PATCH}")


  set( CPACK_PACKAGE_VERSON "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

  set( CPACK_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}_${CMAKE_HOST_SYSTEM_VERSION}" )

  set( CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}" )
  

  set( CPACK_RPM_PACKAGE_REQUIRES "fftw-devel >= 1.0.0, gcc-g++ >= 5.0.0")

  set( CPACK_RPM_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )
  set( CPACK_RPM_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})

  message(STATUS "CPACK_RPM_PACKAGE_VERSION [${CPACK_RPM_PACKAGE_VERSION}]")
  
  set( CPACK_DEBIAN_PACKAGE_PRIORITY "optional" )
  set( CPACK_DEBIAN_PACKAGE_SECTION "devel" )
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "kb1vc@kb1vc.org")
  set( CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} )

  set( CPACK_DEBIAN_PACKAGE_DEPENDS "libfftw3-dev, gcc, g++, build-essential, git, cmake, make")
  
INCLUDE( CPack )

ENDIF()

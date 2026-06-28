# CPack configuration for SoDaLibs.
# SoDaLibs produces static libraries + headers, so this is a development
# package.  Runtime deps are the static-lib build deps: fftw-devel and
# openssl-devel on RPM-based distros, libfftw3-dev and libssl-dev on Debian.

if(NOT EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")
  message(WARNING "CPack not available -- skipping package generation")
  return()
endif()

set(CPACK_SET_DESTDIR "off")
set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(CPACK_GENERATOR "")

if(BUILD_RPM)
  list(APPEND CPACK_GENERATOR "RPM")
endif()
if(BUILD_DEB)
  list(APPEND CPACK_GENERATOR "DEB")
endif()

set(CPACK_PACKAGE_NAME "sodalibs-devel")
set(CPACK_PACKAGE_DESCRIPTION
  "SoDa Libraries: string formatting, command-line parsing, and DSP signal processing. \
Provides static libraries SoDaUtils and SoDaSignals with their CMake config files.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SoDa Libraries (static libs + headers)")
set(CPACK_PACKAGE_VENDOR "kb1vc")
set(CPACK_PACKAGE_CONTACT "https://github.com/kb1vc/")
set(CPACK_PACKAGE_VERSION_MAJOR "${SoDaLibs_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${SoDaLibs_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${SoDaLibs_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION
  "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

set(CPACK_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")

# RPM: static .a files live in -devel packages on Fedora/RHEL
set(CPACK_RPM_PACKAGE_REQUIRES "fftw-devel >= 3.0, openssl-devel >= 1.0")
set(CPACK_RPM_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")

# DEB: static .a files live in -dev packages on Ubuntu/Debian
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libfftw3-dev, libssl-dev")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SECTION "libdevel")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "kb1vc@kb1vc.org")
set(CPACK_DEBIAN_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)

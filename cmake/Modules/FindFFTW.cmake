# - Find the FFTW library
#
# Original version of this file:
#   Copyright (c) 2015, Wenzel Jakob
#   https://github.com/wjakob/layerlab/blob/master/cmake/FindFFTW.cmake, commit 4d58bfdc28891b4f9373dfe46239dda5a0b561c6
# Modifications:
#   Copyright (c) 2017, Patrick Bos
#   Copyright (c) 2025, Matt Reilly -- modified to fit in SoDa specific location on the cmake path
# 
#
# Usage:
#   find_package(SoDa_FFTW [REQUIRED] [QUIET] [COMPONENTS component1 ... componentX] )
#
# It sets the following variables:
#   SoDa_FFTW_FOUND                  ... true if fftw is found on the system
#   SoDa_FFTW_[component]_LIB_FOUND  ... true if the component is found on the system (see components below)
#   SoDa_FFTW_LIBRARIES              ... full paths to all found fftw libraries
#   SoDa_FFTW_[component]_LIB        ... full path to one of the components (see below)
#   SoDa_FFTW_INCLUDE_DIRS           ... fftw include directory paths
#
# The following variables will be checked by the function
#   SoDa_FFTW_USE_STATIC_LIBS        ... if true, only static libraries are found, otherwise both static and shared.
#   SoDa_FFTW_ROOT                   ... if set, the libraries are exclusively searched
#                                   under this path
#
# This package supports the following components:
#   FLOAT_LIB
#   DOUBLE_LIB
#   LONGDOUBLE_LIB
#   FLOAT_THREADS_LIB
#   DOUBLE_THREADS_LIB
#   LONGDOUBLE_THREADS_LIB
#   FLOAT_OPENMP_LIB
#   DOUBLE_OPENMP_LIB
#   LONGDOUBLE_OPENMP_LIB
#

# TODO (maybe): extend with ExternalProject download + build option
# TODO: put on conda-forge


if( NOT SoDa_FFTW_ROOT AND DEFINED ENV{SoDa_FFTWDIR} )
    set( SoDa_FFTW_ROOT $ENV{SoDa_FFTWDIR} )
endif()

# Check if we can use PkgConfig
find_package(PkgConfig)

#Determine from PKG
if( PKG_CONFIG_FOUND AND NOT SoDa_FFTW_ROOT )
    pkg_check_modules( PKG_SoDa_FFTW QUIET "fftw3" )
endif()

#Check whether to search static or dynamic libs
set( CMAKE_FIND_LIBRARY_SUFFIXES_SAV ${CMAKE_FIND_LIBRARY_SUFFIXES} )

if( ${SoDa_FFTW_USE_STATIC_LIBS} )
    set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX} )
else()
    set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_SAV} )
endif()

if( SoDa_FFTW_ROOT )
    # find libs

    find_library(
        SoDa_FFTW_DOUBLE_LIB
        NAMES "fftw3" libfftw3-3
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    find_library(
        SoDa_FFTW_DOUBLE_THREADS_LIB
        NAMES "fftw3_threads"
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    find_library(
        SoDa_FFTW_DOUBLE_OPENMP_LIB
        NAMES "fftw3_omp"
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    find_library(
        SoDa_FFTW_DOUBLE_MPI_LIB
        NAMES "fftw3_mpi"
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    find_library(
        SoDa_FFTW_FLOAT_LIB
        NAMES "fftw3f" libfftw3f-3
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    find_library(
        SoDa_FFTW_FLOAT_THREADS_LIB
        NAMES "fftw3f_threads"
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    find_library(
        SoDa_FFTW_FLOAT_OPENMP_LIB
        NAMES "fftw3f_omp"
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    find_library(
        SoDa_FFTW_FLOAT_MPI_LIB
        NAMES "fftw3f_mpi"
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    find_library(
        SoDa_FFTW_LONGDOUBLE_LIB
        NAMES "fftw3l" libfftw3l-3
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    find_library(
        SoDa_FFTW_LONGDOUBLE_THREADS_LIB
        NAMES "fftw3l_threads"
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    find_library(
        SoDa_FFTW_LONGDOUBLE_OPENMP_LIB
        NAMES "fftw3l_omp"
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    find_library(
        SoDa_FFTW_LONGDOUBLE_MPI_LIB
        NAMES "fftw3l_mpi"
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "lib" "lib64"
        NO_DEFAULT_PATH
        )

    #find includes
    find_path(SoDa_FFTW_INCLUDE_DIRS
        NAMES "fftw3.h"
        PATHS ${SoDa_FFTW_ROOT}
        PATH_SUFFIXES "include"
        NO_DEFAULT_PATH
        )

else()

    find_library(
        SoDa_FFTW_DOUBLE_LIB
        NAMES "fftw3"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_library(
        SoDa_FFTW_DOUBLE_THREADS_LIB
        NAMES "fftw3_threads"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_library(
        SoDa_FFTW_DOUBLE_OPENMP_LIB
        NAMES "fftw3_omp"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_library(
        SoDa_FFTW_DOUBLE_MPI_LIB
        NAMES "fftw3_mpi"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_library(
        SoDa_FFTW_FLOAT_LIB
        NAMES "fftw3f"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_library(
        SoDa_FFTW_FLOAT_THREADS_LIB
        NAMES "fftw3f_threads"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_library(
        SoDa_FFTW_FLOAT_OPENMP_LIB
        NAMES "fftw3f_omp"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_library(
        SoDa_FFTW_FLOAT_MPI_LIB
        NAMES "fftw3f_mpi"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_library(
        SoDa_FFTW_LONGDOUBLE_LIB
        NAMES "fftw3l"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_library(
        SoDa_FFTW_LONGDOUBLE_THREADS_LIB
        NAMES "fftw3l_threads"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_library(SoDa_FFTW_LONGDOUBLE_OPENMP_LIB
        NAMES "fftw3l_omp"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_library(SoDa_FFTW_LONGDOUBLE_MPI_LIB
        NAMES "fftw3l_mpi"
        PATHS ${PKG_SoDa_FFTW_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
        )

    find_path(SoDa_FFTW_INCLUDE_DIRS
        NAMES "fftw3.h"
        PATHS ${PKG_SoDa_FFTW_INCLUDE_DIRS} ${INCLUDE_INSTALL_DIR}
        )

endif( SoDa_FFTW_ROOT )

#--------------------------------------- components

if (SoDa_FFTW_DOUBLE_LIB)
    set(SoDa_FFTW_DOUBLE_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_DOUBLE_LIB})
    add_library(SoDa_FFTW::Double INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::Double
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_DOUBLE_LIB}"
        )
else()
    set(SoDa_FFTW_DOUBLE_LIB_FOUND FALSE)
endif()

if (SoDa_FFTW_FLOAT_LIB)
    set(SoDa_FFTW_FLOAT_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_FLOAT_LIB})
    add_library(SoDa_FFTW::Float INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::Float
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_FLOAT_LIB}"
        )
else()
    set(SoDa_FFTW_FLOAT_LIB_FOUND FALSE)
endif()

if (SoDa_FFTW_LONGDOUBLE_LIB)
    set(SoDa_FFTW_LONGDOUBLE_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_LONGDOUBLE_LIB})
    add_library(SoDa_FFTW::LongDouble INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::LongDouble
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_LONGDOUBLE_LIB}"
        )
else()
    set(SoDa_FFTW_LONGDOUBLE_LIB_FOUND FALSE)
endif()

if (SoDa_FFTW_DOUBLE_THREADS_LIB)
    set(SoDa_FFTW_DOUBLE_THREADS_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_DOUBLE_THREADS_LIB})
    add_library(SoDa_FFTW::DoubleThreads INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::DoubleThreads
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_DOUBLE_THREADS_LIB}"
        )
else()
    set(SoDa_FFTW_DOUBLE_THREADS_LIB_FOUND FALSE)
endif()

if (SoDa_FFTW_FLOAT_THREADS_LIB)
    set(SoDa_FFTW_FLOAT_THREADS_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_FLOAT_THREADS_LIB})
    add_library(SoDa_FFTW::FloatThreads INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::FloatThreads
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_FLOAT_THREADS_LIB}"
        )
else()
    set(SoDa_FFTW_FLOAT_THREADS_LIB_FOUND FALSE)
endif()

if (SoDa_FFTW_LONGDOUBLE_THREADS_LIB)
    set(SoDa_FFTW_LONGDOUBLE_THREADS_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_LONGDOUBLE_THREADS_LIB})
    add_library(SoDa_FFTW::LongDoubleThreads INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::LongDoubleThreads
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_LONGDOUBLE_THREADS_LIB}"
        )
else()
    set(SoDa_FFTW_LONGDOUBLE_THREADS_LIB_FOUND FALSE)
endif()

if (SoDa_FFTW_DOUBLE_OPENMP_LIB)
    set(SoDa_FFTW_DOUBLE_OPENMP_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_DOUBLE_OPENMP_LIB})
    add_library(SoDa_FFTW::DoubleOpenMP INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::DoubleOpenMP
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_DOUBLE_OPENMP_LIB}"
        )
else()
    set(SoDa_FFTW_DOUBLE_OPENMP_LIB_FOUND FALSE)
endif()

if (SoDa_FFTW_FLOAT_OPENMP_LIB)
    set(SoDa_FFTW_FLOAT_OPENMP_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_FLOAT_OPENMP_LIB})
    add_library(SoDa_FFTW::FloatOpenMP INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::FloatOpenMP
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_FLOAT_OPENMP_LIB}"
        )
else()
    set(SoDa_FFTW_FLOAT_OPENMP_LIB_FOUND FALSE)
endif()

if (SoDa_FFTW_LONGDOUBLE_OPENMP_LIB)
    set(SoDa_FFTW_LONGDOUBLE_OPENMP_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_LONGDOUBLE_OPENMP_LIB})
    add_library(SoDa_FFTW::LongDoubleOpenMP INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::LongDoubleOpenMP
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_LONGDOUBLE_OPENMP_LIB}"
        )
else()
    set(SoDa_FFTW_LONGDOUBLE_OPENMP_LIB_FOUND FALSE)
endif()

if (SoDa_FFTW_DOUBLE_MPI_LIB)
    set(SoDa_FFTW_DOUBLE_MPI_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_DOUBLE_MPI_LIB})
    add_library(SoDa_FFTW::DoubleMPI INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::DoubleMPI
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_DOUBLE_MPI_LIB}"
        )
else()
    set(SoDa_FFTW_DOUBLE_MPI_LIB_FOUND FALSE)
endif()

if (SoDa_FFTW_FLOAT_MPI_LIB)
    set(SoDa_FFTW_FLOAT_MPI_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_FLOAT_MPI_LIB})
    add_library(SoDa_FFTW::FloatMPI INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::FloatMPI
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_FLOAT_MPI_LIB}"
        )
else()
    set(SoDa_FFTW_FLOAT_MPI_LIB_FOUND FALSE)
endif()

if (SoDa_FFTW_LONGDOUBLE_MPI_LIB)
    set(SoDa_FFTW_LONGDOUBLE_MPI_LIB_FOUND TRUE)
    set(SoDa_FFTW_LIBRARIES ${SoDa_FFTW_LIBRARIES} ${SoDa_FFTW_LONGDOUBLE_MPI_LIB})
    add_library(SoDa_FFTW::LongDoubleMPI INTERFACE IMPORTED)
    set_target_properties(SoDa_FFTW::LongDoubleMPI
        PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SoDa_FFTW_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${SoDa_FFTW_LONGDOUBLE_MPI_LIB}"
        )
else()
    set(SoDa_FFTW_LONGDOUBLE_MPI_LIB_FOUND FALSE)
endif()

#--------------------------------------- end components

set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_SAV} )

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SoDa_FFTW
    REQUIRED_VARS SoDa_FFTW_INCLUDE_DIRS
    HANDLE_COMPONENTS
    )

mark_as_advanced(
    SoDa_FFTW_INCLUDE_DIRS
    SoDa_FFTW_LIBRARIES
    SoDa_FFTW_FLOAT_LIB
    SoDa_FFTW_DOUBLE_LIB
    SoDa_FFTW_LONGDOUBLE_LIB
    SoDa_FFTW_FLOAT_THREADS_LIB
    SoDa_FFTW_DOUBLE_THREADS_LIB
    SoDa_FFTW_LONGDOUBLE_THREADS_LIB
    SoDa_FFTW_FLOAT_OPENMP_LIB
    SoDa_FFTW_DOUBLE_OPENMP_LIB
    SoDa_FFTW_LONGDOUBLE_OPENMP_LIB
    SoDa_FFTW_FLOAT_MPI_LIB
    SoDa_FFTW_DOUBLE_MPI_LIB
    SoDa_FFTW_LONGDOUBLE_MPI_LIB
    )

CMAKE_MINIMUM_REQUIRED(VERSION 3.10.0) # this lets out really old distros like CentOS 6

IF(CMAKE_VERSION VERSION_GREATER 3.10.0)
  CMAKE_POLICY(SET CMP0048 NEW)
  SET(CMAKE_CXX_STANDARD 11)
ENDIF()

PROJECT(SoDaUtilsExample)

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(yaml-cpp REQUIRED)

# optimizations
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")

# This is all you need to do if the SoDaUtils stuff
# was installed correctly. 
FIND_PACKAGE(SoDaUtils REQUIRED)

########### next target ###############


add_executable(UtilsExample UtilsExample.cxx)
target_link_libraries(UtilsExample PRIVATE sodautils)


add_executable(OptionsExample OptionsExample.cxx)
target_link_libraries(OptionsExample PRIVATE sodautils)

add_executable(FormatExample FormatExample.cxx)
target_link_libraries(FormatExample PRIVATE sodautils)

add_executable(Format_Test Format_Test.cxx)
target_link_libraries(Format_Test PRIVATE sodautils)


add_executable(BarrierExample BarrierExample.cxx)
target_link_libraries(BarrierExample PRIVATE sodautils Threads::Threads)

add_executable(PropertyTreeYamlExample PropertyTreeYamlExample.cxx)
target_link_libraries(PropertyTreeYamlExample PRIVATE sodautils Threads::Threads)






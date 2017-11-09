# project
project(tnsdiff)

# requires cmake version 2.8 or greater
cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

set(TNSDIFF_SOURCE_DIR ${CMAKE_SOURCE_DIR})
 
#
set(CMAKE_MODULE_PATH ${TNSDIFF_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH})

set(CMAKE_MODULE_PATH ${TNSDIFF_SOURCE_DIR}/cmake/modules
    ${CMAKE_MODULE_PATH})

# version
set(TNSDIFF_VERSION "")
file(STRINGS "${CMAKE_SOURCE_DIR}/VERSION"
    _TNSDIFF_VERSION REGEX "version:")
string(REGEX REPLACE "version: " "" TNSDIFF_VERSION ${_TNSDIFF_VERSION})

#
include(Utilities)

#
colormsg(HIBLUE "Configuring TNS Utilities version: ${TNSDIFF_VERSION}")

#
add_subdirectory(src)

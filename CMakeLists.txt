########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(SoapyRFSA CXX)

# select build type to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

find_package(SoapySDR NO_MODULE PATHS "C:/ProgramData/radioconda/pkgs/soapysdr-0.8.1-py311h005e61a_4/Library")
if (NOT SoapySDR_FOUND)
    message(FATAL_ERROR "Soapy SDR development files not found...")
endif ()

########################################################################
# Find RFSA
########################################################################
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
set(RFSA_INCLUDE_DIRS "C:/Program Files/IVI Foundation/VISA/Win64/Include/")
set(IVI_INCLUDE_DIRS "C:/Program Files/IVI Foundation/IVI/Include/")
set(RFSA_LIBRARIES "C:/Program Files/IVI Foundation/VISA/Win64/Lib_x64/msc/")

include_directories(${RFSA_INCLUDE_DIRS})
include_directories(${IVI_INCLUDE_DIRS})
SOAPY_SDR_MODULE_UTIL(
    TARGET RFSASupport
    SOURCES
    RFSA_Registration.cpp
    RFSA_Settings.cpp
    RFSA_Streaming.cpp
    LIBRARIES
    legacy_stdio_definitions.lib
    ${RFSA_LIBRARIES}/niRFSA.lib
    ${RFSA_LIBRARIES}/visa64.lib
)
